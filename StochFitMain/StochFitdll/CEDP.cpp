#include "stdafx.h"
#include "CEDP.h"

//    dz0= 1.0f/InitStruct->Resolution;
//	//Arrays for the electron density and twice the electron density
//    nk = (MyComplex*)_mm_malloc(sizeof(MyComplex)*nl,16);
//    doublenk = (MyComplex*)_mm_malloc(sizeof(MyComplex)*nl,16);
//
//
//	rho_a = InitStruct->FilmSLD * 1e-6 * lambda*lambda/(2.0f*M_PI);
//
//	if(InitStruct->UseSurfAbs == TRUE)
//	{
//		beta_a = InitStruct->FilmAbs * 1e-6 * lambda*lambda/(2.0f*M_PI);;
//		beta_sub = InitStruct->SupAbs * 1e-6 * lambda*lambda/(2.0f*M_PI);;
//		beta_sup = InitStruct->SupAbs * 1e-6 * lambda*lambda/(2.0f*M_PI);;
//	}
//	else
//	{
//		beta_a = beta_sub = beta_sup = 0;
//	}
//
//	//Create scratch arrays for the electron density calculation
//	distarray = (float*)_mm_malloc((InitStruct->Boxes+2)*sizeof(float),64);
//	rhoarray = (float*)_mm_malloc((InitStruct->Boxes+2)*sizeof(float),64);
//	imagrhoarray = (float*)_mm_malloc((InitStruct->Boxes+2)*sizeof(float),64);
//	edspacingarray = (float*)_mm_malloc(nl*sizeof(float),64);
//
//	for(int i = 0; i < nl; i++)
//	{
//		edspacingarray[i] = i*dz0-InitStruct->Leftoffset;
//	}
//
//	for(int k = 0; k < InitStruct->Boxes+2; k++)
//	{
//		distarray[k] = k*m_dboxsize;
//	}
//	//Set the total length of our surface layer - default 80 Angstroms of superphase,
//	//7 extra Angstroms of file, and 40 Angstroms of subphase
//	if(InitStruct->Totallength > 0)
//		totalsize = InitStruct->Totallength; 
//	else
//        totalsize = InitStruct->Leftoffset + InitStruct->FilmLength + 7 + 40;
//
//	nl= totalsize*InitStruct->Resolution;
//
//	m_dboxsize = (InitStruct->FilmLength+7.0)/(InitStruct->Boxes);
//
//	m_dwaveconstant = lambda*lambda/(2.0*M_PI);
//
////The code for the ED calculation section is loosely based on the electron density calculation
////in Motofit (www.sourceforge.net/motofit). It is a standard method of calculating the
////electron density profile. We treat the profile as having a user defined number of boxes
////The last 30% or so of the curve will converge to have rho/rhoinf = 1.0. 
////For lipid and lipid protein films, the absorbance is negligible
//
//
//void CReflCalc::mkdensitytrans(ParamVector* g)
//{
//	int refllayers = g->RealparamsSize()-1;
//	int reflpoints = nl;
//	float roughness = g->getroughness();
//	float dist;
//
//	if(g->getroughness() < 0.000000001)
//		roughness = 1e-6;
//
//	roughness = 1.0f/( roughness * sqrt(2.0f));
//	float supersld = g->GetRealparams(0)*rho_a;
//	
//	//Don't delete this, otherwise the reflectivity calculation won't work sometimes
//	nk[0].im = 0.0f;
//
//			
//		
//	
//		for(int k = 0; k < refllayers; k++)	
//		{
//			rhoarray[k] = rho_a*(g->GetRealparams(k+1)-g->GetRealparams(k))*0.5f;
//		}
//		
//
//		#pragma omp parallel for private(dist)
//		for(int i = 0; i < reflpoints; i++)
// 		{
//			nk[i].re = supersld;
//		
//			
//			for(int k = 0; k < refllayers; k++)
//			{
//				dist = (edspacingarray[i]-distarray[k] )*roughness;
//
//				if(dist > 6.0f)
//				{
//					nk[i].re += (rhoarray[k])*(2.0f);
//				}
//				else if(dist > -6.0f)
//				{
//					nk[i].re += (rhoarray[k])*(1.0f+erff(dist));
//				}
//			}
//
//			//Make double array for the reflectivity calculation
//			doublenk[i].re = 2.0f*nk[i].re;
//			doublenk[i].im = 0.0f;
//		}
//	
//
//		
//}
//
//void CReflCalc::mkdensity(ParamVector* g)
//{
//	int refllayers = g->RealparamsSize()-1;
//	
//	int reflpoints = nl;
//	float roughness = 1.0f/(g->getroughness() * sqrt(2.0));
//	float supersld = g->GetRealparams(0)*rho_a;
//	
//#pragma omp parallel
//	{
//		float dist;
//
//		#pragma omp for
//		for(int k = 0; k < refllayers; k++)
//		{
//			rhoarray[k] = rho_a*(g->GetRealparams(k+1)-g->GetRealparams(k))/2.0;
//			
//			//Imag calculation
//			if(k == 0)
//			{
//				imagrhoarray[k] = (beta_a* g->getSurfAbs() * g->GetRealparams(k+1)/g->GetRealparams(refllayers) - beta_sup)/2.0;
//			}
//			else if(k == refllayers-1)
//			{
//				imagrhoarray[k] = (beta_sub - beta_a* g->getSurfAbs() * g->GetRealparams(k)/g->GetRealparams(refllayers))/2.0;
//			}
//			else
//			{
//				imagrhoarray[k] = (beta_a* g->getSurfAbs() * g->GetRealparams(k+1)/g->GetRealparams(refllayers) - 
//					beta_a * g->getSurfAbs()* g->GetRealparams(k)/g->GetRealparams(refllayers))/2.0;
//			}
//		}
//
//		#pragma omp for private(dist)
//		for(int i = 0; i < reflpoints; i++)
// 		{
//			nk[i].im = beta_sup;
//			nk[i].re = supersld;
//			
//			
//			for(int k = 0; k < refllayers; k++)
//			{
//				dist = (edspacingarray[i]-distarray[k] )*roughness;
//
//				if(dist > 6)
//				{
//					nk[i].re += (rhoarray[k])*(2.0f);
//					nk[i].im += (imagrhoarray[k])*(2.0f);
//				}
//				else if (dist > -6)
//				{
//					nk[i].re += (rhoarray[k])*(1.0f+erff(dist));
//					nk[i].im += (imagrhoarray[k])*(1.0f+erff(dist));
//				}
//			}
//
//			doublenk[i].re = 2.0f*nk[i].re;
//			doublenk[i].im = 2.0f*nk[i].im;
//		}
//	}
//}
//
//if(UseAbs == TRUE)
//	{
//		for(int j=0; j < nl; j++)
//		{
//			rhoout << x << ' ' << nk[j].re/nk[nl-1].re << ' ' << nk[j].im/nk[nl-1].im << endl;
//			x += dz0;
//		}
//	}
//	else
//	{
//		for(int j=0; j < nl; j++) 
//		{
//			rhoout << x << ' ' << nk[j].re/nk[nl-1].re << endl;
//			x += dz0;
//		}
//	}