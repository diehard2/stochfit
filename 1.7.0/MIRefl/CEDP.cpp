#include "stdafx.h"
#include "CEDP.h"

CEDP::~CEDP()
{
	if(m_EDP != NULL)
	{
		_mm_free(m_EDP);
		_mm_free(m_DEDP);
		_mm_free(m_fEDSpacingArray);
		_mm_free(m_fDistArray);
		_mm_free(m_BoxRho);
		_mm_free(m_fRhoArray);
		_mm_free(m_fImagRhoArray);
	}
}

CEDP::CEDP():m_EDP(NULL)
{

}

void CEDP::Init(ReflSettings* InitStruct, double SLD[], double Thickness[], double roughness)
{
    m_dDz0= 1.0f/InitStruct->Resolution;
	m_dLambda = InitStruct->Wavelength;
	m_bUseSurfAbs = InitStruct->UseSurfAbs;
	m_dWaveConstant = m_dLambda*m_dLambda/(2.0*M_PI);
	m_dRho = InitStruct->FilmSLD * 1e-6 * m_dWaveConstant;
	m_Roughness = roughness;
	m_Boxes = InitStruct->Boxes;
	//Set the total length of our surface layer - default 40 Angstroms of superphase,
	//7 extra Angstroms of file, and 40 Angstroms of subphase
	if(InitStruct->Totallength > 0)
		m_iLayers = InitStruct->Totallength; 
	else
        m_iLayers = InitStruct->Leftoffset + InitStruct->FilmLength + 40;

	m_iLayers *= InitStruct->Resolution;


	if(InitStruct->UseSurfAbs == TRUE)
	{
		m_dBeta = InitStruct->FilmAbs * m_dWaveConstant;
		m_dBeta_Sub = InitStruct->SubAbs * m_dWaveConstant;
		m_dBeta_Sup = InitStruct->SupAbs * m_dWaveConstant;
	}
	else
	{
		m_dBeta = m_dBeta_Sub = m_dBeta_Sup = 0;
	}

	//Arrays for the electron density profile and twice the electron density profile
    m_EDP = (MyComplex*)_mm_malloc(sizeof(MyComplex)*m_iLayers,64);
    m_DEDP = (MyComplex*)_mm_malloc(sizeof(MyComplex)*m_iLayers,64);
	m_fEDSpacingArray = (float*)_mm_malloc(m_iLayers*sizeof(float),64);

	//Create scratch arrays for the electron density calculation
	m_fDistArray = (float*)_mm_malloc((InitStruct->Boxes+5)*sizeof(float),64);
	m_BoxRho = (float*)_mm_malloc((InitStruct->Boxes+5)*sizeof(float),64);
	m_fRhoArray = (float*)_mm_malloc((InitStruct->Boxes+5)*sizeof(float),64);
	m_fImagRhoArray = (float*)_mm_malloc((InitStruct->Boxes+5)*sizeof(float),64);
	memset(m_fDistArray, 0, (InitStruct->Boxes+5)*sizeof(float));

	for(int i = 0; i < m_iLayers; i++)
	{
		m_fEDSpacingArray[i] = i*m_dDz0-InitStruct->Leftoffset;
	}

	for(int k = 0; k < InitStruct->Boxes+2; k++)
	{
		if(k == 0)
			m_fDistArray[k] = 0;
		else 
			m_fDistArray[k] = m_fDistArray[k-1]+Thickness[k];
	}

	for(int k = 0; k < InitStruct->Boxes+2; k++)
	{
		m_BoxRho[k] = SLD[k] * 1e-6 * m_dWaveConstant;
	}
}

void CEDP::GenerateEDP()
{
	if(m_bUseSurfAbs == FALSE)
		MakeTranparentEDP();
	else
		MakeEDP();
}


void CEDP::MakeTranparentEDP()
{
	
	int reflpoints = m_iLayers;
	int refllayers = m_Boxes+2-1;
	float supersld = m_BoxRho[0];
	double roughness = m_Roughness;
	float dist;
	roughness = 1.0f/( roughness * sqrt(2.0f));
	
	
	//Don't delete this, otherwise the reflectivity calculation won't work sometimes
	m_EDP[0].im = 0.0f;
	
	for(int k = 0; k < refllayers; k++)	
	{
		m_fRhoArray[k] = (m_BoxRho[k+1]-m_BoxRho[k])*0.5f;

	}

	#pragma omp parallel for private(dist)
	for(int i = 0; i < reflpoints; i++)
 	{
		
		m_EDP[i].re = supersld;
		m_EDP[i].im = 0.0f;
			
		for(int k = 0; k < refllayers; k++)
		{
			dist = (m_fEDSpacingArray[i]-m_fDistArray[k] )*roughness;

			if(dist > 6.0f)
				m_EDP[i].re += (m_fRhoArray[k])*(2.0f);
			else if(dist > -6.0f)
				m_EDP[i].re += (m_fRhoArray[k])*(1.0f+erff(dist));

			//Make double array for the reflectivity calculation
			m_DEDP[i].re = 2.0f*m_EDP[i].re;
			m_DEDP[i].im = 0.0f;
		}
	}
}

void CEDP::MakeEDP()
{
//	float dist;
//	int reflpoints = m_iLayers;
//	int refllayers = g->RealparamsSize()-1;
//	float roughness = g->getroughness();
//	float supersld = g->GetRealparams(0)*m_dRho;
//
//
//	if(g->getroughness() < 1e-6)
//		roughness = 1e-6;
//
//	roughness = 1.0f/( roughness * sqrt(2.0f));
//	
//	#pragma omp parallel
//	{
//		#pragma omp for
//		for(int k = 0; k < refllayers; k++)
//		{
//			m_fRhoArray[k] = m_dRho*(g->GetRealparams(k+1)-g->GetRealparams(k))/2.0f;
//			
//			//Imag calculation
//			if(k == 0)
//			{
//				m_fImagRhoArray[k] = (m_dBeta* g->getSurfAbs() * g->GetRealparams(k+1)/g->GetRealparams(refllayers) - m_dBeta_Sup)/2.0f;
//			}
//			else if(k == refllayers-1)
//			{
//				m_fImagRhoArray[k] = (m_dBeta_Sub - m_dBeta * g->getSurfAbs() * g->GetRealparams(k)/g->GetRealparams(refllayers))/2.0f;
//			}
//			else
//			{
//				m_fImagRhoArray[k] = (m_dBeta* g->getSurfAbs() * g->GetRealparams(k+1)/g->GetRealparams(refllayers) - 
//					(m_dBeta * g->getSurfAbs()* g->GetRealparams(k)/g->GetRealparams(refllayers))/2.0f);
//			}
//		}
//
//		#pragma omp for private(dist)
//		for(int i = 0; i < reflpoints; i++)
// 		{
//			m_EDP[i].im = m_dBeta;
//			m_EDP[i].re = supersld;
//			
//			
//			for(int k = 0; k < refllayers; k++)
//			{
//				dist = (m_fEDSpacingArray[i]-m_fDistArray[k] )*roughness;
//
//				if(dist > 6)
//				{
//					m_EDP[i].re += (m_fRhoArray[k])*(2.0f);
//					m_EDP[i].im += (m_fImagRhoArray[k])*(2.0f);
//				}
//				else if (dist > -6)
//				{
//					m_EDP[i].re += (m_fRhoArray[k])*(1.0f+erff(dist));
//					m_EDP[i].im += (m_fImagRhoArray[k])*(1.0f+erff(dist));
//				}
//			}
//		
//			m_DEDP[i].re = 2.0f*m_EDP[i].re;
//			m_DEDP[i].im = 2.0f*m_EDP[i].im;
//		}
//	}
}

void CEDP::WriteOutputFile(wstring filename)
{
	double z = 0;
	ofstream rhoout(filename.c_str());

	for(int j = 0; j < m_iLayers; j++)
	{
		rhoout << z << ' ' << m_EDP[j].re/m_EDP[m_iLayers-1].re << ' ' ;
		
		if(m_bUseSurfAbs == TRUE)
			rhoout << m_EDP[j].im/m_EDP[m_iLayers-1].im;
		
		rhoout << endl;

		z += m_dDz0;
	}
	
	rhoout.close();
}

double CEDP::Get_LayerThickness()
{
	return m_dDz0;
}

int CEDP::Get_EDPPointCount()
{
	return m_iLayers;
}

BOOL CEDP::Get_UseABS()
{
	return m_bUseSurfAbs;
}

double CEDP::Get_Dz()
{
	return m_dDz0;
}