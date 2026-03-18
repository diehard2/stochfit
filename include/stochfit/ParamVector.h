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

// SA parameter bounds and mutation management.
// Maps a box-model electron density profile to a mutable search space of
// double-precision parameters: per-box SLD values, roughness, surface
// absorption, and imperfect-normalization factor. Enforces clamped bounds
// on all mutations. All public API uses double.

#include "platform.h"
#include "SettingsStruct.h"

class ParamVector 
{
	private:
		vector <double> gnome;
		vector <double> data_params;
		vector <double> data_params_high_val;
		vector <double> data_params_low_val;
		int length;
		bool m_binitialized;
		bool m_bfixroughness;
		bool m_busesurfabs;
		bool m_bfiximpnorm;
		int m_dparameter_size;
		int m_iroughness_index;
		int m_isurfabs_index;
		int m_iimpnorm_index;
		double roughness;

		void SetBounds(double lowrough, double highrough, double highimp, double highabs);

	public:
		ParamVector(ReflSettings* InitStruct);
		ParamVector();
		int RealparamsSize();
		int GetInitializationLength();
		int ParamCount();
		double GetRealparams(int i);
		double GetMutatableParameter(int i);
		int SetMutatableParameter(int i, double x);
		double getroughness();
		int setroughness(double x);
		double getImpNorm();
		int setImpNorm(double rough);
		double getSurfAbs();
		int setSurfAbs(double norm);
		void SetSubphase(double subval);
		void SetSupphase(double supval);
		// ******** MAYBEDEAD ******** CopyArraytoGene — no callers found outside this file
		bool CopyArraytoGene(double* myarray);
		void UpdateBoundaries(double* high, double* low);
		bool Get_FixedRoughness(){ return m_bfixroughness;}
		bool Get_FixImpNorm(){ return m_bfiximpnorm;}
		bool Get_UseSurfAbs(){ return m_busesurfabs;}


		double GetUpperBounds(int index);
		double GetLowerBounds(int index);
};