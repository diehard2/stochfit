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
#include <vector>


class ParamVector 
{
	private:
		vector <double> gnome;
		vector <double> data_params;

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

	public:
		ParamVector();
		void Initialize(const ReflSettings* InitStruct);
		int RealparamsSize() const;
		int GetInitializationLength();
		int ParamCount();
		double GetRealparams(int i) const;
		double GetMutatableParameter(int i);
		int SetMutatableParameter(int i,double x);
		double GetRoughness() const;
		int SetRoughness(double x);
		double getImpNorm();
		int setImpNorm(double rough);
		double GetSurfAbs() const;
		int setSurfAbs(double norm);
		void SetSubphase(double subval);
		void SetSupphase(double supval);
	
		bool m_bXROnly;
};