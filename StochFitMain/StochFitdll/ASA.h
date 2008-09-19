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
#include "../ASA/asa_base.h"
#include "ParamVector.h"
#include "ReflCalc.h"
#include "CEDP.h"

class ASA :	public ASA_Base
{
private:

	ParamVector m_cparams;
	CReflCalc* m_cmulti;

	int m_ifuncevals;
	double cost_function (double *x, double *parameter_lower_bound, double *parameter_upper_bound, double *cost_tangents,
               double *cost_curvature, LONG_INT  parameter_dimension,  int *parameter_int_real,
               int *cost_flag, int *exit_code, USER_DEFINES * USER_OPTIONS);

	int initialize_parameters (double *cost_parameters, double *parameter_lower_bound,  double *parameter_upper_bound,
                   double *cost_tangents, double *cost_curvature,  LONG_INT  parameter_dimension,
                   int *parameter_int_real, USER_DEFINES * USER_OPTIONS);
	int asa_iteration();
	int m_i_acc;
	bool m_bfailed;
	CEDP* m_cEDP;
public:
	ASA(bool debug, wstring filename, int paramcount);
	~ASA(void);

	bool Iteration(ParamVector* params);
	
	void Initialize(ParamVector* params, CReflCalc* multi, CEDP* EDP);
	bool CheckFailure(){return m_bfailed;}

	

};
double GenerateDistrib(LONG_INT* seed, LONG_INT parameter_dimension, long int index_v, double temperature_v,
		double init_param_temp_v, double temp_scale_params_v, double parameter_v, double paramter_range_v, 
		double* last_saved_param, void* Options_tmp);

