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


#include "platform.h"
#include "ParamVector.h"
#include "ReflCalc.h"
#include "SimulatedAnnealing.h"
#include "StochFitHarness.h"

#include "gpu/gpu_detect.h"
#include "gpu/gpu_sa_runner.h"

StochFit::StochFit(ReflSettings* InitStruct, StochRunState* state)
{
	m_bupdated = false;
	m_stop_requested = false;
	m_bwarmedup = false;
	m_ipriority = 2;
	Zinc = nullptr;
	Qinc = nullptr;
	Rho  = nullptr;
	Refl = nullptr;
	params = nullptr;
	m_icurrentiteration = 0;
	m_itotaliterations = 0;
	m_irhocount = 0;
	m_irefldatacount = 0;

	m_Directory = string(InitStruct->Directory);

	m_SA = new SA_Dispatcher();

	m_initStruct = *InitStruct;
	m_isearchalgorithm = InitStruct->Algorithm;

	InitializeSA(InitStruct, m_SA);

	// ── Initialize (inlined) ───────────────────────────────────────────────
	if (auto r = m_cRefl.Init(InitStruct); !r) {
		m_initError = r;
		return;
	}
	m_cEDP.Init(InitStruct);

	params = new ParamVector(InitStruct);

	m_irhocount = m_cEDP.Get_EDPPointCount();
	m_irefldatacount = m_cRefl.GetDataCount();

	Zinc = new double[m_irhocount];
	Qinc = new double[m_irefldatacount];
	Rho  = new double[m_irhocount];
	Refl = new double[m_irefldatacount];

	m_bwarmedup = true;

	// ── Apply run state if provided ────────────────────────────────────────
	// filmAbsInput is the pre-multiplication value: Set_FilmAbs(x) stores x*WC.
	// temperature is raw m_dTemp stored directly via Set_Temp (not 1/m_dTemp).
	// surfAbs is saved independently so it is never baked into filmAbsInput.
	if (state != nullptr && state->edCount == params->RealparamsSize()) {
		params->setroughness(state->roughness);
		params->SetSupphase(state->edValues[0]);
		for (int i = 1; i < state->edCount - 1; i++)
			params->SetMutatableParameter(i - 1, state->edValues[i]);
		params->SetSubphase(state->edValues[state->edCount - 1]);
		m_cEDP.Set_FilmAbs(state->filmAbsInput);  // correct: stores filmAbsInput * WC as m_dBeta
		params->setSurfAbs(state->surfAbs);         // independent, not baked into filmAbsInput
		m_SA->Set_Temp(state->temperature);          // direct: m_dTemp = temperature
		params->setImpNorm(state->impNorm);
		m_SA->Set_AveragefSTUN(state->avgfSTUN);
	}

	params->UpdateBoundaries(NULL, NULL);
	m_SA->InitializeParameters(InitStruct, params, &m_cRefl, &m_cEDP);

	if (m_SA->CheckForFailure() == true)
		platform_error("Catastrophic error in SA - please contact the author");

	m_initError = {};
}

StochFit::~StochFit()
{
	// Stop the worker thread if it's still running
	if(m_thread.joinable())
	{
		m_stop_requested = true;
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


int StochFit::Processing()
{
	try
	{
	if (m_initStruct.UseGpu) {
		auto gpu_info = detect_gpu();
		if (gpu_info.backend != GpuBackend::None) {
			m_gpuBackend = gpu_info.backend;
			return ProcessingGPU();
		}
	}

	bool accepted = false;

	//Main loop
     for(int isteps=0;(isteps < m_itotaliterations) && !m_stop_requested.load();isteps++)
	 {
			accepted = m_SA->Iteration(params);

			if(accepted || isteps == 0)
			{
				m_dChiSquare = m_cRefl.m_dChiSquare;
				m_dGoodnessOfFit = m_cRefl.m_dgoodnessoffit;
			}
		    UpdateFits(isteps);
	 }

	//Update the arrays one last time
	UpdateFits(m_icurrentiteration);

	return 0;
	}
	catch(const std::exception& ex)
	{
		std::cerr << "[StochFit] Processing() caught exception: " << ex.what() << std::endl;
		// Set iteration to total so GetData() knows we're done
		m_icurrentiteration = m_itotaliterations > 0 ? m_itotaliterations - 1 : 0;
		m_bupdated = false;
		return -1;
	}
	catch(...)
	{
		std::cerr << "[StochFit] Processing() caught unknown exception" << std::endl;
		m_icurrentiteration = m_itotaliterations > 0 ? m_itotaliterations - 1 : 0;
		m_bupdated = false;
		return -1;
	}
}

void StochFit::InitGpuData(GpuSAState& sa_state, GpuParams& gpu_params,
                            GpuMeasurement& meas, GpuEDPConfig& edp_config)
{
	// SA state from current SimAnneal
	memset(&sa_state, 0, sizeof(sa_state));
	sa_state.temperature = 1.0f / static_cast<float>(m_SA->Get_Temp());
	sa_state.best_energy = static_cast<float>(m_SA->Get_LowestEnergy());
	sa_state.current_energy = sa_state.best_energy;
	sa_state.best_solution = sa_state.best_energy;
	sa_state.slope = static_cast<float>(m_initStruct.Slope);
	sa_state.gamma = static_cast<float>(m_initStruct.Gamma);
	sa_state.avg_fstun = static_cast<float>(m_initStruct.Inittemp);
	sa_state.gammadec = static_cast<float>(m_initStruct.Gammadec);
	sa_state.stepsize = static_cast<float>(m_initStruct.Paramtemp);
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
		gpu_params.sld_values[i] = static_cast<float>(params->GetRealparams(i));
	gpu_params.roughness = static_cast<float>(params->getroughness());
	gpu_params.surf_abs = static_cast<float>(params->getSurfAbs());
	gpu_params.imp_norm = static_cast<float>(params->getImpNorm());
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
		m_fMeasQ[i] = static_cast<float>(m_cRefl.xi[i]);
		m_fMeasRefl[i] = static_cast<float>(m_cRefl.yi[i]);
		m_fMeasErr[i] = static_cast<float>(m_cRefl.eyi[i]);
		m_fMeasSintheta[i] = static_cast<float>(m_cRefl.sinthetai[i]);
		m_fMeasSinsq[i] = static_cast<float>(m_cRefl.sinsquaredthetai[i]);
	}

	bool use_qspread = (m_cRefl.m_dQSpread > 0.0f && m_cRefl.exi.has_value());
	if (use_qspread) {
		m_fQspreadSin.resize(nd * 13);
		m_fQspreadSin2.resize(nd * 13);
		for (int i = 0; i < nd * 13; i++) {
			m_fQspreadSin[i] = static_cast<float>(m_cRefl.qspreadsinthetai[i]);
			m_fQspreadSin2[i] = static_cast<float>(m_cRefl.qspreadsinsquaredthetai[i]);
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
		m_fEdSpacing[i] = i * static_cast<float>(m_cEDP.Get_Dz()) - 40.0f;

	int FilmSlack = 7;
	for (int k = 0; k < gpu_params.num_boxes + 2; k++)
		m_fDistArray[k] = k * (static_cast<float>(m_initStruct.FilmLength) + static_cast<float>(FilmSlack)) / static_cast<float>(m_initStruct.Boxes);

	float waveConst = static_cast<float>(m_cEDP.Get_WaveConstant());
	float lambda = static_cast<float>(m_initStruct.Wavelength);

	memset(&edp_config, 0, sizeof(edp_config));
	edp_config.ed_spacing = m_fEdSpacing.data();
	edp_config.dist_array = m_fDistArray.data();
	edp_config.rho = static_cast<float>(m_initStruct.FilmSLD * 1e-6) * waveConst;
	edp_config.dz = static_cast<float>(m_cEDP.Get_Dz());
	edp_config.k0 = 2.0f * std::numbers::pi_v<float> / lambda;
	edp_config.num_layers = nl;
	edp_config.use_abs = m_initStruct.UseSurfAbs;
	if (m_initStruct.UseSurfAbs) {
		edp_config.beta = static_cast<float>(m_initStruct.FilmAbs) * waveConst;
		edp_config.beta_sub = static_cast<float>(m_initStruct.SubAbs) * waveConst;
		edp_config.beta_sup = static_cast<float>(m_initStruct.SupAbs) * waveConst;
	}
}

int StochFit::ProcessingGPU()
{
	GpuSAState sa_state;
	GpuParams gpu_params;
	GpuMeasurement meas;
	GpuEDPConfig edp_config;
	InitGpuData(sa_state, gpu_params, meas, edp_config);

	auto gpu_info = detect_gpu();
	int num_chains = (m_initStruct.GpuChains > 0) ? m_initStruct.GpuChains : gpu_info.max_chains;

	m_gpuRunner = GpuSARunner::create(m_gpuBackend);
	if (!m_gpuRunner) {
		// GPU runner creation failed — should not happen if detect_gpu() succeeded
		return -1;
	}

	m_gpuRunner->initialize(sa_state, gpu_params, meas, edp_config, num_chains);

	int batch_size = 5000;
	auto last_update = std::chrono::steady_clock::now();
	constexpr auto update_interval = std::chrono::seconds(2);

	// Scale total iterations so the displayed it/sec reflects total Parratt
	// recursions (outer_iters × chains), making it comparable to the CPU counter.
	int outer_total = m_itotaliterations;
	m_itotaliterations = outer_total * num_chains;

	for (int done = 0; done < outer_total && !m_stop_requested.load(); done += batch_size) {
		int this_batch = std::min(batch_size, outer_total - done);
		m_gpuRunner->run_batch(this_batch);
		m_icurrentiteration = (done + this_batch) * num_chains;

		auto now = std::chrono::steady_clock::now();
		bool time_for_update = (now - last_update) >= update_interval;

		if (m_bupdated || time_for_update) {
			GpuResultSummary result = m_gpuRunner->get_result();

			m_dRoughness = static_cast<double>(result.best_roughness);
			m_dGoodnessOfFit = static_cast<double>(result.best_gof);

			// Copy best params back to ParamVector
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

			// Regenerate EDP and reflectivity on CPU for display — same as
			// UpdateFits(). This gives the correct z-offset (-40 Å) and
			// fills the full interpolated Q-array (tarraysize ≈ 3000 pts).
			m_cEDP.GenerateEDP(params);
			m_cRefl.ComputeRF(&m_cEDP);
			m_dChiSquare = m_cRefl.m_dChiSquare;

			for (int i = 0; i < m_irhocount; i++) {
				Zinc[i] = i * m_cEDP.Get_Dz() - 40.0;
				Rho[i] = m_cEDP.m_EDP[i].real() / m_cEDP.m_EDP[m_cEDP.Get_EDPPointCount()-1].real();
			}

			for (int i = 0; i < m_irefldatacount; i++) {
				if (m_cRefl.m_dQSpread > 0.0) {
					Refl[i] = m_cRefl.reflpt[i];
					Qinc[i] = m_cRefl.xi[i];
				} else {
					Refl[i] = m_cRefl.dataout[i];
					Qinc[i] = m_cRefl.qarray[i];
				}
			}

			m_bupdated = false;
			last_update = now;
		}
	}

	// Final update — params already synced in last in-loop update;
	// regenerate once more to ensure output files and display arrays are current.
	GpuResultSummary result = m_gpuRunner->get_result();
	m_dRoughness = (double)result.best_roughness;
	m_dGoodnessOfFit = (double)result.best_gof;

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

	m_cEDP.GenerateEDP(params);
	m_cRefl.ComputeRF(&m_cEDP);
	m_dChiSquare = m_cRefl.m_dChiSquare;

	for (int i = 0; i < m_irhocount; i++) {
		Zinc[i] = i * m_cEDP.Get_Dz() - 40.0;
		Rho[i] = m_cEDP.m_EDP[i].real() / m_cEDP.m_EDP[m_cEDP.Get_EDPPointCount()-1].real();
	}

	for (int i = 0; i < m_irefldatacount; i++) {
		if (m_cRefl.m_dQSpread > 0.0) {
			Refl[i] = m_cRefl.reflpt[i];
			Qinc[i] = m_cRefl.xi[i];
		} else {
			Refl[i] = m_cRefl.dataout[i];
			Qinc[i] = m_cRefl.qarray[i];
		}
	}

	m_gpuRunner.reset();
	return 0;
}

void StochFit::UpdateFits(int currentiteration)
{
		if(m_bupdated == true)
		{
			//Check to see if we're updating
			m_cEDP.GenerateEDP(params);
			m_cRefl.ComputeRF(&m_cEDP);
			m_dRoughness = params->getroughness();


			for(int i = 0; i<m_irhocount;i++)
			{
				Zinc[i] = i*m_cEDP.Get_Dz() - 40.0;
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
	m_stop_requested = false;
	m_thread = std::thread([this]{ Processing(); });
	return 0;
}

int StochFit::Cancel()
{
	if(m_thread.joinable())
	{
		m_stop_requested = true;
		// Don't join here - let the thread finish on its own
		// Joining blocks the main thread and can cause UI freezes
	}
	return 0;
}

void StochFit::Stop()
{
	if(m_thread.joinable())
	{
		m_stop_requested = true;
		m_thread.join(); // blocks until current iteration completes
	}
}

void StochFit::GetRunState(double* saScalars, double* edValues, int* edCount)
{
	// Only safe to call after Stop() — worker must not be running.
	// saScalars[0..8]: roughness, filmAbsInput, surfAbs, temperature,
	//                  impNorm, avgfSTUN, bestSolution, chiSquare, goodnessOfFit
	saScalars[0] = params->getroughness();
	saScalars[1] = m_cEDP.Get_FilmAbsInput();
	saScalars[2] = params->getSurfAbs();
	saScalars[3] = m_SA->Get_RawTemp();
	saScalars[4] = params->getImpNorm();
	saScalars[5] = m_SA->Get_AveragefSTUN();
	saScalars[6] = m_SA->Get_LowestEnergy();
	saScalars[7] = m_dChiSquare;
	saScalars[8] = m_dGoodnessOfFit;

	int count = params->RealparamsSize();
	for (int i = 0; i < count; i++)
		edValues[i] = params->GetRealparams(i);
	*edCount = count;
}

void StochFit::InitializeSA(ReflSettings* InitStruct, SA_Dispatcher* SA)
{
	SA->Initialize(InitStruct->Debug, m_Directory);
	SA->Initialize_Subsytem(InitStruct);
}

int StochFit::GetData(double* Z, double* RhoOut, double* Q, double* ReflOut, double* roughness, double* chisquare, double* goodnessoffit, int32_t* isfinished)
{
	// Request a data update from the worker thread, but don't block the caller
	// indefinitely. Signal the worker, then wait up to 2 seconds for it to
	// respond. If the worker has stalled or finished, we return the most recent
	// available data instead of spinning forever.
	bool done = (m_icurrentiteration >= m_itotaliterations - 1);
	if (!done)
	{
		m_bupdated = true;
		auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
		while(m_bupdated.load() && std::chrono::steady_clock::now() < deadline)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		m_bupdated = false; // reset in case of timeout
	}

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

	// Finished when all iterations are done (thread may still be "joinable"
	// until the jthread destructor runs, so check iteration count instead)
	*isfinished = (m_icurrentiteration >= m_itotaliterations - 1) ? 1 : 0;

	return m_icurrentiteration;
}

int StochFit::Priority(int priority)
{
	// Thread priority is not portable; store for potential future use.
	m_ipriority = priority;
	return 0;
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
