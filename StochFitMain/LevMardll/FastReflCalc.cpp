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
#include "FastReflCalc.h"

// Multilayer reflection and transmission
// Parrett scheme, L. G. Parrett, Phys. Rev. 95(2), 359(1954)
// formulea to use
//      n \cos(\theta) = \cos(\theta_0)
//
//      (E_{k,+} + E_{k,-} ) \cos(\theta_k) = ( E_{k+1,+}/a + E_{k+1,-} a) \cos( \theta_{k+1})
//      (E_{k,+} - E_{k,-} ) \sin(\theta_k) = ( E_{k+1,+}/a - E_{k+1,-} a) \sin( \theta_{k+1})
//      a = exp( i k_{k+1} d_{k+1})
//      k_{k+1} = sqrt( n^2 - \cos^2(\theta_0)) k_0

//
// Due to restriction with OpenMP, and vectorization all of the arrays must be dynamically
// declared. 

FastReflcalc::~FastReflcalc()
{
	_aligned_free(sinsquaredthetai);
	_aligned_free(sinthetai);
	_aligned_free(reflpt);
	_aligned_free(qspreadreflpt);
	_aligned_free(qspreadsinsquaredthetai);
	_aligned_free(qspreadsinthetai);
	delete[] RhoArray;
	delete[] SigmaArray;
	delete[] ImagArray;
	delete[] LengthArray;
}

void FastReflcalc::init(double xraylambda,int boxes, double subSLD, double SupSLD, double* parameters, int paramcount, double* refldata, double* reflerrors, int refldatacount, bool onesig, double QSpread, double normfactor, BOOL impnorm)
{
    lambda=xraylambda;
	onesigma = onesig;
	Realrefl = refldata;
	Realreflerrors = reflerrors;
	realrefllength = refldatacount;
	subphaseSLD = subSLD;
	m_dsupsld = SupSLD;
	param = parameters;
	pcount = paramcount;
	boxnumber = boxes;
	m_dQSpread = QSpread/100.0;
	m_bImpNorm = impnorm;
	m_dnormfactor = 1.0;
	RhoArray = new double[boxnumber+2];
	SigmaArray = new double[boxnumber+2];
	LengthArray = new double[boxnumber+2];
	ImagArray = new double[boxnumber+2];
}

void FastReflcalc::MakeTheta(double* QRange, double* QError, int QRangesize)
{
	m_idatapoints = QRangesize;

	sinthetai = (double*)_aligned_malloc(QRangesize*sizeof(double),64);
	sinsquaredthetai = (double*)_aligned_malloc(QRangesize*sizeof(double),64);
	qspreadsinthetai = (double*)_aligned_malloc(QRangesize*20*sizeof(double),64);
	qspreadsinsquaredthetai = (double*)_aligned_malloc(QRangesize*20*sizeof(double),64);
	qspreadreflpt = (double*)_aligned_malloc(QRangesize*20*sizeof(double),64);

	reflpt = (double*)_aligned_malloc(QRangesize*sizeof(double),64);

	for(int i = 0; i< m_idatapoints; i++)
	{
		sinthetai[i] = QRange[i]*lambda/(4*M_PI);
	}
	

	//Calculate the qspread sinthetai's for resolution smearing
	if(QError != NULL)
	{
		double holder = lambda/(4.0*M_PI);

		for(int i = 0; i < m_idatapoints; i++)
		{
			qspreadsinthetai[13*i] = sinthetai[i];
			qspreadsinthetai[13*i+1] = holder*(QRange[i]+1.2*QError[i]);
			qspreadsinthetai[13*i+2] = holder*(QRange[i]-1.2*QError[i]);
			qspreadsinthetai[13*i+3] = holder*(QRange[i]+1.0*QError[i]);
			qspreadsinthetai[13*i+4] = holder*(QRange[i]-1.0*QError[i]);
			qspreadsinthetai[13*i+5] = holder*(QRange[i]+0.8*QError[i]);
			qspreadsinthetai[13*i+6] = holder*(QRange[i]-0.8*QError[i]);
			qspreadsinthetai[13*i+7] = holder*(QRange[i]+0.6*QError[i]);
			qspreadsinthetai[13*i+8] = holder*(QRange[i]-0.6*QError[i]);
			qspreadsinthetai[13*i+9] = holder*(QRange[i]+0.4*QError[i]);
			qspreadsinthetai[13*i+10] = holder*(QRange[i]-0.4*QError[i]);
			qspreadsinthetai[13*i+11] = holder*(QRange[i]+0.2*QError[i]);
			qspreadsinthetai[13*i+12] = holder*(QRange[i]-0.2*QError[i]);

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

			if(qspreadsinthetai[13*i+2] < 0.0)
				MessageBox(NULL, L"Error in QSpread please contact the author", NULL,NULL);
		}
	}



	for (int l = 0; l < m_idatapoints; l++)
	{
		sinsquaredthetai[l] = sinthetai[l]*sinthetai[l];
	}

	for (int l = 0; l < 13*m_idatapoints; l++)
	{
		qspreadsinsquaredthetai[l] = qspreadsinthetai[l]*qspreadsinthetai[l];
	}

}


 void FastReflcalc::objective(double* par, double* x, int m, int n, void* data)
{
  FastReflcalc* reflinst = (FastReflcalc*)data;


  if(reflinst->onesigma == true)
	  reflinst->mkdensityonesigma(par,m);
  else
	  reflinst->mkdensity(par,m);
  
  reflinst->myrfdispatch();
  
  for(int i=0; i<n; ++i)
  {
	 x[i] = (log(reflinst->reflpt[i])-log(reflinst->Realrefl[i]));

	 //Sometimes we get NAN. We make that solution unpalatable until I can find a workaround
	 if(x[i] != x[i])
	 {
		 x[i] = 1e6;
	 }
	
  }

}

void FastReflcalc::mkdensityonesigma(double* p, int plength)
{
	//Move our parameters into individual arrays so they're easier to deal with
	double rhofactor = 1e-6*lambda*lambda/(2.0*M_PI);
	double SubRough = p[0];

	RhoArray[0] = m_dsupsld*rhofactor;
	LengthArray[0] = 0.0;
	for(int i = 1; i< boxnumber+1;i++)
	{
		LengthArray[i] = p[2*(i-1)+1];
		RhoArray[i] = p[2*(i-1)+2]*subphaseSLD*rhofactor;
		SigmaArray[i-1] = p[0];
	}

	RhoArray[boxnumber+1] = subphaseSLD*rhofactor;
	SigmaArray[boxnumber] = p[0];
	LengthArray[boxnumber+1] = 0;
	m_dnormfactor = p[plength-1];
}

void FastReflcalc::mkdensity(double* p, int plength)
{
	//Move our parameters into individual arrays so they're easier to deal with
    double rhofactor = 1e-6*lambda*lambda/(2.*M_PI);
	
	double sigfake[20];
	RhoArray[0] = m_dsupsld*rhofactor;
	LengthArray[0] = 0.0;
	for(int i = 1; i< boxnumber+1;i++)
	{
		LengthArray[i] = p[3*(i-1)+1];
		RhoArray[i] = p[3*(i-1)+2]*subphaseSLD*rhofactor;

		if(fabs(p[3*(i-1)+3]) < 1e-8)
			SigmaArray[i-1] = 1e-8;
		else
			SigmaArray[i-1] = p[3*(i-1)+3];
		
	}
	
	RhoArray[boxnumber+1] = subphaseSLD*rhofactor;
	SigmaArray[boxnumber] = p[0];
	LengthArray[boxnumber+1] = 0;

	m_dnormfactor = p[plength-1];
}

void FastReflcalc::mytransmultsigrf(double* sintheta, double* sinsquaredtheta, int datapoints, double* refl)
{
	//Generate the Parratt reflectivity layers
	int nl = boxnumber+2;
	double k0 = 2.0*M_PI/lambda;
	MyComplex imaginary(0.0,1.0);
	int offset = 0;

	double suprefindex = 1-RhoArray[0];
	double suprefindexsquared = suprefindex*suprefindex;
    double sigmacalc[20];

	int neg = 0;
    for(int i =0; i<datapoints;i++)
	{	
		
		for(int k = 1; k<nl;k++)
		{
			if((suprefindexsquared*sinsquaredtheta[i]-2.0*RhoArray[k]+2.0*RhoArray[0])<0)
			{
				neg -= 1;
				break;
			}
		}
		if(neg ==0)
		{
			/*if(m_dQSpread < 0.005 && neg == -5)
				break;*/
		}
		else
		{
			offset = i;
		}
	}
	
	MyComplex  lengthmultiplier = -2.0 * MyComplex (0.0,1.0) ;
	MyComplex  kk[20];
	double dkk[20];
	MyComplex  ak[20];
	double drj[20];
	MyComplex  rj[20];
	MyComplex  Rj[20];
	double dQj[20];
	MyComplex  Qj[20];

	//Boundary conditions
	Rj[nl-1] = 0.0;
	ak[0] = 1.0;
	ak[nl-1] = 1.0;

	MyComplex  doublenk[20];
	MyComplex  lengthcalc[20];
	

	//Move some calcs out of the loop
	for(int i = 0; i<nl;i++)
	{
		doublenk[i]=-2.0*MyComplex (RhoArray[i],0);
		lengthcalc[i] = lengthmultiplier*LengthArray[i];
		sigmacalc[i] = -2.0*SigmaArray[i]*SigmaArray[i];
	}

	for(int l = 0; l <= offset ;l++)
	{
		int nlminone = nl-1;
		kk[0] = k0*suprefindex* sintheta[l];
	
		//Workout the wavevector k
		for(int i = 1; i<nl;i++)
		{
			kk[i] = k0 *compsqrt(suprefindexsquared * sinsquaredtheta[l]+ doublenk[i] - doublenk[0]);
		}

		//Make the aj
		for(int i = 1; i<nlminone;i++)
		{
			ak[i] = compexp(kk[i]*lengthcalc[i]);
		}

		//Make the roughness correction term (Nevot-Croce)
		for(int i = 0; i< nlminone; i++)
		{
		#ifdef GIXOS
			Qj[i]=1;
		#else
			Qj[i] = compexp(sigmacalc[i]*kk[i]*kk[i+1]);
		#endif
		}
		
		//Make the Fresnel coefficients
		for(int i = 0; i <nlminone;i++)
		{
			rj[i] =Qj[i]* (kk[i]-kk[i+1])/(kk[i]+kk[i+1]);
		}
		
		
		for(int i = nl-2; i>=0;i--)
		{
			Rj[i] = ak[i]*(Rj[i+1]+rj[i])/(Rj[i+1]*rj[i]+1.0);
		}
		refl[l] = compabs(Rj[0]);
		refl[l] *= refl[l];
	}
	for(int l = offset+1; l< datapoints ;l++)
	{
		//Leave for vectorization
		int nlminone = nl-1;
		
		//Boundary conditions
		dkk[0] = k0*suprefindex*sintheta[l];
	
		//Workout the wavevector k
		for(int i = 1; i<nl;i++)
		{
			dkk[i] = k0 *sqrt(suprefindexsquared * sinsquaredtheta[l]+ doublenk[i].re - doublenk[0].re);
			
		}	

		//Make the aj
		for(int i = 1; i<nlminone;i++)
		{
			ak[i] = compexp(lengthcalc[i]*dkk[i]);
		}

		//Make the roughness correction term (Nevot-Croce)
		#pragma ivdep
		for(int i = 0; i< nlminone; i++)
		{
			#ifdef GIXOS
				dQj[i]=1;
			#else
				dQj[i] = exp(sigmacalc[i]*dkk[i]*dkk[i+1]);
			#endif
			
		}
		
		//Make the Fresnel coefficients
		for(int i = 0; i < nlminone;i++)
		{
			drj[i] =dQj[i]* (dkk[i]-dkk[i+1])/(dkk[i]+dkk[i+1]);
		}
	
		
		for(int i = nl-2; i>=0;i--)
		{
			Rj[i] = ak[i]*(Rj[i+1]+drj[i])/(Rj[i+1]*drj[i]+1.0);
		}

		refl[l] = compabs(Rj[0]);
		refl[l] *= refl[l];

	}
}   

void FastReflcalc::Rhocalculate(double Zoffset,double* ZIncrement, double* LengthArray, double* RhoArray, double* SigmaArray, double* nk, double* nkb, int loopcounter)
{
	//The code for this section is based on the electron density calculation
	//in Motofit (www.sourceforge.net/motofit). It is a standard method of calculating the
	//electron density profile. We treat the profile as having a user defined number of boxes
	//The last 30% of the curve will converge to have rho/rhoinf = 1.0. Currently, it is only
	//useful for the air-lipid-substrate interfaces. In order to allow for a substrate-lipid-substrate
	//model, set the superphaseSLD variable. Currently, the absorbance is not allowed to vary, however this can
	//be changed by linking it to the density genome. For lipid and lipid protein films, the absorbance is negligible
	//For films with large roughnesses, we allow the roughness of the air-film interface to vary

	
	int refllayers = boxnumber;
	double SuperphaseSLD = RhoArray[0];
	double deltarho = 0;
	double thick = 0;
	double roughness = 0;
	double dist = 0;
	double SubSLD = RhoArray[boxnumber+1];

	//Create arrays so we don't have to redo this calculation for every data point

	double* distarray = (double*)_aligned_malloc((refllayers+1)*sizeof(double),64);
	double* rhoarray = (double*)_aligned_malloc((refllayers+1)*sizeof(double),64);
	double* rougharray = (double*)_aligned_malloc((refllayers+1)*sizeof(double),64);

	//Calculate the portions of the e-density equation that don't need to be repeated
	
	for (int i = 0; i <= boxnumber; i++)
    {
          if (i == 0)
        {
            deltarho = RhoArray[1] - SuperphaseSLD;
            thick = 0;
            roughness = SigmaArray[0];
        }
        else if (i == refllayers)
        {
            deltarho = SubSLD - RhoArray[i];
			roughness = SigmaArray[i];
            thick = LengthArray[i];
        }
        else
        {
            deltarho = (RhoArray[i + 1] - RhoArray[i]);
            thick = LengthArray[i];
            roughness = SigmaArray[i];
        }

		dist  += thick;

		distarray[i] = dist;
		rhoarray[i] = deltarho;
		rougharray[i] = roughness;
	}
	
	double sqrt2 = sqrt(2.0);
	
	// This allows OpenMP to choose the appropriate number of threads
	// The algorithm should now scale with the number of processors in a system

	omp_set_num_threads(omp_get_num_threads());
	
	#pragma omp parallel for schedule(guided)
	for(int j = 0; j < loopcounter;j++)
	{
		double summ = RhoArray[0];
		double bsumm = RhoArray[0];
		
		for (int i = 0; i <= boxnumber; i++)
        {
		#ifdef GIXOS
			summ += (rhoarray[i] / 2.0) * (1.0 + erf((ZIncrement[j] - distarray[i]-Zoffset) / (1e-20 * sqrt2)));
		#else
			summ += (rhoarray[i] / 2.0) * (1.0 + erf((ZIncrement[j] - distarray[i]-Zoffset) / (rougharray[i] * sqrt2)));
		#endif
		}	
	
		for (int i = 0; i <= boxnumber; i++)
        {
			bsumm += (rhoarray[i] / 2.0) * (1.0 + erf((ZIncrement[j] - distarray[i]-Zoffset) / (1e-22 * sqrt2)));
		}	
		
		nk[j] = summ/SubSLD;
		
		nkb[j] = bsumm/SubSLD;
	}

	//Free arrays

	_aligned_free(distarray);
	_aligned_free(rhoarray);
	_aligned_free(rougharray);
}

double FastReflcalc::CalcQc(double dSLD)
{
    return 4 * sqrt(M_PI * (subphaseSLD- m_dsupsld) * 1e-6);
}

double FastReflcalc::CalcFresnelPoint(double Q, double Qc)
{
    if (Q <= Qc)
        return 1;
    else
    {
        double term1 = sqrt(1 - (Qc / Q)*(Qc/Q));
        return ((1.0 - term1) / (1.0 + term1))*((1.0 - term1) / (1.0 + term1));
    }
}


void FastReflcalc::QsmearRf(double* qspreadrefl, double* refl, int datapoints)
{
	double calcholder;

	
	for(int i = 0; i < datapoints; i++)
	{
		calcholder = 0;
		calcholder = qspreadrefl[13*i];
		calcholder += 0.056*qspreadrefl[13*i+1];
		calcholder += 0.056*qspreadrefl[13*i+2];
		calcholder += 0.135*qspreadrefl[13*i+3];
		calcholder += 0.135*qspreadrefl[13*i+4];
		calcholder += 0.278*qspreadrefl[13*i+5];
		calcholder += 0.278*qspreadrefl[13*i+6];
		calcholder += 0.487*qspreadrefl[13*i+7];
		calcholder += 0.487*qspreadrefl[13*i+8];
		calcholder += 0.726*qspreadrefl[13*i+9];
		calcholder += 0.726*qspreadrefl[13*i+10];
		calcholder += 0.923*qspreadrefl[13*i+11];
		calcholder += 0.923*qspreadrefl[13*i+12];

		refl[i] = calcholder/6.211;
	}
}

void FastReflcalc::myrfdispatch()
{
	if(m_dQSpread < 0.005)
	{
		mytransmultsigrf(sinthetai,sinsquaredthetai,m_idatapoints,reflpt);
	}
	else
	{
		mytransmultsigrf(qspreadsinthetai, qspreadsinsquaredthetai, m_idatapoints*13, qspreadreflpt);
		QsmearRf(qspreadreflpt, reflpt, m_idatapoints);
	}

	if(m_bImpNorm == TRUE)
	{
		ImpNorm(reflpt, m_idatapoints);
	}
}

void FastReflcalc::ImpNorm(double* refl, int datapoints)
{
		for(int i = 0; i < datapoints; i++)
		{
			refl[i] = refl[i] * m_dnormfactor;
		}
}