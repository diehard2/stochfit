#include "stdafx.h"
#include "FastReflCalc.h"
#include "AreaFastReflCalc.h"

void AreaFastReflCalc::mkdensityonesigma(double* p, int plength)
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

void AreaFastReflCalc::mkdensity(double* p, int plength)
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

void AreaFastReflCalc::CalcRefl(double* sintheta, double* sinsquaredtheta, int datapoints, double* refl)
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

void AreaFastReflCalc::Rhocalculate(double Zoffset,double* ZIncrement, double* LengthArray, double* RhoArray, double* SigmaArray, double* nk, double* nkb, int loopcounter)
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