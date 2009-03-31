#include "stdafx.h"
#include "CEDP.h"

CEDP::~CEDP()
{
	_mm_free(m_cEDP);
	_mm_free(m_DEDP);
	_mm_free(m_dEDSpacingArray);
	_mm_free(m_dDistArray);
	_mm_free(m_dRhoArray);
	_mm_free(m_dImagRhoArray);
	_mm_free(m_dZ);
}

void CEDP::Initialize(ReflSettings* InitStruct)
{
    m_dDz0 = 1.0/static_cast<double>(InitStruct->Resolution);
	m_bUseSurfAbs = InitStruct->UseSurfAbs;
	m_dWaveConstant = InitStruct->Wavelength * InitStruct->Wavelength/(2.0 * PI);
	m_dRho = InitStruct->FilmSLD * 1e-6 * m_dWaveConstant;
	int FilmSlack = 7;

	//Set the total length of our surface layer - default 40 Angstroms of superphase,
	//7 extra Angstroms of file, and 40 Angstroms of subphase
	if(InitStruct->Totallength > 0)
	{
		m_iLayers = InitStruct->Totallength; 
	}
	else
	{
        m_iLayers = InitStruct->Leftoffset + InitStruct->FilmLength + FilmSlack + 40;
	}

	m_iLayers *= InitStruct->Resolution;

	if(InitStruct->UseSurfAbs)
	{
		m_dBeta = InitStruct->FilmAbs * m_dWaveConstant;
		m_dBeta_Sub = InitStruct->SubAbs * m_dWaveConstant;
		m_dBeta_Sup = InitStruct->SupAbs * m_dWaveConstant;
	}
	else
	{
		m_dBeta = m_dBeta_Sub = m_dBeta_Sup = 0.0;
	}

	//Arrays for the electron density profile and twice the electron density profile
    m_cEDP = (MyComplex*)_mm_malloc(sizeof(MyComplex)*m_iLayers,16);
    m_DEDP = (MyComplex*)_mm_malloc(sizeof(MyComplex)*m_iLayers,16);
	m_dEDSpacingArray = (double*)_mm_malloc(m_iLayers*sizeof(double),16);
	m_dZ = (double*)_mm_malloc(sizeof(double)*m_iLayers,16);
	//Create scratch arrays for the electron density calculation
	m_dDistArray = (double*)_mm_malloc((InitStruct->Boxes+2)*sizeof(double),16);
	m_dRhoArray = (double*)_mm_malloc((InitStruct->Boxes+2)*sizeof(double),16);
	m_dImagRhoArray = (double*)_mm_malloc((InitStruct->Boxes+2)*sizeof(double),16);
	

	for(int i = 0; i < m_iLayers; i++)
	{
		m_dZ[i] = i*m_dDz0;
		m_dEDSpacingArray[i] = i*m_dDz0-InitStruct->Leftoffset;
	}

	for(int k = 0; k < InitStruct->Boxes+2; k++)
	{
		m_dDistArray[k] = k*(InitStruct->FilmLength+static_cast<double>(FilmSlack))/InitStruct->Boxes;
	}
}

void CEDP::GenerateEDP(const ParamVector* g)
{
	if(!m_bUseSurfAbs)
		MakeTranparentEDP(g);
	else
		MakeEDP(g);
}

//The code for the ED calculation section is loosely based on the electron density calculation
//in Motofit (www.sourceforge.net/motofit). It is a standard method of calculating the
//electron density profile. We treat the profile as having a user defined number of boxes
//The last 30% or so of the curve will converge to have rho/rhoinf = 1.0. 
//For lipid and lipid protein films, the absorbance is negligible


void CEDP::MakeTranparentEDP(const ParamVector* g)
{
	double dist;
	int reflpoints = m_iLayers;
	int refllayers = g->RealparamsSize()-1;
	double roughness = g->GetRoughness();
	double supersld = g->GetRealparams(0)*m_dRho;


	if(g->GetRoughness() < 1e-6)
		roughness = 1e-6;

	roughness = 1.0/( roughness * sqrt(2.0));
	
	
	//Don't delete this, otherwise the reflectivity calculation won't work sometimes
	m_cEDP[0].im = 0.0;
	
	for(int k = 0; k < refllayers; k++)	
	{
		m_dRhoArray[k] = m_dRho*(g->GetRealparams(k+1)-g->GetRealparams(k))*0.5;
	}

	#pragma omp parallel for private(dist)
	for(int i = 0; i < reflpoints; i++)
 	{
		m_cEDP[i].re = supersld;
			
		for(int k = 0; k < refllayers; k++)
		{
			dist = (m_dEDSpacingArray[i]-m_dDistArray[k] )*roughness;

			if(dist > 6.0f)
				m_cEDP[i].re += (m_dRhoArray[k])*(2.0f);
			else if(dist > -6.0f)
				m_cEDP[i].re += (m_dRhoArray[k])*(1.0f+erff(dist));

			//Make double array for the reflectivity calculation
			m_DEDP[i].re = 2.0f*m_cEDP[i].re;
			m_DEDP[i].im = 0.0f;
		}
	}
}

void CEDP::MakeEDP(const ParamVector* g)
{
	double dist;
	int reflpoints = m_iLayers;
	int refllayers = g->RealparamsSize()-1;
	double roughness = g->GetRoughness();
	double supersld = g->GetRealparams(0)*m_dRho;


	if(g->GetRoughness() < 1e-6)
		roughness = 1e-6;

	roughness = 1.0/( roughness * sqrt(2.0));
	
	#pragma omp parallel
	{
		#pragma omp for
		for(int k = 0; k < refllayers; k++)
		{
			m_dRhoArray[k] = m_dRho*(g->GetRealparams(k+1)-g->GetRealparams(k))/2.0f;
			
			//Imag calculation
			if(k == 0)
			{
				m_dImagRhoArray[k] = (m_dBeta* g->GetSurfAbs() * g->GetRealparams(k+1)/g->GetRealparams(refllayers) - m_dBeta_Sup)/2.0f;
			}
			else if(k == refllayers-1)
			{
				m_dImagRhoArray[k] = (m_dBeta_Sub - m_dBeta * g->GetSurfAbs() * g->GetRealparams(k)/g->GetRealparams(refllayers))/2.0f;
			}
			else
			{
				m_dImagRhoArray[k] = (m_dBeta* g->GetSurfAbs() * g->GetRealparams(k+1)/g->GetRealparams(refllayers) - 
					(m_dBeta * g->GetSurfAbs()* g->GetRealparams(k)/g->GetRealparams(refllayers))/2.0f);
			}
		}

		#pragma omp for private(dist)
		for(int i = 0; i < reflpoints; i++)
 		{
			m_cEDP[i].im = m_dBeta;
			m_cEDP[i].re = supersld;
			
			
			for(int k = 0; k < refllayers; k++)
			{
				dist = (m_dEDSpacingArray[i]-m_dDistArray[k] )*roughness;

				if(dist > 6)
				{
					m_cEDP[i].re += (m_dRhoArray[k])*(2.0f);
					m_cEDP[i].im += (m_dImagRhoArray[k])*(2.0f);
				}
				else if (dist > -6)
				{
					m_cEDP[i].re += (m_dRhoArray[k])*(1.0f+erff(dist));
					m_cEDP[i].im += (m_dImagRhoArray[k])*(1.0f+erff(dist));
				}
			}
		
			m_DEDP[i].re = 2.0f*m_cEDP[i].re;
			m_DEDP[i].im = 2.0f*m_cEDP[i].im;
		}
	}
}


double CEDP::Get_LayerThickness() const
{
	return m_dDz0;
}

int CEDP::Get_EDPPointCount() const
{
	return m_iLayers;
}

bool CEDP::Get_UseABS() const
{
	return m_bUseSurfAbs;
}

double CEDP::Get_FilmAbs() const
{
	return m_dBeta;
}

double CEDP::Get_WaveConstant() 
{
	return m_dWaveConstant;
}

void CEDP::Set_FilmAbs(double absorption) 
{
	m_dBeta = absorption*m_dWaveConstant;
}

//Check to see if there is any negative electron density for the XR case, false if there is neg ED
bool CEDP::CheckForNegDensity()
{
	int EDPoints = m_iLayers;
	
	#pragma ivdep
	for(int i = 0; i < EDPoints; i++)
	{
		if(m_cEDP[i].re < 0)
			return false;
	}

	return true;
}

void CEDP::WriteOutputFile(wstring filename)
{
	ofstream rhoout(filename.c_str());

	for(int j = 0; j < m_iLayers; j++)
	{
		rhoout << m_dZ[j] << ' ' << m_cEDP[j].re/m_cEDP[m_iLayers-1].re << ' ' ;
		
		if(m_bUseSurfAbs)
			rhoout << m_cEDP[j].im/m_cEDP[m_iLayers-1].im;
		
		rhoout << endl;
	}
	
	rhoout.close();
}

void CEDP::GetData(double* Z, double* EDP)
{
	memcpy(Z,m_dZ, sizeof(double)*m_iLayers);

	for(int i = 0; i < m_iLayers; i++)
	{
		EDP[i] = m_cEDP[i].re;
	}
} 

MyComplex* CEDP::GetDoubledEDP() const
{
	return m_DEDP;
}

double* CEDP::GetZ() const
{
	return m_dZ;
}