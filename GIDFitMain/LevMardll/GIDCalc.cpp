#include "stdafx.h"
#include "GIDCalc.h"

//To avoid error message
bool GIDCalc::m_bwarnedonce = false;

void GIDCalc::Init(int FuncSize, double* Q, int QSize, double* RealGID, double* RealGIDErrors, double* ModelGID, double* IndividGraphs, double* parameters)
{
	m_iFuncNum = FuncSize;
	m_dQ = Q;
	m_iQSize = QSize;

	if(RealGID != NULL)
	{
		m_dRealGID = RealGID;
		m_dRealGIDErrors = RealGIDErrors;
	}
	
	m_dModelGID = ModelGID;
	m_dIndividGraphs = IndividGraphs;
	
	//Trick the ls routines into not changing the type of distribution
	m_dfuncholder = new double[FuncSize];
	for(int i = 0; i < FuncSize; i++)
	{
		m_dfuncholder[i] = parameters[5*i+2]; 
	}
	m_bfitting = false;

}

GIDCalc::~GIDCalc()
{
	//Free arrays
	delete[] m_dfuncholder;
}

 void GIDCalc::objective(double* par, double* x, int m, int n, void* data)
{
  
  GIDCalc* GIDinst = (GIDCalc*)data;
  GIDinst->m_bfitting = true;
  GIDinst->MakeGID(par,m);

  //This gets squared in the LM routines
  for(int i=0; i<GIDinst->m_iQSize; ++i)
  {
	  x[i] = GIDinst->m_dModelGID[i]-GIDinst->m_dRealGID[i];	
  }

  //Reset the distribution types that the LS routines may have changed
  for(int i = 0; i < GIDinst->m_iFuncNum; i++)
  {
	  par[5*i+2] = GIDinst->m_dfuncholder[i];
  }
}

 void GIDCalc::MakeGaussian(double position, double sigma, double intensity, double* outarray)
 {
	int length = m_iQSize;

	if(sigma == 0)
		sigma = 1e-16;

	double sigfact = -1/(2.0*sigma*sigma);
	double posshift;

	#pragma ivdep
	for(int i = 0; i < length; i++)
	{
		posshift = m_dQ[i]-position;
		posshift *= posshift;
		outarray[i] = intensity*exp(posshift*sigfact);
	}
 }

 void GIDCalc::MakeLorentzian(double position, double gamma, double intensity, double* outarray)
 {
	int length = m_iQSize;
	if(gamma == 0)
		gamma = 1e-16;
	/*
	double gammasquared = 0.25*gamma*gamma;
	double normfactor = 0.5*gamma/2.0;*/

	double gammasquared = gamma*gamma;
	double normfactor = 0.5*gamma/2.0;
	double posshift;
	
	#pragma ivdep
	for(int i = 0; i < length; i++)
	{
		posshift = m_dQ[i]-position;
		posshift *= posshift;
		outarray[i] = intensity*gammasquared/(posshift+gammasquared);
	}
 }

 void GIDCalc::MakeVoigt(double position, double sigma, double gamma, double intensity, double* outarray)
 {
	 
	 //Use the real portion of the Fadeeva function (complex error function) to calculate the Voigt profile
	 if(gamma == 0)
		 gamma = 1e-16;

	 if(sigma == 0)
		 sigma = 1e-16;

	 
	 MyComplex c;
	
	 double denom = 1.0/(sigma*sqrt(2.0));
	 double pos;
	 MyComplex norm(0.0,gamma*denom);
	 double normfactinv = cerf(norm).re;
	 double normfact = 1.0/cerf(norm).re;
	 
	 for(int j = 0; j < m_iQSize; j++)
	 {
		 //Take advantage of the fact that the profile is symmetric
		 pos = fabs(m_dQ[j]-position);
		 c.re = pos*denom;
		 c.im = gamma*denom;
	
		 //Make negative gamma's very unpopular. This is necessary, as our Fadeeva function is only
		 //valid for the first and fourth quadrants and the origin.
		 if(sigma < 0 || gamma < 0) 
		 {
			outarray[j] = -1e6;
		 }
		 else if (normfactinv > 0 && gamma > 0)
		 {
			 outarray[j] = normfact*intensity*cerf(c).re;
			 m_bwarnedonce = false;
		 }
		 else if (normfactinv  == 0)
		 {
			 if(m_bfitting == false && m_bwarnedonce == false)
			 {
				MessageBox(NULL, L"The sigma value has dropped below resolvable levels (~8e-4) - Lorentzian will be used", NULL,NULL);
				m_bwarnedonce = true;
			 }

			 MakeLorentzian(position, gamma, intensity, outarray);
			 break;
		 }
		 else
		 {
			 MessageBox(NULL,L"Error",NULL,NULL);
			 break;
		 }

	 }
 }

 //Parse the parameter list from the main program
 void GIDCalc::MakeGID(double* parameters, int paramsize)
 {
	//Parse parameters
	 double slope = parameters[0];
	 double offset = parameters[1];

	 //Make all of our curves
	 for(int i = 0; i < m_iFuncNum; i++)
	 {
		if(m_dfuncholder[i] == 0)
		{
			MakeGaussian(parameters[5*i+3+2],parameters[5*i+2+2],parameters[5*i+1+2],m_dIndividGraphs+i*m_iQSize);
		}
		else if(m_dfuncholder[i] == 1)
		{
			MakeLorentzian(parameters[5*i+3+2],parameters[5*i+2+4],parameters[5*i+1+2],m_dIndividGraphs+i*m_iQSize);
		}
		else if(m_dfuncholder[i] == 2)
		{
			MakeVoigt(parameters[5*i+3+2],parameters[5*i+2+2], parameters[5*i+4+2], parameters[5*i+1+2],m_dIndividGraphs+i*m_iQSize);
		}
	 }

	 //Put all of our curves into the correct arrays
	 if(m_iFuncNum > 1)
	 {
		for(int j = 0; j< m_iQSize; j++)
		{
		 m_dModelGID[j] = 0;
		 for(int i = 0; i < m_iFuncNum; i++)
		 {
			m_dModelGID[j] += m_dIndividGraphs[j+i*m_iQSize];
		 }
		}

		for(int j= 0; j< m_iQSize;j++)
		{
			m_dModelGID[j] = m_dModelGID[j]+/*slope*m_dQ[j]+*/offset;
		}
		 //Change slope and offset - not very happy with how the slope is turning out, needs work
		 for(int i = 0; i < m_iQSize*m_iFuncNum; i++)
		 {
			 m_dIndividGraphs[i] = m_dIndividGraphs[i]+/*slope*m_dQ[i%m_iQSize]+*/offset;
		 }
	 }
	 else
	 {
		 //Change slope and offset
		 for(int i = 0; i < m_iQSize*m_iFuncNum; i++)
		 {
			 m_dIndividGraphs[i] = m_dIndividGraphs[i]/*+slope*m_dQ[i]*/+offset;
		 }
		 memcpy(m_dModelGID, m_dIndividGraphs, sizeof(double)*m_iQSize);
	 }
 }

void GIDCalc::writefiles(const char* filename)
{
	std::ofstream outrhofile("gidfit.dat");
	for(int i = 0; i<m_iQSize;i++)
	{
		outrhofile << m_dQ[i] << ' ' << m_dRealGID[i] << ' ' << m_dRealGIDErrors[i] << ' ' << m_dModelGID[i] << std::endl;
	}
	outrhofile.close();

	std::ofstream outindividfile("individgraphs.dat");
	for(int j = 0; j < m_iQSize; j++)
	{
		for(int i = 0; i < m_iFuncNum; i++)
		{
			outindividfile << (m_dIndividGraphs+i*m_iQSize)[j] << ' ';
		}
		outindividfile << std::endl;
	}
	outindividfile.close();
}

double GIDCalc::CalcChiSquare()
{
	double chisquare; 

	//Calculate ChiSquare - A little weird, since we aren't centered at zero
	for(int i = 0; i < m_iQSize;i++)
	{
		if(m_dRealGIDErrors[i] > 0)
			chisquare += ((m_dRealGID[i]-m_dModelGID[i])*(m_dRealGID[i]-m_dModelGID[i]))/(m_dRealGIDErrors[i] * m_dRealGIDErrors[i]);
	}
	
	return chisquare;
}