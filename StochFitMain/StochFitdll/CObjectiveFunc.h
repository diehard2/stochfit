#pragma once

class CObjective
{
private:
	double* m_dData;
	double* m_dDataError;
	int m_iFunction;
	int m_iDataPoints;

	double LogLS(const double* Exp);
	double LogErrorLS(const double* Exp);
	double InverseLS(const double* Exp);
	double InverseErrorLS(const double* Exp);
public:

	~CObjective();
	void Initialize(const ReflSettings* InitStruct);
	double ChiSquare(const double* Exp);
	double CalculateFitScore(const double* func);


};