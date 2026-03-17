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

#pragma once

// Top-level SA orchestrator.
// Manages the worker jthread, GPU dispatch (CUDA/Metal when UseGpu=true),
// and data marshaling between C++ and the FFI layer.
// Session file I/O is now handled by Electron: Init() accepts an optional
// StochRunState* with pre-parsed state (null = fresh start).
// On stop: call Stop() to block until worker exits, then GetRunState() to read
// the raw internal values, then Destroy() to clean up the object.
// When UseGpu=true and detect_gpu() finds a suitable device, ProcessingGPU()
// runs multi-chain SA via the CUDA plugin (loaded at runtime); otherwise
// Processing() runs single-chain CPU SA.
// Float buffers (m_fMeas*, m_fQspread*, m_fEdSpacing, m_fDistArray) hold
// double→float downsampled data for GPU transfer.

#include <memory>
#include <thread>
#include <atomic>
#include "ParamVector.h"
#include "ReflCalc.h"
#include "CEDP.h"
#include "SA_Dispatcher.h"
#include <stochfit/SettingsStruct.h>

class GpuSARunner;
struct GpuSAState;
struct GpuParams;
struct GpuMeasurement;
struct GpuEDPConfig;

class StochFit
{
	public:
		StochFit(ReflSettings* InitStruct, StochRunState* state = nullptr);
		~StochFit();
		int Start(int iterations);
		int Cancel();
		void Stop(); // request_stop + join — blocks until worker exits
		// ******** MAYBEDEAD ******** Priority — stores value but never acts on it
		int Priority(int priority);
		int GetData(double* Z, double* RhoOut, double* Q, double* ReflOut, double* roughness, double* chisquare, double* goodnessoffit, int32_t* isfinished);
		// GetRunState: fills flat output buffers with current SA state.
		// Only call after Stop() — not safe to call while worker is running.
		// saScalars[9]: roughness, filmAbsInput, surfAbs, temperature, impNorm, avgfSTUN, bestSolution, chiSquare, goodnessOfFit
		// edValues[Boxes+2]: raw GetRealparams() values
		// edCount[1]: actual count written
		void GetRunState(double* saScalars, double* edValues, int* edCount);
		void InitializeSA(ReflSettings* InitStruct, SA_Dispatcher* SA);
		void GetArraySizes(int* RhoSize, int* ReflSize);
		bool GetWarmedUp();
		tl::expected<void, std::string> GetInitError() const { return m_initError; }

		SA_Dispatcher* m_SA;
		bool m_bwarmedup;

	private:
		int Processing();
		void UpdateFits(int currentiteration);
		tl::expected<void, std::string> m_initError;

		int ProcessingGPU();
		void InitGpuData(GpuSAState& sa_state, GpuParams& gpu_params,
		                 GpuMeasurement& meas, GpuEDPConfig& edp_config);
		std::unique_ptr<GpuSARunner> m_gpuRunner;
		GpuBackend m_gpuBackend = GpuBackend::None;

		// Float buffers for GPU data transfer
		std::vector<float> m_fMeasSintheta;
		std::vector<float> m_fMeasSinsq;
		std::vector<float> m_fMeasQ;
		std::vector<float> m_fMeasRefl;
		std::vector<float> m_fMeasErr;
		std::vector<float> m_fQspreadSin;
		std::vector<float> m_fQspreadSin2;
		std::vector<float> m_fEdSpacing;
		std::vector<float> m_fDistArray;

		double* Zinc;
		double* Qinc;
		double* Rho;
		double* Refl;

		std::thread m_thread;
		std::atomic<bool> m_bupdated;
		std::atomic<bool> m_stop_requested;

		//Set the output file names
		string m_Directory;
		string fnrf;
		string fnrho;
		double m_dRoughness;
		double m_dChiSquare;
		double m_dGoodnessOfFit;
		int m_itotaliterations;
		int m_icurrentiteration;
		int m_iparratlayers;
		int m_ipriority;
		int m_irhocount;
		int m_irefldatacount;
		int m_isearchalgorithm;
		CReflCalc m_cRefl;
		CEDP m_cEDP;
		ParamVector* params;
		ReflSettings m_initStruct;
};
