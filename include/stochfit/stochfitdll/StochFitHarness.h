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
// data marshaling between C++ and the FFI layer, and population file I/O.
// On GPU builds (STOCHFIT_HAS_GPU), ProcessingGPU() runs multi-chain SA
// on the GPU; otherwise Processing() runs single-chain CPU SA.
// Float buffers (m_fMeas*, m_fQspread*, m_fEdSpacing, m_fDistArray) hold
// double→float downsampled data for GPU transfer.

#include <memory>
#include <thread>
#include <stop_token>
#include "ParamVector.h"
#include "ReflCalc.h"
#include "CEDP.h"
#include "SA_Dispatcher.h"

#if STOCHFIT_HAS_GPU
class GpuSARunner;
struct GpuSAState;
struct GpuParams;
struct GpuMeasurement;
struct GpuEDPConfig;
#endif

class StochFit
{
	public:
		StochFit(ReflSettings* InitStruct);
		~StochFit();
		int Start(int iterations);
		int Cancel();
		// ******** MAYBEDEAD ******** Priority — stores value but never acts on it
		int Priority(int priority);
		int GetData(double* Z, double* RhoOut, double* Q, double* ReflOut, double* roughness, double* chisquare, double* goodnessoffit, BOOL* isfinished);
		void InitializeSA(ReflSettings* InitStruct, SA_Dispatcher* SA);
		void GetArraySizes(int* RhoSize, int* ReflSize);
		bool GetWarmedUp();
		tl::expected<void, std::string> GetInitError() const { return m_initError; }

		SA_Dispatcher* m_SA;
		bool m_bwarmedup;

	private:
		int Processing(std::stop_token stop_tok);
		void LoadFromFile(string File = string());
		void WritetoFile(const char* filename);
		void UpdateFits(int currentiteration);
	    tl::expected<void, std::string> Initialize(ReflSettings* InitStruct);
		tl::expected<void, std::string> m_initError;

#if STOCHFIT_HAS_GPU
		int ProcessingGPU(std::stop_token stop_tok);
		void InitGpuData(GpuSAState& sa_state, GpuParams& gpu_params,
		                 GpuMeasurement& meas, GpuEDPConfig& edp_config);
		std::unique_ptr<GpuSARunner> m_gpuRunner;
		GpuBackend m_gpuBackend = GpuBackend::None;

		// Float buffers for GPU->host data transfer
		std::vector<float> m_fMeasSintheta;
		std::vector<float> m_fMeasSinsq;
		std::vector<float> m_fMeasQ;
		std::vector<float> m_fMeasRefl;
		std::vector<float> m_fMeasErr;
		std::vector<float> m_fQspreadSin;
		std::vector<float> m_fQspreadSin2;
		std::vector<float> m_fEdSpacing;
		std::vector<float> m_fDistArray;
#endif

		double* Zinc;
		double* Qinc;
		double* Rho;
		double* Refl;

		std::jthread m_thread;
		std::atomic<bool> m_bupdated;

		//Set the output file names
		string m_Directory;
		string fnpop;
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
