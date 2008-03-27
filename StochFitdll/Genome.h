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
#include "stdafx.h"

class GARealGenome 
{
	private:
		vector <float> gnome;
		vector <float> data_params;
		vector <float> data_params_high_val;
		vector <float> data_params_low_val;
		int length;
		bool m_binitialized;
		bool m_bfixroughness;
		bool m_busesurfabs;
		bool m_bfiximpnorm;
		int m_dparameter_size;
		int m_iroughness_index;
		int m_isurfabs_index;
		int m_iimpnorm_index;
		float roughness;

		void SetBounds(float lowrough, float highrough, float highimp, float highabs);

	public:
		GARealGenome(int l, float force_sig, bool use_surf_abs, bool fix_impnorm);
		GARealGenome();
		int RealGenomeSize();
		int GetInitializationLength();
		int ParamCount();
		float GetRealGenome(int i);
		float GetMutatableParameter(int i);
		int SetMutatableParameter(int i,float x);
		float getroughness();
		int setroughness(float x);
		float getImpNorm();
		int setImpNorm(float rough);
		float getSurfAbs();
		int setSurfAbs(float norm);
		void SetSubphase(float subval);
		void SetSupphase(float supval);
		bool CopyArraytoGene(double* myarray);
		void UpdateBoundaries(double* high, double* low);
		bool Get_FixedRoughness(){ return m_bfixroughness;}
		bool Get_FixImpNorm(){ return m_bfiximpnorm;}
		bool Get_UseSurfAbs(){ return m_busesurfabs;}

	
		float GetUpperBounds(int index);
		float GetLowerBounds(int index);

		bool m_bXROnly;
};