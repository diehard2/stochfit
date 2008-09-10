/* 
 *	Copyright (C) 2008 Stephen Danauskas
 *	
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "stdafx.h"
#include "ParamVector.h"
#include "ReflCalc.h"

// Multilayer reflection and transmission
// The calculation scheme used can be found in 
// L. G. Parratt, Phys. Rev. 95(2), 359(1954)


CReflCalc::~CReflCalc()
{
		_mm_free(nk);
		_mm_free(xi);
		_mm_free(yi);
		_mm_free(sinthetai);
		_mm_free(reflpt);
		_mm_free(dataout);
		_mm_free(tsinthetai);
		_mm_free(qarray);
		_mm_free(sinsquaredthetai);
		_mm_free(tsinsquaredthetai);
		_mm_free(eyi);
		_mm_free(doublenk);
		_mm_free(distarray);
		_mm_free(rhoarray);
		_mm_free(imagrhoarray);
		_mm_free(m_ckk);
		_mm_free(m_dkk);
		_mm_free(m_cak);
		_mm_free(m_crj);
		_mm_free(m_drj);
		_mm_free(m_cRj);
		_mm_free(edspacingarray);
		_mm_free(fresnelcurve);
		_mm_free(qspreadsinsquaredthetai);
		_mm_free(qspreadreflpt);
		_mm_free(qspreadsinthetai);

		if(exi != NULL)
			_mm_free(exi);

}

CReflCalc::CReflCalc():exi(NULL),eyi(NULL),xi(NULL),yi(NULL)
{}

void CReflCalc::init(ReflSettings* InitStruct)
{
	int resolution;
	m_InitStruct = InitStruct;
	
	//int numberofdatapoints, double xraylambda,double d0, BOOL usesurfabs, int parratlayers, double leftoffset, BOOL forcenorm, double Qspread, bool XRonly

   
	m_bUseSurfAbs = InitStruct->UseSurfAbs;
	lambda = InitStruct->Wavelength;
	k0 = 2.0*M_PI/lambda;
    dz0= 1.0f/InitStruct->Resolution;
	objectivefunction = InitStruct->Objectivefunction;
	m_bforcenorm = InitStruct->Forcenorm;
	m_dnormfactor = 1.0;
	m_dQSpread = InitStruct->QErr/100;
	m_ihighEDduplicatepts = 0;
	m_ilowEDduplicatepts = 0;
	m_bXRonly = InitStruct->XRonly;
	m_bImpNorm = InitStruct->Impnorm;
	//Set the total length of our surface layer - default 80 Angstroms of superphase,
	//7 extra Angstroms of file, and 40 Angstroms of subphase
	if(InitStruct->Totallength > 0)
		totalsize = InitStruct->Totallength; 
	else
        totalsize = InitStruct->Leftoffset + InitStruct->FilmLength + 7 + 40;

	nl= totalsize*InitStruct->Resolution;

	m_dboxsize = (InitStruct->FilmLength+7.0)/(InitStruct->Boxes);

	m_dwaveconstant = lambda*lambda/(2.0*M_PI);
	//Setup OpenMP - currently a maximum of 6 processors is allowed. After a certain number
	//of data points, there will not be much of a benefit to allowing additional processors
	//to work on the calculation. If more than 2-300 data points are being analyzed, it would
	//make sense to increase this number
	if(MAX_OMP_THREADS > omp_get_num_procs())
	{
		#ifdef SINGLEPROCDEBUG
			m_iuseableprocessors = 1;
		#else
			m_iuseableprocessors = omp_get_num_procs();
		#endif
	}
	else
	{
		#ifdef SINGLEPROCDEBUG
			m_iuseableprocessors = 1;
		#else
			m_iuseableprocessors = MAX_OMP_THREADS;
		#endif
	}

	omp_set_num_threads(m_iuseableprocessors);
	
	rho_a = InitStruct->FilmSLD * 1e-6 * lambda*lambda/(2.0f*M_PI);

	if(InitStruct->UseSurfAbs == TRUE)
	{
		beta_a = InitStruct->FilmAbs * 1e-6 * lambda*lambda/(2.0f*M_PI);;
		beta_sub = InitStruct->SupAbs * 1e-6 * lambda*lambda/(2.0f*M_PI);;
		beta_sup = InitStruct->SupAbs * 1e-6 * lambda*lambda/(2.0f*M_PI);;
	}
	else
	{
		beta_a = beta_sub = beta_sup = 0;
	}

	//Arrays for the electron density and twice the electron density
    nk = (MyComplex*)_mm_malloc(sizeof(MyComplex)*nl,16);
    doublenk = (MyComplex*)_mm_malloc(sizeof(MyComplex)*nl,16);

	//Create the scratch arrays for the reflectivity calculation
	m_ckk = (MyComplex *)_mm_malloc(sizeof(MyComplex )*nl*m_iuseableprocessors,16);
	m_dkk = (float*)_mm_malloc(sizeof(float)*nl*m_iuseableprocessors,16);
	m_cak = (MyComplex *)_mm_malloc(sizeof(MyComplex )*nl*m_iuseableprocessors,16);
	m_crj = (MyComplex *)_mm_malloc(sizeof(MyComplex )*nl*m_iuseableprocessors,16);
	m_drj = (float*)_mm_malloc(sizeof(float)*nl*m_iuseableprocessors,16);
	m_cRj = (MyComplex *)_mm_malloc(sizeof(MyComplex )*nl*m_iuseableprocessors,16);

	//Create scratch arrays for the electron density calculation
	distarray = (float*)_mm_malloc((InitStruct->Boxes+2)*sizeof(float),64);
	rhoarray = (float*)_mm_malloc((InitStruct->Boxes+2)*sizeof(float),64);
	imagrhoarray = (float*)_mm_malloc((InitStruct->Boxes+2)*sizeof(float),64);
	edspacingarray = (float*)_mm_malloc(nl*sizeof(float),64);

	for(int i = 0; i < nl; i++)
	{
		edspacingarray[i] = i*dz0-InitStruct->Leftoffset;
	}

	for(int k = 0; k < InitStruct->Boxes+2; k++)
	{
		distarray[k] = k*m_dboxsize;
	}

	//Set the output file names
    fnpop = InitStruct->Directory + wstring(L"\\pop.dat");
    fnrf = InitStruct->Directory + wstring(L"\\rf.dat");
    fnrho = InitStruct->Directory + wstring(L"\\rho.dat");

	SetupRef(InitStruct);
}

void CReflCalc::SetupRef(ReflSettings* InitStruct)
{
	//Now create our xi,yi,dyi, and thetai
	m_idatapoints = InitStruct->QPoints - InitStruct->HighQOffset - InitStruct->CritEdgeOffset - 1;
	xi = (float*)_mm_malloc(m_idatapoints*sizeof(float),64);
	yi = (float*)_mm_malloc(m_idatapoints*sizeof(float),64);
	
	if(InitStruct->QError != NULL)
		exi = (float*)_mm_malloc(m_idatapoints*sizeof(float),64);
	
	eyi = (float*)_mm_malloc(m_idatapoints*sizeof(float),64);

	sinthetai = (float*)_mm_malloc(m_idatapoints*sizeof(float),64);
	sinsquaredthetai = (float*)_mm_malloc(m_idatapoints*sizeof(float),64);
	qspreadsinthetai = (float*)_mm_malloc(m_idatapoints*13*sizeof(float),64);
	qspreadsinsquaredthetai = (float*)_mm_malloc(m_idatapoints*13*sizeof(float),64);
	reflpt = (float*)_mm_malloc(m_idatapoints*sizeof(float),64);
	qspreadreflpt = (float*)_mm_malloc(m_idatapoints*13*sizeof(float),64);

	//and fill them up
	for(int i = 0; i< m_idatapoints; i++)
	{
		xi[i] = InitStruct->Q[i+InitStruct->CritEdgeOffset + 1];
		yi[i] = InitStruct->Refl[i+InitStruct->CritEdgeOffset + 1];
		eyi[i] = InitStruct->ReflError[i+InitStruct->CritEdgeOffset + 1];
		
		if(InitStruct->QError != NULL)
			exi[i] = InitStruct->QError[i+InitStruct->CritEdgeOffset + 1];
		
		sinthetai[i] = InitStruct->Q[i+InitStruct->CritEdgeOffset + 1]*lambda/(4*M_PI);
		reflpt[i] = 1.0;
	}

	//Calculate the qspread sinthetai's for resolution smearing
	if(InitStruct->QError != NULL)
	{
		double holder = lambda/(4.0*M_PI);
		float qholder;
		float qerrorholder;
		for(int i = 0; i < m_idatapoints; i++)
		{
			qholder = InitStruct->Q[i];
			qerrorholder = InitStruct->QError[i];

			qspreadsinthetai[13*i] = sinthetai[i];
			qspreadsinthetai[13*i+1] = holder*(qholder+1.2*qerrorholder);
			qspreadsinthetai[13*i+2] = holder*(qholder-1.2*qerrorholder);
			qspreadsinthetai[13*i+3] = holder*(qholder+1.0*qerrorholder);
			qspreadsinthetai[13*i+4] = holder*(qholder-1.0*qerrorholder);
			qspreadsinthetai[13*i+5] = holder*(qholder+0.8*qerrorholder);
			qspreadsinthetai[13*i+6] = holder*(qholder-0.8*qerrorholder);
			qspreadsinthetai[13*i+7] = holder*(qholder+0.6*qerrorholder);
			qspreadsinthetai[13*i+8] = holder*(qholder-0.6*qerrorholder);
			qspreadsinthetai[13*i+9] = holder*(qholder+0.4*qerrorholder);
			qspreadsinthetai[13*i+10] = holder*(qholder-0.4*qerrorholder);
			qspreadsinthetai[13*i+11] = holder*(qholder+0.2*qerrorholder);
			qspreadsinthetai[13*i+12] = holder*(qholder-0.2*qerrorholder);

			if(qspreadsinthetai[13*i+1] < 0.0)
				MessageBox(NULL, L"Error in QSpread please contact the author - the program will now crash :(", NULL,NULL);
		}
	}
	else
	{
		for(int i = 0; i < m_idatapoints; i++)
		{
			qspreadsinthetai[13*i] = sinthetai[i];
			qspreadsinthetai[13*i+1] = sinthetai[i]*(1+1.2*m_dQSpread);
			qspreadsinthetai[13*i+2] = sinthetai[i]*(1-1.2*m_dQSpread);
			qspreadsinthetai[13*i+3] = sinthetai[i]*(1+1.0*m_dQSpread);
			qspreadsinthetai[13*i+4] = sinthetai[i]*(1-1.0*m_dQSpread);
			qspreadsinthetai[13*i+5] = sinthetai[i]*(1+0.8*m_dQSpread);
			qspreadsinthetai[13*i+6] = sinthetai[i]*(1-0.8*m_dQSpread);
			qspreadsinthetai[13*i+7] = sinthetai[i]*(1+0.6*m_dQSpread);
			qspreadsinthetai[13*i+8] = sinthetai[i]*(1-0.6*m_dQSpread);
			qspreadsinthetai[13*i+9] = sinthetai[i]*(1+0.4*m_dQSpread);
			qspreadsinthetai[13*i+10] = sinthetai[i]*(1-0.4*m_dQSpread);
			qspreadsinthetai[13*i+11] = sinthetai[i]*(1+0.2*m_dQSpread);
			qspreadsinthetai[13*i+12] = sinthetai[i]*(1-0.2*m_dQSpread);

			if(qspreadsinthetai[13*i+1] < 0.0)
				MessageBox(NULL, L"Error in QSpread please contact the author - the program will now crash :(", NULL,NULL);
		}
	}

	//Now, create our q's for plotting, but mix-in the actual data points
	double x0=InitStruct->Q[0];
    double x1=InitStruct->Q[InitStruct->QPoints - 1];
    double dx=(x1-x0)/150.0;
    x1=1.1*x1-0.1*x0;
	
	tsinthetai = (float*)_mm_malloc(3000*sizeof(float),64);
	dataout = (float*)_mm_malloc(3000*sizeof(float),64);
	qarray = (float*)_mm_malloc(3000*sizeof(float),64);
	tarraysize = 0;

	int j=0;
	int inc=0;
	double x;

	for(x=x0;x<=x1;x += dx)
	{
		if(inc < m_idatapoints)
		{
			if(x == xi[inc])
			{
				qarray[j] = x;
				tsinthetai[j] = x*lambda/(4*M_PI);
				inc++;
			}
			else if (x < xi[inc])
			{
				qarray[j] = x;
				tsinthetai[j] = x*lambda/(4*M_PI);
			}
			else
			{
				qarray[j] = xi[inc];
				tsinthetai[j] = xi[inc]*lambda/(4*M_PI);
				inc++;
				x-=dx;
			}
		}
		else
		{
			qarray[j] = x;
			tsinthetai[j] = x*lambda/(4*M_PI);
		}
		tarraysize++;
		j++;
	}

	//Calculate the theta's we'll use to make our reflectivities
	for(int l=0; l<m_idatapoints; l++)
	{
		sinsquaredthetai[l] = sinthetai[l]*sinthetai[l];
	}

	//Calculate the theta's we'll use to make our plotting reflectivity
	tsinsquaredthetai = (float*)_mm_malloc(tarraysize*sizeof(float),64);

	for(int l=0; l<tarraysize; l++)
	{
		tsinsquaredthetai[l] = tsinthetai[l]*tsinthetai[l];
	}

	for(int l=0; l < m_idatapoints*13; l++)
	{
		qspreadsinsquaredthetai[l] = qspreadsinthetai[l]*qspreadsinthetai[l];
	}
	
	//Setup the fresnel curve - we store the electron density portion of the refractive index in the params
	fresnelcurve = (float*)_mm_malloc(m_idatapoints*sizeof(float), 64);

	double Qc = CalcQc(InitStruct->SubSLD, InitStruct->SupSLD);

	for(int i = 0; i < m_idatapoints; i++)
	{
		fresnelcurve[i] = CalcFresnelPoint(xi[i], Qc);
	}
}


//Write output files
void CReflCalc::paramsrf(ParamVector * g)
{
    ofstream rhoout(fnrho.c_str());
  
	if(m_bUseSurfAbs == TRUE)
		mkdensity(g);
	else
		mkdensitytrans(g);

	double x=0;

	if(m_bUseSurfAbs == TRUE)
	{
		for(int j=0; j < nl; j++)
		{
			rhoout << x << ' ' << nk[j].re/nk[nl-1].re << ' ' << nk[j].im/nk[nl-1].im << endl;
			x += dz0;
		}
	}
	else
	{
		for(int j=0; j < nl; j++) 
		{
			rhoout << x << ' ' << nk[j].re/nk[nl-1].re << endl;
			x += dz0;
		}
	}
  
	rhoout.close();

	ofstream reflout(fnrf.c_str());
	
	if(m_bUseSurfAbs == FALSE)
		mytransparentrf(tsinthetai, tsinsquaredthetai, tarraysize, dataout);
	else
		myrf(tsinthetai, tsinsquaredthetai, tarraysize, dataout);
	
	if(m_bforcenorm == TRUE)
	{
		impnorm(dataout,tarraysize,false);
	}

	if(m_bImpNorm == TRUE)
	{
		impnorm(dataout,tarraysize,true);
	}

	#ifndef CHECKREFLCALC
		for(int i = 0;i<tarraysize;i++)
		{
				reflout << qarray[i] << " " << dataout[i] << endl;
		}
	#else
		for(int i = 0;i< m_idatapoints;i++)
		{
			reflout << xi[i] << " " << reflpt[i] << endl;
		}
	#endif

    reflout.close();
}


//The code for the ED calculation section is loosely based on the electron density calculation
//in Motofit (www.sourceforge.net/motofit). It is a standard method of calculating the
//electron density profile. We treat the profile as having a user defined number of boxes
//The last 30% or so of the curve will converge to have rho/rhoinf = 1.0. 
//For lipid and lipid protein films, the absorbance is negligible


void CReflCalc::mkdensitytrans(ParamVector* g)
{
	int refllayers = g->RealparamsSize()-1;
	int reflpoints = nl;
	float roughness = g->getroughness();
	float dist;

	if(g->getroughness() < 0.000000001)
		roughness = 1e-6;

	roughness = 1.0f/( roughness * sqrt(2.0f));
	float supersld = g->GetRealparams(0)*rho_a;
	
	//Don't delete this, otherwise the reflectivity calculation won't work sometimes
	nk[0].im = 0.0f;

			
		
	
		for(int k = 0; k < refllayers; k++)	
		{
			rhoarray[k] = rho_a*(g->GetRealparams(k+1)-g->GetRealparams(k))*0.5f;
		}
		

		#pragma omp parallel for private(dist)
		for(int i = 0; i < reflpoints; i++)
 		{
			nk[i].re = supersld;
		
			
			for(int k = 0; k < refllayers; k++)
			{
				dist = (edspacingarray[i]-distarray[k] )*roughness;

				if(dist > 6.0f)
				{
					nk[i].re += (rhoarray[k])*(2.0f);
				}
				else if(dist > -6.0f)
				{
					nk[i].re += (rhoarray[k])*(1.0f+erff(dist));
				}
			}

			//Make double array for the reflectivity calculation
			doublenk[i].re = 2.0f*nk[i].re;
			doublenk[i].im = 0.0f;
		}
	

	//Find duplicate pts so we don't do the same calculation over and over again
		for(int i = 0; i < reflpoints; i++)
		{
			if(nk[i].re == nk[0].re)
				m_ilowEDduplicatepts++;
			else
				break;
		}

		for(int i = reflpoints - 1 ; i != 0; i--)
		{
			if(nk[i].re == nk[reflpoints-1].re)
				m_ihighEDduplicatepts = i;
			else
				break;
		}
}

void CReflCalc::mkdensity(ParamVector* g)
{
	int refllayers = g->RealparamsSize()-1;
	
	int reflpoints = nl;
	float roughness = 1.0f/(g->getroughness() * sqrt(2.0));
	float supersld = g->GetRealparams(0)*rho_a;
	
#pragma omp parallel
	{
		float dist;

		#pragma omp for
		for(int k = 0; k < refllayers; k++)
		{
			rhoarray[k] = rho_a*(g->GetRealparams(k+1)-g->GetRealparams(k))/2.0;
			
			//Imag calculation
			if(k == 0)
			{
				imagrhoarray[k] = (beta_a* g->getSurfAbs() * g->GetRealparams(k+1)/g->GetRealparams(refllayers) - beta_sup)/2.0;
			}
			else if(k == refllayers-1)
			{
				imagrhoarray[k] = (beta_sub - beta_a* g->getSurfAbs() * g->GetRealparams(k)/g->GetRealparams(refllayers))/2.0;
			}
			else
			{
				imagrhoarray[k] = (beta_a* g->getSurfAbs() * g->GetRealparams(k+1)/g->GetRealparams(refllayers) - 
					beta_a * g->getSurfAbs()* g->GetRealparams(k)/g->GetRealparams(refllayers))/2.0;
			}
		}

		#pragma omp for private(dist)
		for(int i = 0; i < reflpoints; i++)
 		{
			nk[i].im = beta_sup;
			nk[i].re = supersld;
			
			
			for(int k = 0; k < refllayers; k++)
			{
				dist = (edspacingarray[i]-distarray[k] )*roughness;

				if(dist > 6)
				{
					nk[i].re += (rhoarray[k])*(2.0f);
					nk[i].im += (imagrhoarray[k])*(2.0f);
				}
				else if (dist > -6)
				{
					nk[i].re += (rhoarray[k])*(1.0f+erff(dist));
					nk[i].im += (imagrhoarray[k])*(1.0f+erff(dist));
				}
			}

			doublenk[i].re = 2.0f*nk[i].re;
			doublenk[i].im = 2.0f*nk[i].im;
		}
	}
		//Find duplicate pts so we don't do the same calculation over and over again
		for(int i = 0; i < reflpoints; i++)
		{
			if(nk[i].re == nk[0].re)
				m_ilowEDduplicatepts++;
			else
				break;
		}

		for(int i = reflpoints - 1 ; i != 0; i--)
		{
			if(nk[i].re == nk[reflpoints-1].re)
				m_ihighEDduplicatepts = i;
			else
				break;
		}
}

//Check to see if there is any negative electron density for the XR case
bool CReflCalc::CheckDensity()
{
	int reflpoints = nl;
	

	for(int i = 0; i < reflpoints; i++)
	{
		if(nk[i].re < 0)
			return false;
	}

	return true;
}

double CReflCalc::objective(ParamVector * g)
{
    double sy=0.0,sy2=0.0,b = 0.0;
	int counter = m_idatapoints;

	if(m_bUseSurfAbs == false)
		mkdensitytrans(g);
	else
		mkdensity(g);

	if(m_bXRonly)
	{
		if(CheckDensity() == false)
		{
			if(m_bUseSurfAbs == false)
				mytransparentrf(sinthetai, sinsquaredthetai, m_idatapoints, reflpt);
			else
				myrf(sinthetai, sinsquaredthetai, m_idatapoints, reflpt);

			return -1;
		}
		
	}

	if(m_dQSpread < 0.005)
	{
		if(m_bUseSurfAbs == false)
			mytransparentrf(sinthetai, sinsquaredthetai, m_idatapoints, reflpt);
		else
			myrf(sinthetai, sinsquaredthetai, m_idatapoints, reflpt);
	}
	else
	{
		if(m_bUseSurfAbs == false)
		{
			mytransparentrf(qspreadsinthetai, qspreadsinsquaredthetai, 13*m_idatapoints, qspreadreflpt);
			QsmearRf(qspreadreflpt, reflpt, m_idatapoints);
		}
		else
		{
			myrf(qspreadsinthetai, qspreadsinsquaredthetai, 13*m_idatapoints, qspreadreflpt);
			QsmearRf(qspreadreflpt, reflpt, m_idatapoints);
		}
	}

	//Normalize if we let the absorption vary
	if(m_bforcenorm == TRUE)
	{
		impnorm(reflpt, m_idatapoints, false);
	}
	
	//Fix imperfect normalization
	if(m_bImpNorm == TRUE)
	{
		impnorm(reflpt, m_idatapoints, true);
	}

	//Calculate the fitness score
	int count = 1;
	double calcholder1 = 0;
	double holder;
	
	m_dChiSquare = 0;
    m_dgoodnessoffit = 0;

	//Log difference
	if(objectivefunction == 0)
	{
		for(int i = 0; i< counter; i++)
		{
			calcholder1 = log(yi[i])-log(reflpt[i]);
			m_dgoodnessoffit += calcholder1*(calcholder1);///fabs(log(eyi[i]));
		}

		m_dgoodnessoffit /= counter+1;
	}
	else if (objectivefunction == 1)
	{
		//Inverse difference
		for(int i = 0; i< counter; i++)
		{
			calcholder1 = yi[i]/reflpt[i];
			
			if(calcholder1 < 1.0)
				calcholder1 = 1.0/calcholder1;

			double errormap = 1.0;//yi[i]/eyi[i];
			m_dgoodnessoffit +=(1.0-calcholder1)*(1.0-calcholder1);
		}

		m_dgoodnessoffit /= counter+1;
	}
	else if (objectivefunction == 2)
	{	//Log difference with errors
		for(int i = 0; i< counter; i++)
		{
			calcholder1 = log(yi[i])-log(reflpt[i]);
			m_dgoodnessoffit += calcholder1*calcholder1/fabs(log(eyi[i]));
		}

		m_dgoodnessoffit /= counter+1;
	}
	else if (objectivefunction == 3)
	{
		//Inverse difference with errors
		for(int i = 0; i< counter; i++)
		{
			calcholder1 = yi[i]/reflpt[i];
			
			if(calcholder1 < 1.0)
				calcholder1 = 1.0/calcholder1;

			double errormap = (yi[i]/eyi[i])*(yi[i]/eyi[i]);
			m_dgoodnessoffit +=(1.0-calcholder1)*(1.0-calcholder1)*errormap;
		}

		m_dgoodnessoffit /= counter+1;
		

	}
	else if(objectivefunction == 4)
	{
		//R/Rf error
		for(int i = 0; i< counter; i++)
		{
			calcholder1 = (yi[i]/fresnelcurve[i]-reflpt[i]/fresnelcurve[i]);
			m_dgoodnessoffit += calcholder1*calcholder1/(eyi[i]/fresnelcurve[i]);
		}
		
		m_dgoodnessoffit /= counter+1;
	}


	//Calculate the Chi Square of R/Rf(reduced with no parameters)
	for(int i= 0; i< counter; i++)
	{
		calcholder1 = (yi[i]-reflpt[i]);
		m_dChiSquare += (calcholder1*calcholder1)/(eyi[i]*eyi[i]);
	}

	m_dChiSquare /= counter;
	
    return(m_dgoodnessoffit);
} 


//Perform a rudimentary normalization on the modeled reflectivity (for absorbing films)
//This is for the output reflectivity. If the normalization is imperfect for neutrons,
//this should use isimprefl = true
void CReflCalc::impnorm(float* refl, int datapoints, bool isimprefl)
{
	float normfactor;

	if(isimprefl == true)
		normfactor = m_dnormfactor;
	else
		normfactor = 1.0f/refl[0];

	for(int i = 0; i< datapoints; i++)
	{
		refl[i] *= normfactor;
	}
}

void CReflCalc::myrf(float* sintheta, float* sinsquaredtheta, int datapoints, float* refl)
{
	//Calculate some complex constants to keep them out of the loop
	MyComplex  lengthmultiplier = -2.0f*MyComplex (0.0,1.0)*dz0 ;
	MyComplex  indexsup = 1.0 - nk[0];
	MyComplex  indexsupsquared = indexsup * indexsup;
	MyComplex  zero;
	
	#pragma omp parallel
	{
		//Figure out the number of the thread we're currently in
		int threadnum = omp_get_thread_num();
		int arrayoffset = threadnum*nl;

		float* dkk = m_dkk+arrayoffset;
		MyComplex * kk = m_ckk+arrayoffset;
		MyComplex * ak = m_cak+arrayoffset;
		float* drj = m_drj+arrayoffset;
		MyComplex * rj= m_crj+arrayoffset;
		MyComplex * Rj= m_cRj+arrayoffset;
		MyComplex  cholder, tempk1, tempk2;
		float holder;
		
		/********Boundary conditions********/
		//No reflection in the last layer
		Rj[nl-1] = 0.0f;
		//The first layer and the last layer are assumed to be infinite e^-(0+i*(layerlength)) = 1+e^(-inf*i*beta) = 1 + 0*i*beta
		ak[nl-1] = 1.0f;
		ak[0] = 1.0f;
		
		//In order to vectorize loops, you cannot use global variables
		int nlminone = nl-1;
		int numlay =  nl;

		#pragma omp for schedule(runtime)
		for(int l = 0; l < datapoints;l++)
		{
			//The refractive index for air is 1, so there is no refractive index term for kk[0]
			kk[0] = k0 * indexsup * sintheta[l];

			tempk1 = k0 * compsqrt(indexsupsquared*sinsquaredtheta[l]-doublenk[1]+doublenk[0]);
			tempk2 = k0 * compsqrt(indexsupsquared*sinsquaredtheta[l]-doublenk[numlay-1]+doublenk[0]);
			//Workout the wavevector k -> kk[i] = k0 *compsqrt(sinsquaredthetai[l]-2.0*nk[i]);
			#pragma ivdep
			for(int i = 1; i <= m_ilowEDduplicatepts;i++)
			{
				kk[i] = tempk1;
			}

			#pragma ivdep
			for(int i = m_ilowEDduplicatepts+1; i < m_ihighEDduplicatepts;i++)
			{
				kk[i] = k0 * compsqrt(indexsupsquared*sinsquaredtheta[l]-doublenk[i]+doublenk[0]);
			}

			#pragma ivdep
			for(int i = m_ihighEDduplicatepts; i < numlay; i++)
			{
				kk[i] = tempk2;
			}

			//Make the phase factors ak -> ak[i] = compexp(-1.0*imaginary*kk[i]*dz0/2.0);

			tempk1 = compexp(lengthmultiplier*kk[1]);
			tempk2 = compexp(lengthmultiplier*kk[numlay-1]);

			#pragma ivdep
			for(int i = 1; i <= m_ilowEDduplicatepts;i++)
			{
				ak[i] = tempk1;
			}

			#pragma ivdep
			for(int i = m_ilowEDduplicatepts+1; i < m_ihighEDduplicatepts;i++)
			{
				ak[i] = compexp(lengthmultiplier*kk[i]);
			}

			#pragma ivdep
			for(int i = m_ihighEDduplicatepts; i < numlay-1; i++)
			{
				ak[i] = tempk2;
			}

			//Make the Fresnel coefficients -> rj[i] =(kk[i]-kk[i+1])/(kk[i]+kk[i+1]);
			
			#pragma ivdep
			for(int i = 0; i <= m_ilowEDduplicatepts;i++)
			{
				rj[i] = zero;
			}

			#pragma ivdep
			for(int i = m_ilowEDduplicatepts+1; i < m_ihighEDduplicatepts;i++)
			{
				rj[i] =(kk[i]-kk[i+1])/(kk[i]+kk[i+1]);
			}
			
			#pragma ivdep
			for(int i = m_ihighEDduplicatepts; i < numlay-1; i++)
			{
				rj[i] = zero;
			}
			
			
			//Parratt recursion of the amplitude reflectivity
			
			for(int i = nl-2; i >= m_ihighEDduplicatepts ;i--)
			{
				Rj[i] = ak[nl-2]*(Rj[i+1]+rj[i])/(Rj[i+1]*rj[i]+1.0);
			}
			
			
			for(int i = m_ihighEDduplicatepts-1; i>= m_ilowEDduplicatepts;i--)
			{
				Rj[i] = ak[i]*(Rj[i+1]+rj[i])/(Rj[i+1]*rj[i]+1.0);
			}
			
			for(int i = m_ilowEDduplicatepts; i >= 0; i--)
			{
				Rj[i] = (Rj[i+1]+rj[i])/(Rj[i+1]*rj[i]+1.0);
			}

			//The magnitude of the reflection at layer 0 is the measured reflectivity of the film
			holder = compabs(Rj[0]);
			refl[l] = holder*holder;
		}
	}
	m_ilowEDduplicatepts = 0;
	m_ihighEDduplicatepts = 0;
}

void CReflCalc::mytransparentrf(float* sintheta, float* sinsquaredtheta, int datapoints, float* refl)
{
	////Calculate some complex constants to keep them out of the loop
	MyComplex  lengthmultiplier = -2.0f*MyComplex (0.0f,1.0f)*dz0;
	MyComplex  indexsup = 1.0 - nk[0];
	MyComplex  indexsupsquared = indexsup * indexsup;
	
	int offset = 0;
	int neg = 0;
    for(int i = 0; i< datapoints;i++)
	{	
		
		for(int k = 0; k<nl;k++)
		{
			if((indexsupsquared.re*sinsquaredtheta[i]-doublenk[k].re+doublenk[0].re)< 0.0f)
			{
				neg -= 1;
				break;
			}
		}
		if(neg == 0)
		{
			if(m_dQSpread < 0.005f)
					break;
		}
		else
			offset = i;
	}

	#pragma omp parallel
	{
		//Figure out the number of the thread we're currently in
		int threadnum = omp_get_thread_num();
		int arrayoffset = threadnum*nl;

		float* dkk = m_dkk+arrayoffset;
		MyComplex * kk = m_ckk+arrayoffset;
		MyComplex * ak = m_cak+arrayoffset;
		float* drj = m_drj+arrayoffset;
		MyComplex * rj= m_crj+arrayoffset;
		MyComplex * Rj= m_cRj+arrayoffset;
		MyComplex  cholder, tempk1, tempk2, zero;
		float holder, dtempk1, dtempk2;
		
		/********Boundary conditions********/
		//No reflection in the last layer
		Rj[nl-1] = 0.0f;
		//The first layer and the last layer are assumed to be infinite
		ak[nl-1] = 1.0f;
		ak[0] = 1.0f;

		//In order to vectorize loops, you cannot use global variables
		int nlminone = nl-1;
		int numlay =  nl;

		#pragma omp for nowait schedule(guided)
		for(int l = 0; l<= offset;l++)
		{

			//The refractive index for air is 1, so there is no refractive index term for kk[0]
			kk[0] = k0 * indexsup * sintheta[l];

			tempk1 = k0 * compsqrt(indexsupsquared*sinsquaredtheta[l]-doublenk[1]+doublenk[0]);
			tempk2 = k0 * compsqrt(indexsupsquared*sinsquaredtheta[l]-doublenk[numlay-1]+doublenk[0]);
			//Workout the wavevector k -> kk[i] = k0 *compsqrt(sinsquaredthetai[l]-2.0*nk[i]);
			#pragma ivdep
			for(int i = 1; i <= m_ilowEDduplicatepts;i++)
			{
				kk[i] = tempk1;
			}

			#pragma ivdep
			for(int i = m_ilowEDduplicatepts+1; i < m_ihighEDduplicatepts;i++)
			{
				kk[i] = k0 * compsqrt(indexsupsquared*sinsquaredtheta[l]-doublenk[i]+doublenk[0]);
			}

			#pragma ivdep
			for(int i = m_ihighEDduplicatepts; i < numlay; i++)
			{
				kk[i] = tempk2;
			}

			//Make the phase factors ak -> ak[i] = compexp(-1.0*imaginary*kk[i]*dz0/2.0);

			tempk1 = compexp(lengthmultiplier*kk[1]);
			tempk2 = compexp(lengthmultiplier*kk[numlay-1]);

			#pragma ivdep
			for(int i = 1; i <= m_ilowEDduplicatepts;i++)
			{
				ak[i] = tempk1;
			}

			#pragma ivdep
			for(int i = m_ilowEDduplicatepts+1; i < m_ihighEDduplicatepts;i++)
			{
				ak[i] = compexp(lengthmultiplier*kk[i]);
			}

			#pragma ivdep
			for(int i = m_ihighEDduplicatepts; i < numlay-1; i++)
			{
				ak[i] = tempk2;
			}

			//Make the Fresnel coefficients -> rj[i] =(kk[i]-kk[i+1])/(kk[i]+kk[i+1]);
			
			#pragma ivdep
			for(int i = 0; i <= m_ilowEDduplicatepts;i++)
			{
				rj[i] = zero;
			}

			#pragma ivdep
			for(int i = m_ilowEDduplicatepts+1; i < m_ihighEDduplicatepts;i++)
			{
				rj[i] =(kk[i]-kk[i+1])/(kk[i]+kk[i+1]);
			}
			
			#pragma ivdep
			for(int i = m_ihighEDduplicatepts; i < numlay-1; i++)
			{
				rj[i] = zero;
			}
			
			
			//Parratt recursion of the amplitude reflectivity
			
			for(int i = nl-2; i >= m_ihighEDduplicatepts ;i--)
			{
				Rj[i] = ak[nl-2]*(Rj[i+1]+rj[i])/(Rj[i+1]*rj[i]+1.0);
			}
			
			
			for(int i = m_ihighEDduplicatepts-1; i>= m_ilowEDduplicatepts;i--)
			{
				Rj[i] = ak[i]*(Rj[i+1]+rj[i])/(Rj[i+1]*rj[i]+1.0);
			}
			
			for(int i = m_ilowEDduplicatepts; i >= 0; i--)
			{
				Rj[i] = (Rj[i+1]+rj[i])/(Rj[i+1]*rj[i]+1.0);
			}

			//The magnitude of the reflection at layer 0 is the measured reflectivity of the film
			holder = compabs(Rj[0]);
			refl[l] = holder*holder;
		}

		//Now calculate the rest using doubles
		#pragma omp for schedule(guided)
		for(int l = offset+1; l < datapoints;l++)
		{
				//The refractive index for air is 1, so there is no refractive index term for kk[0]
			dkk[0] = k0 * indexsup.re * sintheta[l];

			dtempk1 = k0 * sqrtf(indexsupsquared.re*sinsquaredtheta[l]-doublenk[1].re+doublenk[0].re);
			dtempk2 = k0 * sqrtf(indexsupsquared.re*sinsquaredtheta[l]-doublenk[numlay-1].re+doublenk[0].re);
			//Workout the wavevector k -> kk[i] = k0 *compsqrt(sinsquaredthetai[l]-2.0*nk[i]);
			#pragma ivdep
			for(int i = 1; i <= m_ilowEDduplicatepts;i++)
			{
				dkk[i] = dtempk1;
			}

			#pragma ivdep
			for(int i = m_ilowEDduplicatepts+1; i < m_ihighEDduplicatepts;i++)
			{
				dkk[i] = k0 * sqrtf(indexsupsquared.re*sinsquaredtheta[l]-doublenk[i].re+doublenk[0].re);
			}

			#pragma ivdep
			for(int i = m_ihighEDduplicatepts; i < numlay; i++)
			{
				dkk[i] = dtempk2;
			}

			//Make the phase factors ak -> ak[i] = compexp(-1.0*imaginary*kk[i]*dz0/2.0);

			tempk1 = compexp(lengthmultiplier*dkk[1]);
			tempk2 = compexp(lengthmultiplier*dkk[numlay-1]);

			#pragma ivdep
			for(int i = 1; i <= m_ilowEDduplicatepts;i++)
			{
				ak[i] = tempk1;
			}

			#pragma ivdep
			for(int i = m_ilowEDduplicatepts+1; i < m_ihighEDduplicatepts;i++)
			{
				ak[i] = compexp(lengthmultiplier*dkk[i]);
			}

			#pragma ivdep
			for(int i = m_ihighEDduplicatepts; i < numlay-1; i++)
			{
				ak[i] = tempk2;
			}

			//Make the Fresnel coefficients -> rj[i] =(kk[i]-kk[i+1])/(kk[i]+kk[i+1]);
			
			#pragma ivdep
			for(int i = 0; i <= m_ilowEDduplicatepts;i++)
			{
				drj[i] = 0.0f;
			}

			#pragma ivdep
			for(int i = m_ilowEDduplicatepts+1; i < m_ihighEDduplicatepts;i++)
			{
				drj[i] =(dkk[i]-dkk[i+1])/(dkk[i]+dkk[i+1]);
			}
			
			#pragma ivdep
			for(int i = m_ihighEDduplicatepts; i < numlay-1; i++)
			{
				drj[i] = 0.0f;
			}
			
			
			//Parratt recursion of the amplitude reflectivity
			
			for(int i = nl-2; i >= m_ihighEDduplicatepts ;i--)
			{
				Rj[i] = ak[nl-2]*(Rj[i+1]+drj[i])/(Rj[i+1]*drj[i]+1.0);
			}
			
			
			for(int i = m_ihighEDduplicatepts-1; i>= m_ilowEDduplicatepts;i--)
			{
				Rj[i] = ak[i]*(Rj[i+1]+drj[i])/(Rj[i+1]*drj[i]+1.0);
			}
			
			for(int i = m_ilowEDduplicatepts; i >= 0; i--)
			{
				Rj[i] = (Rj[i+1]+rj[i])/(Rj[i+1]*drj[i]+1.0);
			}

			//The magnitude of the reflection at layer 0 is the measured reflectivity of the film
			holder = compabs(Rj[0]);
			refl[l] = holder*holder;
			/*if(refl[l] != refl[l])
				MessageBox(NULL,L"Underflow",NULL,NULL);*/
		}
	}
	m_ilowEDduplicatepts = 0;
	m_ihighEDduplicatepts = 0;
}

void CReflCalc::QsmearRf(float* qspreadreflpt, float* refl, int datapoints)
{
	float calcholder;
	#pragma ivdep
	for(int i = 0; i < datapoints; i++)
	{
		calcholder = 0.0f;
		calcholder = qspreadreflpt[13*i];
		calcholder += 0.056f*qspreadreflpt[13*i+1];
		calcholder += 0.056f*qspreadreflpt[13*i+2];
		calcholder += 0.135f*qspreadreflpt[13*i+3];
		calcholder += 0.135f*qspreadreflpt[13*i+4];
		calcholder += 0.278f*qspreadreflpt[13*i+5];
		calcholder += 0.278f*qspreadreflpt[13*i+6];
		calcholder += 0.487f*qspreadreflpt[13*i+7];
		calcholder += 0.487f*qspreadreflpt[13*i+8];
		calcholder += 0.726f*qspreadreflpt[13*i+9];
		calcholder += 0.726f*qspreadreflpt[13*i+10];
		calcholder += 0.923f*qspreadreflpt[13*i+11];
		calcholder += 0.923f*qspreadreflpt[13*i+12];

		refl[i] = calcholder/6.211f;
	}
}
float CReflCalc::CalcQc(double SubPhaseSLD, double SuperPhaseSLD)
{
	//Critical Q for an interface.		
	return 4.0f * sqrt(M_PI*(SubPhaseSLD-SuperPhaseSLD));
}

float CReflCalc::CalcFresnelPoint(float Q, float Qc)
{
    if (Q <= Qc)
        return 1.0f;
    else
    {
        double term1 = sqrt(1.0f - (Qc / Q)*(Qc/Q));
        return ((1.0f - term1) / (1.0f + term1))*((1.0f - term1) / (1.0f + term1));
    }
}