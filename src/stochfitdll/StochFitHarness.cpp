/*
 *	Copyright (C) 2008 Stephen Danauskas
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */


#include <stochfit/common/platform.h>
#include "ParamVector.h"
#include "ReflCalc.h"
#include "SimulatedAnnealing.h"
#include "StochFitHarness.h"

#if STOCHFIT_HAS_GPU
#include "gpu/gpu_detect.h"
#include "gpu/gpu_sa_runner.h"
#endif

StochFit::StochFit(ReflSettings* InitStruct)
{
	m_bupdated = false;
	m_bwarmedup = false;
	m_ipriority = 2;

	m_Directory = string(InitStruct->Directory);

    fnpop = m_Directory + "/pop.dat";
    fnrf  = m_Directory + "/rf.dat";
    fnrho = m_Directory + "/rho.dat";

	m_SA = new SA_Dispatcher();

	m_initStruct = *InitStruct;
	m_isearchalgorithm = InitStruct->Algorithm;

	InitializeSA(InitStruct, m_SA);
	m_initError = Initialize(InitStruct);
}

StochFit::~StochFit()
{
	// Stop the worker thread if it's still running
	if(m_thread.joinable())
	{
		m_thread.request_stop();
		m_thread.join();
	}

	if(Zinc != NULL)
	{
		delete params;

		if(m_SA != NULL)
			delete m_SA;

		delete[] Zinc;
		delete[] Qinc;
		delete[] Rho;
		delete[] Refl;
	}
}

tl::expected<void, std::string> StochFit::Initialize(ReflSettings* InitStruct)
{
	 //////////////////////////////////////////////////////////
	 /******** Setup Variables and ReflectivityClass ********/
	 ////////////////////////////////////////////////////////

	if(auto r = m_cRefl.Init(InitStruct); !r)
		return r;
	m_cEDP.Init(InitStruct);


	//Setup the params - We start with a slightly roughened ED curve
	params = new ParamVector(InitStruct);

	 /////////////////////////////////////////////////////
     /******** Prepare Arrays for the Front End ********/
	////////////////////////////////////////////////////

	m_irhocount = m_cEDP.Get_EDPPointCount();
	m_irefldatacount = m_cRefl.GetDataCount();

	Zinc = new double[m_irhocount];
	Qinc = new double[m_irefldatacount];
	Rho = new double[m_irhocount];
	Refl = new double[m_irefldatacount];

	//Let the frontend know that we've set up the arrays
	m_bwarmedup = true;

	//If we have a population already, load it
	LoadFromFile();

	// Update the constraints on the params
	params->UpdateBoundaries(NULL,NULL);

	m_SA->InitializeParameters(InitStruct, params, &m_cRefl, &m_cEDP);

	if(m_SA->CheckForFailure() == true)
		platform_error("Catastrophic error in SA - please contact the author");
	return {};
}

int StochFit::Processing(std::stop_token stop_tok)
{
#if STOCHFIT_HAS_GPU
	auto gpu_info = detect_gpu();
	if (m_initStruct.UseGpu && gpu_info.backend != GpuBackend::None) {
		m_gpuBackend = gpu_info.backend;
		return ProcessingGPU(stop_tok);
	}
#endif

	bool accepted = false;

	//Main loop
     for(int isteps=0;(isteps < m_itotaliterations) && !stop_tok.stop_requested();isteps++)
	 {
			accepted = m_SA->Iteration(params);

			if(accepted || isteps == 0)
			{
				m_dChiSquare = m_cRefl.m_dChiSquare;
				m_dGoodnessOfFit = m_cRefl.m_dgoodnessoffit;
			}
		    UpdateFits(isteps);

			//Write the population file every 5000 iterations
			if((isteps+1)%5000 == 0 || isteps == m_itotaliterations-1)
			{
				//Write out the population file for the best minimum found so far
				if(m_isearchalgorithm != 0 && m_SA->Get_IsIterMinimum())
					WritetoFile((m_Directory + "/BestSASolution.txt").c_str());

				WritetoFile(fnpop.c_str());
			}
	 }

	//Update the arrays one last time
	UpdateFits(m_icurrentiteration);

	return 0;
}

#if STOCHFIT_HAS_GPU
void StochFit::InitGpuData(GpuSAState& sa_state, GpuParams& gpu_params,
                            GpuMeasurement& meas, GpuEDPConfig& edp_config)
{
	// SA state from current SimAnneal
	memset(&sa_state, 0, sizeof(sa_state));
	sa_state.temperature = 1.0f / (float)m_SA->Get_Temp();
	sa_state.best_energy = (float)m_SA->Get_LowestEnergy();
	sa_state.current_energy = sa_state.best_energy;
	sa_state.best_solution = sa_state.best_energy;
	sa_state.slope = (float)m_initStruct.Slope;
	sa_state.gamma = (float)m_initStruct.Gamma;
	sa_state.avg_fstun = (float)m_initStruct.Inittemp;
	sa_state.gammadec = (float)m_initStruct.Gammadec;
	sa_state.stepsize = (float)m_initStruct.Paramtemp;
	sa_state.algorithm = m_initStruct.Algorithm;
	sa_state.iteration = 0;
	sa_state.plat_time = m_initStruct.Platiter;
	sa_state.temp_iter = m_initStruct.Tempiter;
	sa_state.stun_func = m_initStruct.STUNfunc;
	sa_state.stun_dec_iter = m_initStruct.STUNdeciter;
	sa_state.adaptive = m_initStruct.Adaptive;
	sa_state.sigmasearch = m_initStruct.Sigmasearch;
	sa_state.normsearch = m_initStruct.NormalizationSearchPerc;
	sa_state.abssearch = m_initStruct.AbsorptionSearchPerc;

	// Params from ParamVector
	memset(&gpu_params, 0, sizeof(gpu_params));
	int rps = params->RealparamsSize();
	gpu_params.real_params_size = rps;
	gpu_params.num_boxes = params->GetInitializationLength();
	for (int i = 0; i < rps; i++)
		gpu_params.sld_values[i] = params->GetRealparams(i);
	gpu_params.roughness = params->getroughness();
	gpu_params.surf_abs = params->getSurfAbs();
	gpu_params.imp_norm = params->getImpNorm();
	gpu_params.fix_roughness = params->Get_FixedRoughness() ? 1 : 0;
	gpu_params.use_surf_abs = params->Get_UseSurfAbs() ? 1 : 0;
	gpu_params.fix_imp_norm = params->Get_FixImpNorm() ? 1 : 0;
	gpu_params.roughness_low = 0.1f;
	gpu_params.roughness_high = 8.0f;
	gpu_params.surfabs_high = 10000.0f;
	gpu_params.impnorm_high = 10000.0f;
	gpu_params.param_low = -5.0f;
	gpu_params.param_high = 5.0f;

	// Measurement data (convert double -> float)
	int nd = m_cRefl.m_idatapoints;
	m_fMeasQ.resize(nd);
	m_fMeasRefl.resize(nd);
	m_fMeasErr.resize(nd);
	m_fMeasSintheta.resize(nd);
	m_fMeasSinsq.resize(nd);
	for (int i = 0; i < nd; i++) {
		m_fMeasQ[i] = (float)m_cRefl.xi[i];
		m_fMeasRefl[i] = (float)m_cRefl.yi[i];
		m_fMeasErr[i] = (float)m_cRefl.eyi[i];
		m_fMeasSintheta[i] = (float)m_cRefl.sinthetai[i];
		m_fMeasSinsq[i] = (float)m_cRefl.sinsquaredthetai[i];
	}

	bool use_qspread = (m_cRefl.m_dQSpread > 0.0f && m_cRefl.exi.has_value());
	if (use_qspread) {
		m_fQspreadSin.resize(nd * 13);
		m_fQspreadSin2.resize(nd * 13);
		for (int i = 0; i < nd * 13; i++) {
			m_fQspreadSin[i] = (float)m_cRefl.qspreadsinthetai[i];
			m_fQspreadSin2[i] = (float)m_cRefl.qspreadsinsquaredthetai[i];
		}
	}

	memset(&meas, 0, sizeof(meas));
	meas.q_values = m_fMeasQ.data();
	meas.refl_values = m_fMeasRefl.data();
	meas.refl_errors = m_fMeasErr.data();
	meas.sintheta = m_fMeasSintheta.data();
	meas.sinsquaredtheta = m_fMeasSinsq.data();
	meas.qspread_sintheta = use_qspread ? m_fQspreadSin.data() : nullptr;
	meas.qspread_sin2theta = use_qspread ? m_fQspreadSin2.data() : nullptr;
	meas.num_datapoints = nd;
	meas.objective_function = m_cRefl.objectivefunction;
	meas.use_qspread = use_qspread ? 1 : 0;
	meas.force_norm = m_initStruct.Forcenorm;
	meas.imp_norm = m_initStruct.Impnorm;
	meas.xr_only = m_initStruct.XRonly;

	// EDP config
	int nl = m_cEDP.Get_EDPPointCount();
	m_fEdSpacing.resize(nl);
	m_fDistArray.resize(gpu_params.num_boxes + 2);

	// Read from CEDP's internal arrays (they are float already)
	// We need to access them - use the public Init data
	for (int i = 0; i < nl; i++)
		m_fEdSpacing[i] = i * (float)m_cEDP.Get_Dz() - (float)m_initStruct.Leftoffset;

	int FilmSlack = 7;
	for (int k = 0; k < gpu_params.num_boxes + 2; k++)
		m_fDistArray[k] = k * ((float)m_initStruct.FilmLength + (float)FilmSlack) / (float)m_initStruct.Boxes;

	float waveConst = m_cEDP.Get_WaveConstant();
	float lambda = (float)m_initStruct.Wavelength;

	memset(&edp_config, 0, sizeof(edp_config));
	edp_config.ed_spacing = m_fEdSpacing.data();
	edp_config.dist_array = m_fDistArray.data();
	edp_config.rho = (float)(m_initStruct.FilmSLD * 1e-6) * waveConst;
	edp_config.dz = (float)m_cEDP.Get_Dz();
	edp_config.k0 = 2.0f * (float)M_PI / lambda;
	edp_config.num_layers = nl;
	edp_config.use_abs = m_initStruct.UseSurfAbs;
	if (m_initStruct.UseSurfAbs) {
		edp_config.beta = (float)m_initStruct.FilmAbs * waveConst;
		edp_config.beta_sub = (float)m_initStruct.SubAbs * waveConst;
		edp_config.beta_sup = (float)m_initStruct.SupAbs * waveConst;
	}
}

int StochFit::ProcessingGPU(std::stop_token stop_tok)
{
	GpuSAState sa_state;
	GpuParams gpu_params;
	GpuMeasurement meas;
	GpuEDPConfig edp_config;
	InitGpuData(sa_state, gpu_params, meas, edp_config);

	auto gpu_info = detect_gpu();
	int num_chains = std::min(gpu_info.max_chains, 64);

	m_gpuRunner = GpuSARunner::create(m_gpuBackend);
	if (!m_gpuRunner) {
		// GPU runner creation failed — should not happen if detect_gpu() succeeded
		return -1;
	}

	m_gpuRunner->initialize(sa_state, gpu_params, meas, edp_config, num_chains);

	int batch_size = 5000;
	auto last_update = std::chrono::steady_clock::now();
	constexpr auto update_interval = std::chrono::seconds(2);

	for (int done = 0; done < m_itotaliterations && !stop_tok.stop_requested(); done += batch_size) {
		int this_batch = std::min(batch_size, m_itotaliterations - done);
		m_gpuRunner->run_batch(this_batch);
		m_icurrentiteration = done + this_batch;

		auto now = std::chrono::steady_clock::now();
		bool time_for_update = (now - last_update) >= update_interval;

		if (m_bupdated || time_for_update) {
			GpuResultSummary result = m_gpuRunner->get_result();

			m_dRoughness = (double)result.best_roughness;
			m_dChiSquare = (double)result.best_chi_square;
			m_dGoodnessOfFit = (double)result.best_gof;

			// Update EDP arrays for frontend
			std::vector<float> edp_buf(m_irhocount);
			m_gpuRunner->get_best_edp(edp_buf.data(), m_irhocount);
			float edp_last = edp_buf[m_irhocount - 1];
			for (int i = 0; i < m_irhocount; i++) {
				Zinc[i] = i * m_cEDP.Get_Dz();
				Rho[i] = (edp_last != 0.0f) ? (double)(edp_buf[i] / edp_last) : 0.0;
			}

			// Update reflectivity arrays for frontend
			int refl_count = m_cRefl.m_idatapoints;
			std::vector<float> refl_buf(refl_count);
			m_gpuRunner->get_best_reflectivity(refl_buf.data(), refl_count);

			// For GetData, we need Q values and reflectivity
			// Use the measurement Q values directly
			int out_count = std::min(m_irefldatacount, refl_count);
			for (int i = 0; i < out_count; i++) {
				Qinc[i] = (double)m_fMeasQ[i];
				Refl[i] = (double)refl_buf[i];
			}

			m_bupdated = false;
			last_update = now;

			// Copy best params back to ParamVector for file writing
			int rps = params->RealparamsSize();
			for (int i = 0; i < rps && i < (int)(GPU_MAX_BOXES + 2); i++) {
				if (i == 0)
					params->SetSupphase(result.best_params[i]);
				else if (i == rps - 1)
					params->SetSubphase(result.best_params[i]);
				else
					params->SetMutatableParameter(i - 1, result.best_params[i]);
			}
			params->setroughness(result.best_roughness);
			if (params->Get_UseSurfAbs())
				params->setSurfAbs(result.best_surf_abs);
			if (params->Get_FixImpNorm())
				params->setImpNorm(result.best_imp_norm);

			m_SA->Set_Temp(1.0 / (double)result.best_temperature);
			m_SA->Set_AveragefSTUN((double)result.best_avg_fstun);

			WritetoFile(fnpop.c_str());
		}
	}

	// Final update
	GpuResultSummary result = m_gpuRunner->get_result();
	m_dRoughness = (double)result.best_roughness;
	m_dChiSquare = (double)result.best_chi_square;
	m_dGoodnessOfFit = (double)result.best_gof;

	// Copy final params back
	int rps = params->RealparamsSize();
	for (int i = 0; i < rps && i < (int)(GPU_MAX_BOXES + 2); i++) {
		if (i == 0)
			params->SetSupphase(result.best_params[i]);
		else if (i == rps - 1)
			params->SetSubphase(result.best_params[i]);
		else
			params->SetMutatableParameter(i - 1, result.best_params[i]);
	}
	params->setroughness(result.best_roughness);

	WritetoFile(fnpop.c_str());

	m_gpuRunner.reset();
	return 0;
}
#endif

void StochFit::UpdateFits(int currentiteration)
{
		if(m_bupdated == true)
		{
			//Check to see if we're updating
			m_cEDP.GenerateEDP(params);
			m_cEDP.WriteOutputFile(fnrho);
			m_cRefl.ParamsRF(&m_cEDP, fnrf);
			m_dRoughness = params->getroughness();


			for(int i = 0; i<m_irhocount;i++)
			{
				Zinc[i] = i*m_cEDP.Get_Dz();
				Rho[i] =  m_cEDP.m_EDP[i].real()/m_cEDP.m_EDP[m_cEDP.Get_EDPPointCount()-1].real();
			}

			for(int i = 0; i < m_irefldatacount;i++)
			{
				#ifndef CHECKREFLCALC

					if(m_cRefl.m_dQSpread > 0.0)
					{
						Refl[i] = m_cRefl.reflpt[i];
						Qinc[i] = m_cRefl.xi[i];
					}
					else
					{
						Refl[i] = m_cRefl.dataout[i];
						Qinc[i] = m_cRefl.qarray[i];
					}
				#else
					Qinc[i] = m_cRefl.xi[i];
					Refl[i] = m_cRefl.reflpt[i];
				#endif
			}
			m_bupdated = false;
		}
		m_icurrentiteration = currentiteration;
}

int StochFit::Start(int iterations)
{
	m_itotaliterations = iterations;
	m_thread = std::jthread([this](std::stop_token stop_tok){ Processing(stop_tok); });
	return 0;
}

int StochFit::Cancel()
{
	if(m_thread.joinable())
	{
		m_thread.request_stop();
		// Don't join here - let the thread finish on its own
		// Joining blocks the main thread and can cause UI freezes
	}
	return 0;
}

void StochFit::InitializeSA(ReflSettings* InitStruct, SA_Dispatcher* SA)
{
	SA->Initialize(InitStruct->Debug, m_Directory);
	SA->Initialize_Subsytem(InitStruct);
}

int StochFit::GetData(double* Z, double* RhoOut, double* Q, double* ReflOut, double* roughness, double* chisquare, double* goodnessoffit, BOOL* isfinished)
{
	//Sleep while we are generating our output data
	if(m_icurrentiteration != m_itotaliterations-1)
	{
		m_bupdated = true;

		while(m_bupdated == true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
	else
	{
		m_bupdated = false;
	}
	//We only have one thread, and we're controlling access to it, so no need for fancy synchronization here

	for(int i = 0; i < m_irhocount; i++)
	{
		Z[i] = Zinc[i];
		RhoOut[i] = Rho[i];
	}

	for(int i = 0; i < m_irefldatacount; i++)
	{
		Q[i] = Qinc[i];
		ReflOut[i] = Refl[i];
	}

	*roughness = m_dRoughness;
	*chisquare = m_dChiSquare;
	*goodnessoffit = m_dGoodnessOfFit;

	// Check if thread is still running
	*isfinished = !m_thread.joinable();

	return m_icurrentiteration;
}

int StochFit::Priority(int priority)
{
	// Thread priority is not portable; store for potential future use.
	m_ipriority = priority;
	return 0;
}

void StochFit::WritetoFile(const char* filename)
{
	ofstream outfile;
	outfile.open(filename);
	outfile<< params->getroughness() <<' '<< m_cEDP.Get_FilmAbs()*params->getSurfAbs()/m_cEDP.Get_WaveConstant() <<
		' '<< m_SA->Get_Temp() <<' '<< params->getImpNorm() << ' ' << m_SA->Get_AveragefSTUN() << endl;

	for(int i = 0; i < params->RealparamsSize(); i++)
	{
		outfile<<params->GetRealparams(i)<< endl;
	}

	outfile.close();
}

void StochFit::LoadFromFile(string file)
{
   ParamVector params1 = *params;
   ifstream infile;

   if(file.empty())
	 infile.open(fnpop.c_str());
   else
	 infile.open(file.c_str());

   int size = params->RealparamsSize();
   int counter = 0;
   bool kk = true;
   double beta = 0;
   double  avgfSTUN = 0;
   double currenttemp = 0;
   double normfactor = 0;

   int i = 0;


   if(infile.is_open())
   {
       double ED, roughness;
       infile >> roughness >> beta >> currenttemp >> normfactor >> avgfSTUN;

       params1.setroughness(roughness);

	   while(!infile.eof() && i < params1.RealparamsSize())
	   {
            if(infile>>ED)
			{
				if(i == 0)
					params1.SetSupphase(ED);
				else if (i == params1.RealparamsSize()-1)
					params1.SetSubphase(ED);
				else
				    params1.SetMutatableParameter(i-1,ED);

				counter++;
				i++;
			}
			else
			{
                kk = false;
                break;
            }
        }
	   infile>>ED;

    }
    else
	{
		kk = false;
	}

	if(kk == true && infile.eof() == false)
	{
		kk = false;
	}

    if(kk == true)
	{
		*params = params1;
		m_cEDP.Set_FilmAbs(beta);
		m_SA->Set_Temp(1.0/currenttemp);
		params->setImpNorm(normfactor);
		m_SA->Set_AveragefSTUN(avgfSTUN);
    }
	infile.close();

	//If we're resuming, and we were Tunneling, load the best file
	if(kk == true && fnpop.find("BestSASolution.txt") == string::npos)
	{
		LoadFromFile(m_Directory + "BestSASolution.txt");
	}
}

void StochFit::GetArraySizes(int* RhoSize, int* ReflSize)
{
	*RhoSize = m_irhocount;
	*ReflSize = m_irefldatacount;
}

bool StochFit::GetWarmedUp()
{
	return m_bwarmedup;
}
