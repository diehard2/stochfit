#include "platform.h"
#include "CEDP.h"

void CEDP::Init(ReflSettings* InitStruct)
{
    if(InitStruct->Resolution <= 0) InitStruct->Resolution = 3;
    m_dDz0 = 1.0/InitStruct->Resolution;
	m_dLambda = InitStruct->Wavelength;
	m_bUseSurfAbs = InitStruct->UseSurfAbs;
	m_dWaveConstant = m_dLambda*m_dLambda/(2.0*std::numbers::pi);
	m_dRho = InitStruct->FilmSLD * 1e-6 * m_dWaveConstant;
	// Safe boundary offsets: 40 Å covers 5× the maximum roughness bound (8 Å).
	// FilmSlack adds 7 Å past the last box so the substrate erf tail fully converges.
	const double leftOffset = 40.0;
	const double substrateOffset = 40.0;
	const int FilmSlack = 7;

	m_iLayers = static_cast<int>(leftOffset + InitStruct->FilmLength + FilmSlack + substrateOffset);
	m_iLayers *= InitStruct->Resolution;


	if(InitStruct->UseSurfAbs != 0)
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
    m_EDP.resize(m_iLayers);
    m_DEDP.resize(m_iLayers);
	m_fEDSpacingArray.resize(m_iLayers);

	//Create scratch arrays for the electron density calculation
	m_fDistArray.resize(InitStruct->Boxes+2);
	m_fRhoArray.resize(InitStruct->Boxes+2);
	m_fImagRhoArray.resize(InitStruct->Boxes+2);


	for(int i = 0; i < m_iLayers; i++)
	{
		m_fEDSpacingArray[i] = i*m_dDz0-leftOffset;
	}

	for(int k = 0; k < InitStruct->Boxes+2; k++)
	{
		m_fDistArray[k] = k*(InitStruct->FilmLength+(double)FilmSlack)/InitStruct->Boxes;;
	}
}

void CEDP::GenerateEDP(ParamVector* g)
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


void CEDP::MakeTranparentEDP(ParamVector* g)
{
	double dist;
	int reflpoints = m_iLayers;
	int refllayers = g->RealparamsSize()-1;
	double roughness = g->getroughness();
	double supersld = g->GetRealparams(0)*m_dRho;


	if(g->getroughness() < 1e-6)
		roughness = 1e-6;

	roughness = 1.0/( roughness * sqrt(2.0));


	//Don't delete this, otherwise the reflectivity calculation won't work sometimes
	m_EDP[0].imag(0.0);

	for(int k = 0; k < refllayers; k++)
	{
		m_fRhoArray[k] = m_dRho*(g->GetRealparams(k+1)-g->GetRealparams(k))*0.5;
	}

	#pragma omp parallel for private(dist)
	for(int i = 0; i < reflpoints; i++)
 	{
		m_EDP[i].real(supersld);

		for(int k = 0; k < refllayers; k++)
		{
			dist = (m_fEDSpacingArray[i]-m_fDistArray[k] )*roughness;

			if(dist > 6.0)
				m_EDP[i] += (m_fRhoArray[k])*(2.0);
			else if(dist > -6.0)
				m_EDP[i] += (m_fRhoArray[k])*(1.0+erf(dist));

			//Make double array for the reflectivity calculation
			m_DEDP[i] = 2.0 * m_EDP[i].real();
		}
	}
}

void CEDP::MakeEDP(ParamVector* g)
{
	double dist;
	int reflpoints = m_iLayers;
	int refllayers = g->RealparamsSize()-1;
	double roughness = g->getroughness();
	double supersld = g->GetRealparams(0)*m_dRho;


	if(g->getroughness() < 1e-6)
		roughness = 1e-6;

	roughness = 1.0/( roughness * sqrt(2.0));

	#pragma omp parallel
	{
		#pragma omp for
		for(int k = 0; k < refllayers; k++)
		{
			m_fRhoArray[k] = m_dRho*(g->GetRealparams(k+1)-g->GetRealparams(k))/2.0;

			//Imag calculation
			if(k == 0)
			{
				m_fImagRhoArray[k] = (m_dBeta* g->getSurfAbs() * g->GetRealparams(k+1)/g->GetRealparams(refllayers) - m_dBeta_Sup)/2.0;
			}
			else if(k == refllayers-1)
			{
				m_fImagRhoArray[k] = (m_dBeta_Sub - m_dBeta * g->getSurfAbs() * g->GetRealparams(k)/g->GetRealparams(refllayers))/2.0;
			}
			else
			{
				m_fImagRhoArray[k] = (m_dBeta* g->getSurfAbs() * g->GetRealparams(k+1)/g->GetRealparams(refllayers) -
					(m_dBeta * g->getSurfAbs()* g->GetRealparams(k)/g->GetRealparams(refllayers))/2.0);
			}
		}

		#pragma omp for private(dist)
		for(int i = 0; i < reflpoints; i++)
 		{
			m_EDP[i] = std::complex<double>(supersld, m_dBeta);

			for(int k = 0; k < refllayers; k++)
			{
				dist = (m_fEDSpacingArray[i]-m_fDistArray[k] )*roughness;

				if(dist > 6)
				{
					m_EDP[i] += std::complex<double>((m_fRhoArray[k])*(2.0), (m_fImagRhoArray[k])*(2.0));
				}
				else if (dist > -6)
				{
					m_EDP[i] += std::complex<double>((m_fRhoArray[k])*(1.0+erf(dist)), (m_fImagRhoArray[k])*(1.0+erf(dist)));
				}
			}

			m_DEDP[i] = 2.0 * m_EDP[i];
		}
	}
}

int CEDP::Get_EDPPointCount()
{
	return m_iLayers;
}

bool CEDP::Get_UseABS()
{
	return m_bUseSurfAbs;
}

double CEDP::Get_Dz()
{
	return m_dDz0;
}

double CEDP::Get_FilmAbs()
{
	return m_dBeta;
}

double CEDP::Get_FilmAbsInput()
{
	// Returns the value that, when passed to Set_FilmAbs(), reproduces m_dBeta.
	// Set_FilmAbs(x) stores x * m_dWaveConstant, so x = m_dBeta / m_dWaveConstant.
	return (m_dWaveConstant > 0.0) ? m_dBeta / m_dWaveConstant : 0.0;
}

double CEDP::Get_WaveConstant()
{
	return m_dWaveConstant;
}

void CEDP::Set_FilmAbs(double abs)
{
	m_dBeta = abs*m_dWaveConstant;
}
