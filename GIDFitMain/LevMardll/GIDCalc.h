#pragma once

class GIDCalc
{
private:
	double* m_dRealGID, *m_dRealGIDErrors, *m_dQ, *m_dIndividGraphs,* m_dfuncholder;
	int m_iQSize;
	int m_iFuncNum;
	double *m_dModelGID;
	bool m_bfitting;
	static bool m_bwarnedonce;

	void MakeGaussian(double position, double sigma, double intensity, double* outarray);
	void MakeLorentzian(double position, double gamma, double intensity, double* outarray);
	void MakeVoigt(double position, double sigma, double gamma, double intensity, double* outarray);

public:	
	//Member functions
	~GIDCalc();
    void Init(int FuncSize, double* Q, int QSize, double* RealGID, double* RealGIDErrors, double* ModelGID, double* IndividGraphs, double* params);
	void MakeGID(double* parameters, int paramsize);
	static void objective(double *p, double *x, int m, int n, void *data);
	void writefiles(const char* filename);
	double CalcChiSquare();

};

