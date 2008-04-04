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

#include "ASA.h"

ASA::ASA(bool debug, string filename, int paramcount):ASA_Base(filename,paramcount), m_ifuncevals(0), m_i_acc(0),
		m_bfailed(false)
{
		m_bASA_print = debug;
		m_bASA_print_intermed = debug;
		m_bincl_stdout = debug;
		m_basa_print_more = debug;
}

ASA::~ASA(void)
{
}

void ASA::Initialize(int paramcount, GARealGenome* genome, CReflCalc* multi)
{
	SetNumParameters(paramcount);
	m_cmulti = multi;
	m_cgenome = *genome;

	asa_alloc();

	int success = asa_init();
	if( success == -1)
	{
		MessageBox(NULL,L"ASA Initialization failed", NULL,NULL);
		m_bfailed = true;
	}
}

bool ASA::Iteration(GARealGenome* genome)
{
	if(m_bfailed == false)
	{
		asa_iteration();
		bool success = genome->CopyArraytoGene(current_generated_state.parameter);
		if(success == false)
			MessageBox(NULL,L"4",NULL,NULL);
	

		if(Options.N_Accepted > m_i_acc)
		{
			if(m_cgenome.CopyArraytoGene(best_generated_state.parameter)== false)
				MessageBox(NULL,L"test5",NULL,NULL);
			m_cgenome.UpdateBoundaries(m_dparameter_maximum, m_dparameter_minimum);
			m_i_acc = Options.N_Accepted;
			return true;
		}
		else
			return false;
	}
	return false;
}

int ASA::asa_iteration()
{
	ASA_Base::asa_iteration();

	//If we accept, alter our bounds
	//if(Options.N_Accepted > m_inumber_accepted)
		

	return 0;
}

double ASA::cost_function (double *x, double *parameter_lower_bound, double *parameter_upper_bound, double *cost_tangents,
               double *cost_curvature, LONG_INT  parameter_dimension,  int *parameter_int_real,
               int *cost_flag, int *exit_code, USER_DEFINES * USER_OPTIONS)
{
	double fitscore = 1e18;
	


	if(m_cgenome.CopyArraytoGene(x) == true)
	{
		fitscore = m_cmulti->objective(&m_cgenome);

		if(fitscore > 0)
		{
			
			
		}
		else
		{
			//Setting the flag to false seems problematic, so lets just make this a 
			//highly unpalatable solution if we're not valid
			fitscore = 1e200;
		}

		
		*cost_flag = TRUE;
	}
	else
	{
		*cost_flag = FALSE;
		MessageBox(NULL, L"3", NULL,NULL);
	}
  return fitscore;
}


int ASA::initialize_parameters(double *cost_parameters, double *parameter_lower_bound, double *parameter_upper_bound,
		double *cost_tangents, double *cost_curvature, long parameter_dimension, int *parameter_int_real, USER_DEFINES *USER_OPTIONS)
{

		//Settings
		Options.Limit_Acceptances = 99999;
		Options.Limit_Generated = 999999;
		Options.Limit_Invalid_Generated_States = 10000000;
		Options.Accepted_To_Generated_Ratio = 1.0E-4;

		Options.Cost_Precision = 1.0E-30;
		Options.Maximum_Cost_Repeat = 0;
		Options.Number_Cost_Samples = 5;
		Options.Temperature_Ratio_Scale = .1;
		Options.Cost_Parameter_Scale_Ratio = 1;
		Options.Temperature_Anneal_Scale = 100.0;

		Options.Include_Integer_Parameters = FALSE;
		Options.User_Initial_Parameters = FALSE;
		Options.Sequential_Parameters = -1;
		Options.Initial_Parameter_Temperature = .05;

		Options.Acceptance_Frequency_Modulus = 100;
		Options.Generated_Frequency_Modulus = 10000;
		Options.Reanneal_Cost = 1;
		Options.Reanneal_Parameters = 0;

		Options.Delta_X = 0.05;
		Options.User_Tangents = FALSE;
		Options.Curvature_0 = -1;
		Options.Asa_Recursive_Level = 0;
		
		/* Fit_Local, Iter_Max and Penalty may be set adaptively - for Simplex fit */
		//asatest.Options.Penalty = 1000;
		//asatest.Options.Fit_Local = 1;
		//asatest.Options.Iter_Max = 500;
		 
		user_generating_function = false;
		user_initial_parameters_temps = true;
		user_initial_cost_temp = false;

		multi_min = false;
		m_basa_sample = false;
		m_blocalfit = false;
		//
		m_bquench_cost = false;
	
 
 
		Options.Generating_Distrib = GenerateDistrib;

		//user_initial_cost_temp = true;
		/* store the parameter ranges */
		m_cgenome.UpdateBoundaries(parameter_upper_bound, parameter_lower_bound);


	   /* store the initial parameter types */
	   for (int index = 0; index < parameter_dimension; ++index)
			parameter_int_real[index] = REAL_TYPE;

   //Set the initial parameter temperature, and the 
   
   /* store the initial parameter values */
   for (int index = 0; index < parameter_dimension ; ++index) 
	  cost_parameters[index] = m_cgenome.GetMutatableParameter(index);

   //ASA options that are not currently used
	if(user_initial_parameters_temps)
	{
		USER_OPTIONS->User_Parameter_Temperature = (double *) calloc (parameter_dimension, sizeof (double));
		for (int index = 0; index < parameter_dimension; ++index)
				USER_OPTIONS->User_Parameter_Temperature[index] = .05;
	}
	
	if(user_initial_cost_temp)
	{
	    USER_OPTIONS->User_Cost_Temperature = 10.0;
	}

	if(delta_parameters)
	{
        for (int index = 0; index < parameter_dimension; ++index)
			USER_OPTIONS->User_Delta_Parameter[index] = 0.05;
	}

	if(quench_parameters)
	{
		for (int index = 0; index < parameter_dimension; ++index)
			USER_OPTIONS->User_Quench_Param_Scale[index] = 1.0;
	}

	if(m_bquench_cost)
	{
		USER_OPTIONS->User_Quench_Cost_Scale = 0.9;
	}

	if(ratio_temperature_scales)
	{
	    for (int index = 0; index < parameter_dimension; ++index)
			USER_OPTIONS->User_Temperature_Ratio[index] = 1.0;
	}
  
	USER_OPTIONS->Asa_Recursive_Level = 0;

	  return (0);
}

double GenerateDistrib(LONG_INT* seed, LONG_INT parameter_dimension, long int index_v, double temperature_v,
		double init_param_temp_v, double temp_scale_params_v, double parameter_v, double paramter_range_v, 
		double* last_saved_param, void* Options_tmp)
{

	if(((USER_DEFINES*)Options_tmp)->N_Generated < 100000)
		return random(1.05*parameter_v, 0.95*parameter_v);
	else if(((USER_DEFINES*)Options_tmp)->N_Generated < 2000000)
		return random(1.03*parameter_v, 0.97*parameter_v);
	else
		return random(1.01*parameter_v, 0.99*parameter_v);
}