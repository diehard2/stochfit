#pragma once

class CObjective
{
private:
	double* m_dData;
	double* m_dDataError;
	int m_iFunction;
	int m_iDataPoints;

	double LogLS(double* Exp);
	double LogErrorLS(double* Exp);
	double InverseLS(double* Exp);
	double InverseErrorLS(double* Exp);
public:
	typedef double (CObjective::*func)(double*);

	CObjective();
	void Initialize(ReflSettings* InitStruct);
	double ChiSquare(double* Exp);
	double GetFunction(double* func);


};