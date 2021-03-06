#include "stdafx.h"
#include "CObjectiveFunc.h"


CObjective::~CObjective()
{
	_mm_free(m_dData);
	_mm_free(m_dDataError);
}

void CObjective::Initialize(const ReflSettings* InitStruct)
{
	m_iFunction = InitStruct->Objectivefunction;
	m_iDataPoints = InitStruct->QPoints-InitStruct->HighQOffset - InitStruct->CritEdgeOffset;

	m_dData = (double*)_mm_malloc(m_iDataPoints * sizeof(double), 16);
	m_dDataError = (double*)_mm_malloc(m_iDataPoints*sizeof(double), 16);
	memcpy(m_dData, (void*)(InitStruct->Refl + InitStruct->CritEdgeOffset), sizeof(double)* m_iDataPoints);
	memcpy(m_dDataError, (void*)(InitStruct->ReflError + InitStruct->CritEdgeOffset), sizeof(double)*m_iDataPoints);
}


double CObjective::CalculateFitScore(const double* generateddata)
{
	switch(m_iFunction)
	{
		case 0:	
			return LogLS(generateddata);
		case 1:
			return InverseLS(generateddata);
		case 2:
			return LogErrorLS(generateddata);
		case 3:
			return InverseErrorLS(generateddata);
		default:
			return LogLS(generateddata);
	}
}

//Index 1
double CObjective::LogLS(const double* Exp)
{
	double goodnessoffit = 0;
	double calcholder = 0;
	int counter = m_iDataPoints;

	for(int i = 0; i< counter; i++)
	{
		calcholder = log(m_dData[i])-log(Exp[i]);
		goodnessoffit += calcholder*(calcholder);
	}

	return goodnessoffit/(double)counter;
}

//Index 3
double CObjective::LogErrorLS(const double* Exp)
{
	double goodnessoffit = 0;
	double calcholder = 0;
	int counter = m_iDataPoints;

	for(int i = 0; i< counter; i++)
	{
		calcholder = log(m_dData[i])-log(Exp[i]);
		goodnessoffit += calcholder*calcholder/fabs(log(m_dDataError[i]));
	}

	return goodnessoffit/(double)counter;
}

//Index 2
double CObjective::InverseLS(const double* Exp)
{
		double goodnessoffit = 0;
		double calcholder = 0;
		int counter = m_iDataPoints;

		for(int i = 0; i< counter; i++)
		{
			calcholder = m_dData[i]/Exp[i];
			
			if(calcholder < 1.0)
				calcholder = 1.0/calcholder;
	
			goodnessoffit +=(1.0-calcholder)*(1.0-calcholder);
		}

		return goodnessoffit/(double)counter;
}

//Index 4
double CObjective::InverseErrorLS(const double* Exp)
{
		double goodnessoffit = 0;
		double calcholder = 0;
		double errormap = 0;
		int counter = m_iDataPoints;

		for(int i = 0; i< counter; i++)
		{
			calcholder = m_dData[i]/Exp[i];
			
			if(calcholder < 1.0)
				calcholder = 1.0/calcholder;
			
			errormap = (m_dData[i]/m_dDataError[i])*(m_dData[i]/m_dDataError[i]);
			goodnessoffit +=(1.0-calcholder)*(1.0-calcholder)*errormap;
		}

		return goodnessoffit/(double)counter;
}

double CObjective::ChiSquare(const double* Exp)
{
	double ChiSquare = 0;
	double calcholder = 0;
	int counter = m_iDataPoints;

	//Calculate the Chi Square (reduced with no parameters)
	for(int i= 0; i< counter; i++)
	{
		calcholder = (m_dData[i]-Exp[i]);
		ChiSquare += (calcholder*calcholder)/(m_dDataError[i]*m_dDataError[i]);
	}

	return ChiSquare/(double)counter;
}