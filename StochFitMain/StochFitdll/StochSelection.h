#pragma once

class StochSelection
{
private:
	ofstream* outtestfile;
	double oldscore, score0,mc_step;
	int oldscorecount;
	int executioncount;
public:
	StochSelection();
	void Init(GARealGenome* genome1, multilayer* ml0);
	double StochasticSelection(GARealGenome* g, multilayer* ml, double fuzzylength, double jmax);
	~StochSelection();
	bool m_bthreadstop;
};
