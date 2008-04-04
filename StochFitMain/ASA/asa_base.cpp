/***********************************************************************
* Adaptive Simulated Annealing (ASA)
* Lester Ingber <ingber@ingber.com>
* Copyright (c) 1993-2008 Lester Ingber.  All Rights Reserved.
* The ASA-LICENSE file must be included with ASA code.
*
* This a modified version of ASA written by Stephen Danauskas
* The ASA code was split into classes,
* and several defines and options unneeded by x86 Windows were removed.
* With those exceptions, it is believed that the results for routines between the 
* official ASA code and this version are identical. The original version is
* located at http://www.ingber.com/
***********************************************************************/

#include "stdafx.h"
#include "asa_base.h"


int multi_compare (const void *ii, const void *jj);
ASA_Base* ptr;

ASA_Base::ASA_Base(wstring filename, int paramcount): m_bASAOpen(false),m_inumber_asa_open(0),m_bASA_print(false),m_sfile_name(filename),
m_irecursive_asa_open(0),m_bincl_stdout(true), m_bASA_recursive(false), ptr_asa_out(NULL), m_bASA_print_intermed(false),
m_basa_print_more(false),m_bdropped_parameters(false),m_basa_sample(false),m_buser_reanneal_parameters(false), 
multi_min(false),user_initial_cost_temp(false), ratio_temperature_scales(false), delta_parameters(false),
user_initial_parameters_temps(false),user_cost_schedule(false),optional_data_dbl(false), optional_data_ptr(false),
user_acceptance_test(false), user_generating_function(false),user_reanneal_cost(false), quench_parameters(false),
m_bquench_cost(false),  m_buser_accept_asymp_exp(false), m_buser_accept_threshold(false),m_bself_optimize(false), 
m_inumber_parameters(paramcount), asa_recursive_max(0),m_bquench_parameters_scale(false), m_bquench_cost_scale(false),
m_bno_param_temp_test(true), m_bno_cost_temp_test(true), exit_status(0), valid_state_generated_flag(TRUE),m_blocalfit(false),
m_buser_param_init(false), m_buser_cost_func(false), temperature_scale_cost(0), m_bASA_initialized(false)
{
	ptr = this;
	
	Options.Asa = (void*)this;

}

ASA_Base::~ASA_Base()
{
	if(m_bASA_initialized)
	{
		if(m_bASA_print && ptr_asa_out != NULL)
		{
			fprintf (ptr_asa_out, "\n\n\n");
			fflush (ptr_asa_out);
			fclose (ptr_asa_out);
		}

		 free (current_generated_state.parameter);
		 free (last_saved_state.parameter);
		 free (best_generated_state.parameter);

		 free (m_dparameter_maximum);
		 free (m_dparameter_minimum);
		 free (parameter_initial_final);
		 free (parameter_type);
		 
		 if(m_dtangents != NULL)
			free (m_dtangents);

		 if(m_dcurvature != NULL)
			 free (m_dcurvature);

		 if(delta_parameters)
			 free(Options.User_Delta_Parameter);
		 
		 if(quench_parameters)
			 free(Options.User_Quench_Param_Scale);

		 if(ratio_temperature_scales)
			 free(Options.User_Temperature_Ratio);

		 if(m_basa_sample)
			 free(Options.Bias_Generated);

		


		if(multi_min)
		{
		  for (int multi_index = 0; multi_index <= Options.Multi_Number; ++multi_index)
		  {
			free (multi_params[multi_index]);
			free (Options.Multi_Params[multi_index]);
		  }
		  free (Options.Multi_Params);
		  free (Options.Multi_Cost);
		  free (Options.Multi_Grid);
		  free (multi_params);
		  free (multi_sort);
		  free (multi_cost);
		}

		  free (initial_user_parameter_temp);
		  free (index_parameter_generations);
		  free (current_user_parameter_temp);
		  free (temperature_scale_parameters);
	}
	  
}

void ASA_Base::asa_alloc()
{

  m_dparameter_minimum =(double *) calloc (m_inumber_parameters, sizeof (double));
  m_dparameter_maximum = (double *) calloc (m_inumber_parameters, sizeof (double));
  parameter_initial_final =(double *) calloc (m_inumber_parameters, sizeof (double));
  parameter_type =(int *) calloc (m_inumber_parameters, sizeof (int));
  m_dtangents =(double *) calloc (m_inumber_parameters, sizeof (double));

  if (Options.Curvature_0 == FALSE || Options.Curvature_0 == -1) 
	  m_dcurvature =(double *) calloc ((m_inumber_parameters) *(m_inumber_parameters),sizeof (double));
  else 
	  m_dcurvature = (double *) NULL;

    //Allocate memory
	  current_generated_state.parameter =(double *) calloc (m_inumber_parameters, sizeof (double));
	  last_saved_state.parameter = (double *) calloc (m_inumber_parameters, sizeof (double));
	  best_generated_state.parameter =(double *) calloc (m_inumber_parameters, sizeof (double));
      initial_user_parameter_temp =(double *) calloc (m_inumber_parameters, sizeof (double));

	  index_parameter_generations =(LONG_INT *) calloc (m_inumber_parameters,sizeof (LONG_INT));
	  current_user_parameter_temp = (double *) calloc (m_inumber_parameters, sizeof (double));
	  temperature_scale_parameters = (double *) calloc (m_inumber_parameters, sizeof (double));
	  
	  if(multi_min)
		{
		  multi_cost = (double *) calloc (Options.Multi_Number + 1,  sizeof (double));
		  multi_cost_qsort = multi_cost;
		  multi_sort = (int *) calloc (Options.Multi_Number + 1, sizeof (int));
		  multi_params =  (double **) calloc (Options.Multi_Number + 1, sizeof (double *));
		  
		  for (multi_index = 0; multi_index <= Options.Multi_Number; ++multi_index)
			multi_params[multi_index] = (double *) calloc (m_inumber_parameters, sizeof (double));
		}

    if(user_generating_function)
	{
		Options.User_Parameter_Temperature = (double *) calloc (m_inumber_parameters, sizeof (double));
	}

	if(delta_parameters)
		Options.User_Delta_Parameter =  (double *) calloc (m_inumber_parameters, sizeof (double));

	if(quench_parameters)
		Options.User_Quench_Param_Scale =  (double *) calloc (m_inumber_parameters, sizeof (double));


	if(ratio_temperature_scales)
		Options.User_Temperature_Ratio =(double *) calloc (m_inumber_parameters, sizeof (double));

	if(m_basa_sample)
		Options.Bias_Generated = (double *) calloc (m_inumber_parameters, sizeof (double));

	if(multi_min)
	{
		if(Options.Multi_Number == 0)
			Options.Multi_Number = 1;

		Options.Multi_Cost = (double *) calloc (Options.Multi_Number,sizeof (double));
		Options.Multi_Grid =(double *) calloc (m_inumber_parameters, sizeof (double));
		Options.Multi_Params =(double **) calloc (Options.Multi_Number,sizeof (double *));
		
		for (multi_index = 0; multi_index < Options.Multi_Number; ++multi_index) 
			Options.Multi_Params[multi_index] =(double *) calloc (m_inumber_parameters, sizeof (double));
	}
	
	if(m_buser_param_init == false)
		initialize_parameters(parameter_initial_final,m_dparameter_minimum, m_dparameter_maximum,m_dtangents, m_dcurvature,
			m_inumber_parameters,parameter_type,&Options);
	else
		Options.Initialize_Parameters(parameter_initial_final,m_dparameter_minimum, m_dparameter_maximum,m_dtangents, m_dcurvature,
			m_inumber_parameters,parameter_type,&Options);

	
}

int ASA_Base::asa_init()
{
  ASA_Output_System();
  double log_new_temperature_ratio;

  //Initialize the random seed

  m_iseed = time(NULL) ;
  
  /* initialize random number generator with first call */
  resettable_randflt (&m_iseed, 1);

  if(m_bASA_print)
  {
	  /* print header information as defined by user */
	 print_asa_options ();
	 fflush (ptr_asa_out);
  }

  /* set indices and counts to 0 */
  best_number_generated_saved = m_inumber_generated = recent_number_generated = recent_number_acceptances = 0;
  index_cost_acceptances = best_number_accepted_saved = m_inumber_accepted = number_acceptances_saved = 0;
  index_cost_repeat = 0;

  Options.N_Accepted = m_inumber_accepted;
  Options.N_Generated = m_inumber_generated;

  if(m_basa_sample)
  {
	  Options.N_Generated = 0;
	  Options.Average_Weights = 1.0;
  }

	  /* do not calculate curvatures initially */
	  m_bcurvature_flag = FALSE;
  
	

	  Options.Best_Cost = &(best_generated_state.cost);
	  Options.Best_Parameters = best_generated_state.parameter;
	  Options.Last_Cost = &(last_saved_state.cost);
	  Options.Last_Parameters = last_saved_state.parameter;

	

	  if(user_initial_parameters_temps)
	  {
		for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
			current_user_parameter_temp[index_v] =initial_user_parameter_temp[index_v] = Options.User_Parameter_Temperature[index_v];
	  }
	  else
	  {
		for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
			current_user_parameter_temp[index_v] = initial_user_parameter_temp[index_v] = Options.Initial_Parameter_Temperature;
	  }

	if(user_initial_cost_temp)
	{
	  initial_cost_temperature = current_cost_temperature = Options.User_Cost_Temperature;

	  if(user_acceptance_test)
		  Options.Cost_Temp_Curr = Options.Cost_Temp_Init = initial_cost_temperature;
	}   
    
	 /* set parameters to the initial parameter values */
	  for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
		last_saved_state.parameter[index_v] = current_generated_state.parameter[index_v] = parameter_initial_final[index_v];

		if(user_acceptance_test)
		{
		  Options.Random_Seed = m_iseed;
		  Options.User_Acceptance_Flag = TRUE;
		  Options.Cost_Acceptance_Flag = FALSE;
		}

	  /* save initial user value of Options.Sequential_Parameters */
	  start_sequence = Options.Sequential_Parameters;

	  print_starting_parameters();

	  if (Options.Asa_Recursive_Level > asa_recursive_max)
			asa_recursive_max = Options.Asa_Recursive_Level;


		  tmp_var_int = cost_function_test (current_generated_state.cost,current_generated_state.parameter);

		  /* compute temperature scales */
		  tmp_var_db1 = -log((Options.Temperature_Ratio_Scale));
		  tmp_var_db2 = log(Options.Temperature_Anneal_Scale);
		  temperature_scale = tmp_var_db1 * exp(-tmp_var_db2 / m_dxnumber_parameters);

		  /* set here in case not used */
		  tmp_var_db = 0.0;

		if(quench_parameters)
		{

			if(ratio_temperature_scales)
			{
			  if(m_bquench_parameters_scale)
			  {
				for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
					temperature_scale_parameters[index_v] = tmp_var_db1 * exp(-(tmp_var_db2 * Options.User_Quench_Param_Scale[index_v]) / m_dxnumber_parameters)
					* Options.User_Temperature_Ratio[index_v];
			  }
			  else
			  {
				 for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
					 temperature_scale_parameters[index_v] = tmp_var_db1 * exp(-(tmp_var_db2) / m_dxnumber_parameters)
					  * Options.User_Temperature_Ratio[index_v];
			  }
			}   
			else
			{
			  if(m_bquench_parameters_scale)
			  {
				 for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
					 temperature_scale_parameters[index_v] = tmp_var_db1 * exp(-(tmp_var_db2 * Options.User_Quench_Param_Scale[index_v])  / m_dxnumber_parameters);
			  }
			  else
			  {
				for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
					temperature_scale_parameters[index_v] = tmp_var_db1 * exp(-(tmp_var_db2)  / m_dxnumber_parameters);
			  }
			}   
		}
		else
		{
			if(ratio_temperature_scales)
			{
			  for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
				temperature_scale_parameters[index_v] = tmp_var_db1 * exp (-(tmp_var_db2) / m_dxnumber_parameters)* Options.User_Temperature_Ratio[index_v];
			}
			else
			{
			  for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
				temperature_scale_parameters[index_v] = tmp_var_db1 * exp(-(tmp_var_db2) / m_dxnumber_parameters);
			}
		}



		if(m_bquench_cost)
		{
				if(m_bquench_cost_scale)
				{
				  temperature_scale_cost = tmp_var_db1 * exp (-(tmp_var_db2 * Options.User_Quench_Cost_Scale) / m_dxnumber_parameters) * Options.Cost_Parameter_Scale_Ratio;
				}
				else
				{
				  temperature_scale_cost = tmp_var_db1 * exp (-(tmp_var_db2) / m_dxnumber_parameters) * Options.Cost_Parameter_Scale_Ratio;
				}
		}
		else
		{
			temperature_scale_cost = tmp_var_db1 * exp(-(tmp_var_db2) / m_dxnumber_parameters) * Options.Cost_Parameter_Scale_Ratio;
		}
		    
		if(user_acceptance_test)
			Options.Cost_Temp_Scale = temperature_scale_cost;

		  /* set the initial index of parameter generations to 1 */
		  for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
			  index_parameter_generations[index_v] = 1;

		  /* test user-defined options before calling cost function */
		  tmp_var_int = asa_test_asa_options ();

		  if (tmp_var_int > 0)
		  {
			if(m_bASA_print)
			{
				fprintf (ptr_asa_out, "total number invalid Options = %d\n", tmp_var_int);
				fflush (ptr_asa_out);
			}
			exit_status = INVALID_USER_INPUT;
			asa_exit();
			return -1;
		  }

		if(user_initial_cost_temp == false)
		{
		  /* calculate the average cost over samplings of the cost function */
		  if (Options.Number_Cost_Samples < -1) 
		  {
			tmp_var_db1 = 0.0;
			tmp_var_db2 = 0.0;
			tmp_var_int = -Options.Number_Cost_Samples;
		  }
		  else 
		  {
			tmp_var_db1 = 0.0;
			tmp_var_int = Options.Number_Cost_Samples;
		  }

		  Options.Locate_Cost = 0;     /* initial cost temp */

		  
		  for (int index_cost_constraint = 0; index_cost_constraint < tmp_var_int; ++index_cost_constraint) 
		  {
			m_inumber_invalid_generated_states = 0;
			m_irepeated_invalid_states = 0;
			Options.Sequential_Parameters = start_sequence - 1;
			do {
			  ++(m_inumber_invalid_generated_states);
			  generate_new_state ();

			  valid_state_generated_flag = TRUE;

				if(user_acceptance_test)
				{
					  Options.User_Acceptance_Flag = TRUE;
					  Options.Cost_Acceptance_Flag = FALSE;
				}
			  
				tmp_var_db = CalcCostFunc(current_generated_state.parameter);
		      
			  if (cost_function_test(tmp_var_db, current_generated_state.parameter) == 0) 
			  {
				exit_status = INVALID_COST_FUNCTION;
				asa_exit();
				return -1;
			  }

			  ++m_irepeated_invalid_states;
			  if (m_irepeated_invalid_states > Options.Limit_Invalid_Generated_States)
			  {
				exit_status = TOO_MANY_INVALID_STATES;
				asa_exit();
				return -1;
			  }
			}
			while (valid_state_generated_flag == FALSE);
		
			--(m_inumber_invalid_generated_states);

			if (Options.Number_Cost_Samples < -1) 
			{
			  tmp_var_db1 += tmp_var_db;
			  tmp_var_db2 += (tmp_var_db * tmp_var_db);
			} else 
			{
			  tmp_var_db1 += fabs (tmp_var_db);
			}
		  }

		  if (Options.Number_Cost_Samples < -1) 
		  {
			tmp_var_db1 /= (double) tmp_var_int;
			tmp_var_db2 /= (double) tmp_var_int;
			tmp_var_db = sqrt (fabs ((tmp_var_db2 - tmp_var_db1 * tmp_var_db1) * ((double) tmp_var_int / ((double) tmp_var_int - 1.0))))
						+ (double) EPS_DOUBLE;
		  }
		  else 
		  {
			tmp_var_db = tmp_var_db1 / (double) tmp_var_int;
		  }

		  if(user_acceptance_test)
			  Options.Cost_Temp_Curr = Options.Cost_Temp_Init = initial_cost_temperature = current_cost_temperature = tmp_var_db;
		  else
			  initial_cost_temperature = current_cost_temperature = tmp_var_db;
		}

		  /* set all parameters to the initial parameter values */
		  for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
			best_generated_state.parameter[index_v] =last_saved_state.parameter[index_v] =
				current_generated_state.parameter[index_v] = parameter_initial_final[index_v];

		  Options.Locate_Cost = 1;     /* initial cost value */

		  /* if using user's initial parameters */
		  if (Options.User_Initial_Parameters == TRUE) {
			valid_state_generated_flag = TRUE;

			if(user_acceptance_test)
			{
				Options.User_Acceptance_Flag = TRUE;
				Options.Cost_Acceptance_Flag = FALSE;
			}


			  current_generated_state.cost = CalcCostFunc(current_generated_state.parameter);

			if (cost_function_test(current_generated_state.cost, current_generated_state.parameter) == 0) 
			{
			  exit_status = INVALID_COST_FUNCTION;
			  asa_exit();
			  return -1;
			}
			if(m_bASA_print)
			{
				if (valid_state_generated_flag == FALSE)
					  fprintf (ptr_asa_out, "user's initial parameters generated FALSE *valid_state_generated_flag\n");
			}
		  }
		  else
		  {
			/* let asa generate valid initial parameters */
			m_irepeated_invalid_states = 0;
			Options.Sequential_Parameters = start_sequence - 1;
			do {
			  ++(m_inumber_invalid_generated_states);
			  generate_new_state ();
		      
			  valid_state_generated_flag = TRUE;

			  if(user_acceptance_test)
			  {
				  Options.User_Acceptance_Flag = TRUE;
				  Options.Cost_Acceptance_Flag = FALSE;
			  }

			  current_generated_state.cost = CalcCostFunc(current_generated_state.parameter);
		      
			  if (cost_function_test(current_generated_state.cost,current_generated_state.parameter) == 0) 
			  {
				exit_status = INVALID_COST_FUNCTION;
				asa_exit();
				return -1;
			  }
			  ++m_irepeated_invalid_states;
			  if (m_irepeated_invalid_states > Options.Limit_Invalid_Generated_States) {
				exit_status = TOO_MANY_INVALID_STATES;
				asa_exit();
				return -1;
			  }
			}
			while (valid_state_generated_flag == FALSE);
			--(m_inumber_invalid_generated_states);
		  }                             /* Options.User_Initial_Parameters */

		  /* set all states to the last one generated */
		  for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
		  {
			if(m_bdropped_parameters)
			{
				/* ignore parameters that have too small a range */
				if (PARAMETER_RANGE_TOO_SMALL (index_v))
				  continue;
			}
			best_generated_state.parameter[index_v] =  last_saved_state.parameter[index_v] = current_generated_state.parameter[index_v];
		  }

		  /* set all costs to the last one generated */
		  best_generated_state.cost = last_saved_state.cost = current_generated_state.cost;

		  m_daccepted_to_generated_ratio = 1.0;

		  /* do not calculate curvatures initially */
		  m_bcurvature_flag = FALSE;

		  if(m_bASA_print)
		  {
			  fprintf (ptr_asa_out,"temperature_scale = %*.*g\n", G_FIELD, G_PRECISION, temperature_scale);
				
			  if(ratio_temperature_scales)
				{
					if(m_bASA_print_intermed)
					{
					  for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
						fprintf (ptr_asa_out,"temperature_scale_parameters[%ld] = %*.*g\n", index_v,
								 G_FIELD, G_PRECISION, temperature_scale_parameters[index_v]);
					}	
				}
				else
				{
				  fprintf (ptr_asa_out, "temperature_scale_parameters[0] = %*.*g\n",G_FIELD, G_PRECISION, temperature_scale_parameters[0]);
				}
			  fprintf (ptr_asa_out, "temperature_scale_cost = %*.*g\n", G_FIELD, G_PRECISION, temperature_scale_cost);
			  fprintf (ptr_asa_out, "\n\n");

			  if(m_basa_print_more)
			  {
				print_state ();
			  }

			  fprintf (ptr_asa_out, "\n");
			  fflush (ptr_asa_out);
			}

			if(m_basa_sample)
			{
				if(m_bASA_print)
				{
				  fprintf (ptr_asa_out, ":SAMPLE:   n_accept   cost        cost_temp    bias_accept    aver_weight\n");
				  fprintf (ptr_asa_out, ":SAMPLE:   index      param[]     temp[]       bias_gener[]   range[]\n");
				}
			}

		  /* reset the current cost and the number of generations performed */
		  m_inumber_invalid_generated_states = 0;
		  best_number_generated_saved = m_inumber_generated = recent_number_generated = 0;
		  Options.N_Generated = m_inumber_generated;
		  for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
		  {
			/* ignore parameters that have too small a range */
			if (PARAMETER_RANGE_TOO_SMALL (index_v))
			  continue;
			index_parameter_generations[index_v] = 1;
		  }
		  if(user_acceptance_test)
		  {
			  Options.User_Acceptance_Flag = FALSE;
			  Options.Cost_Acceptance_Flag = FALSE;
		  }

		if(multi_min)
		{
		  multi_sort[Options.Multi_Number] = Options.Multi_Number;
		  multi_cost[Options.Multi_Number] = current_generated_state.cost;
		 
		  for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
			multi_params[Options.Multi_Number][index_v] = current_generated_state.parameter[index_v];
		  
		  for (multi_index = 0; multi_index < Options.Multi_Number; ++multi_index) {
			multi_sort[multi_index] = multi_index;
			multi_cost[multi_index] = Options.Multi_Cost[multi_index] = current_generated_state.cost;
		   
			for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
			  multi_params[multi_index][index_v] =Options.Multi_Params[multi_index][index_v] = current_generated_state.parameter[index_v];
		    
		  }
		}

		  /* this test is after MULTI_MIN so that params are not all just set to 0 */
		  if (initial_cost_temperature < (double) EPS_DOUBLE) 
		  {
			  if(m_bASA_print)
			  {
				fprintf (ptr_asa_out, "*initial_cost_temperature (= %g) < EPS_DOUBLE\n",initial_cost_temperature);
				fflush (ptr_asa_out);
			  }
				exit_status = INVALID_COST_FUNCTION;
				asa_exit();
				return -1;
		  }

		  Options.Sequential_Parameters = start_sequence - 1;
		  
		  m_bASA_initialized = true;

		  return 0;
}

double ASA_Base::CalcCostFunc(double* parameters)
{
	if(m_buser_cost_func == false)
		return cost_function(parameters, m_dparameter_minimum,m_dparameter_maximum,m_dtangents,
			m_dcurvature,m_inumber_parameters,parameter_type,&valid_state_generated_flag, &exit_status, &Options);
	else
		return Options.Cost_Function(parameters, m_dparameter_minimum,m_dparameter_maximum,m_dtangents,
			m_dcurvature,m_inumber_parameters,parameter_type,&valid_state_generated_flag, &exit_status, &Options);

}

double ASA_Base::asa_loop()
{
	double cost_value = 0;

	while (((m_inumber_accepted <= Options.Limit_Acceptances) || (Options.Limit_Acceptances == 0))
			&& ((m_inumber_generated <= Options.Limit_Generated)|| (Options.Limit_Generated == 0))) 
	 {
		  if(asa_iteration() == -1)
			  break;
	 }


	

	exit_status = NORMAL_EXIT;
	asa_exit();

	if(m_blocalfit)
	  cost_value = fitloc();
	else
	  cost_value = final_cost;

	return final_cost;
}



int ASA_Base::asa_iteration()
{
	double log_new_temperature_ratio;
	int tmp_var_int1,tmp_var_int2;

		tmp_var_db1 = -log((Options.Temperature_Ratio_Scale));

		/* compute temperature scales */
		tmp_var_db2 = log(Options.Temperature_Anneal_Scale);
		temperature_scale = tmp_var_db1 * exp(-tmp_var_db2 / m_dxnumber_parameters);

	if(quench_parameters)
	{
		if(ratio_temperature_scales)
		{
			for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
			{
				if(m_bquench_parameters_scale)
					 temperature_scale_parameters[index_v] = tmp_var_db1 * exp(-(tmp_var_db2 * Options.User_Quench_Param_Scale[index_v]) / m_dxnumber_parameters)
						  * Options.User_Temperature_Ratio[index_v];
				else
					 temperature_scale_parameters[index_v] = tmp_var_db1 * exp(-(tmp_var_db2) / m_dxnumber_parameters)
						  * Options.User_Temperature_Ratio[index_v];
			}
		}
		else
		{
			for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
			{
				if(m_bquench_parameters_scale)
					 temperature_scale_parameters[index_v] = tmp_var_db1 * exp(-(tmp_var_db2 * Options.User_Quench_Param_Scale[index_v]) / m_dxnumber_parameters);
				else
					 temperature_scale_parameters[index_v] = tmp_var_db1 * exp(-(tmp_var_db2) / m_dxnumber_parameters);

			}
		}
	}
	else
	{
		if(ratio_temperature_scales)
		{
			for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
			  temperature_scale_parameters[index_v] = tmp_var_db1 * exp (-(tmp_var_db2) / m_dxnumber_parameters)* Options.User_Temperature_Ratio[index_v];
		}
		else
		{
			for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
			  temperature_scale_parameters[index_v] =  tmp_var_db1 * exp(-(tmp_var_db2) / m_dxnumber_parameters);
		}
	}



	if(m_bquench_cost)
	{

		if(m_bquench_cost_scale)
		{
			temperature_scale_cost = tmp_var_db1 * exp (-(tmp_var_db2 * Options.User_Quench_Cost_Scale) / m_dxnumber_parameters) * Options.Cost_Parameter_Scale_Ratio;
		}
		else
		{
			temperature_scale_cost =   tmp_var_db1 * exp (-(tmp_var_db2) / m_dxnumber_parameters) * Options.Cost_Parameter_Scale_Ratio;
		}
	}
	else
	{
		  temperature_scale_cost = tmp_var_db1 * exp(-(tmp_var_db2)/ m_dxnumber_parameters) * Options.Cost_Parameter_Scale_Ratio;
	}

	if(user_acceptance_test)
		Options.Cost_Temp_Scale = temperature_scale_cost;

		/* CALCULATE NEW TEMPERATURES */

		/* calculate new parameter temperatures */
		for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
		{
		  /* skip parameters with too small a range */
		  if (PARAMETER_RANGE_TOO_SMALL (index_v))
			continue;

		  if(quench_parameters)
		  {
			log_new_temperature_ratio = -temperature_scale_parameters[index_v] * pow((double) index_parameter_generations[index_v],
				  Options.User_Quench_Param_Scale[index_v] / m_dxnumber_parameters);
		  }
		  else
		  {
			log_new_temperature_ratio = -temperature_scale_parameters[index_v] * pow((double) index_parameter_generations[index_v],
				1.0 / m_dxnumber_parameters);
		  }
	      
	             
		  /* check (and correct) for too large an exponent */
		  log_new_temperature_ratio = EXPONENT_CHECK (log_new_temperature_ratio);
		  current_user_parameter_temp[index_v] = initial_user_parameter_temp[index_v]* exp(log_new_temperature_ratio);

			if(m_bno_param_temp_test)
			{
				  if (current_user_parameter_temp[index_v] < (double) EPS_DOUBLE)
					current_user_parameter_temp[index_v] = (double) EPS_DOUBLE;
			}
			else
			{
				  /* check for too small a parameter temperature */
				  if (current_user_parameter_temp[index_v] < (double) EPS_DOUBLE) {
					exit_status = P_TEMP_TOO_SMALL;
					index_exit_v = index_v;
					asa_exit();
					return -1;
				  }
			}
		}

		/* calculate new cost temperature */

		if(m_bquench_cost)
			log_new_temperature_ratio = -temperature_scale_cost * pow((double)index_cost_acceptances,Options.User_Quench_Cost_Scale/ m_dxnumber_parameters);
		else
			log_new_temperature_ratio = -temperature_scale_cost * pow((double)index_cost_acceptances,1.0/ m_dxnumber_parameters);
	                                        
		log_new_temperature_ratio = EXPONENT_CHECK (log_new_temperature_ratio);

		current_cost_temperature = initial_cost_temperature * exp (log_new_temperature_ratio);

		if(user_acceptance_test)
			 Options.Cost_Temp_Curr = Options.Cost_Temp_Init = current_cost_temperature;

	if(m_bno_cost_temp_test)
	{
			if (current_cost_temperature < (double) EPS_DOUBLE)
			{
					current_cost_temperature = (double) EPS_DOUBLE;

					if(user_acceptance_test)
						Options.Cost_Temp_Curr = current_cost_temperature;
			}
	}
	else
	{
		/* check for too small a cost temperature */
		if (current_cost_temperature < (double) EPS_DOUBLE) 
		{
		  exit_status = C_TEMP_TOO_SMALL;
		  asa_exit();
		  return -1;
		}
	}
		/* GENERATE NEW PARAMETERS */

		/* generate a new valid set of parameters */

		  if (Options.Locate_Cost < 0) {
			Options.Locate_Cost = 12;      /* generate new state from new best */
		  } else {
			Options.Locate_Cost = 2;       /* generate new state */
		  }

		  m_irepeated_invalid_states = 0;
		  do {
			++(m_inumber_invalid_generated_states);
			generate_new_state ();

			valid_state_generated_flag = TRUE;

			if(user_acceptance_test)
			{
				Options.User_Acceptance_Flag = FALSE;
				Options.Cost_Acceptance_Flag = FALSE;
			}

			tmp_var_db = CalcCostFunc(current_generated_state.parameter);

			if (cost_function_test (tmp_var_db,  current_generated_state.parameter) == 0) 
			{
			  exit_status = INVALID_COST_FUNCTION;
			  asa_exit();
			  return -1;
			}

			current_generated_state.cost = tmp_var_db;
			++m_irepeated_invalid_states;
			if (m_irepeated_invalid_states > Options.Limit_Invalid_Generated_States)
			{
			  exit_status = TOO_MANY_INVALID_STATES;
			  asa_exit();
			  return -1;
			}
		  }
		  while (valid_state_generated_flag == FALSE);
		  --(m_inumber_invalid_generated_states);
		/* ACCEPT/REJECT NEW PARAMETERS */

		  /* decide to accept/reject the new state */
		  accept_new_state ();


		  /* calculate the ratio of acceptances to generated states */
		  m_daccepted_to_generated_ratio = (double) (recent_number_acceptances + 1) /(double) (recent_number_generated + 1);

	if(multi_min)
	{
		  if (((Options.Multi_Specify == 0)&& (current_generated_state.cost <= best_generated_state.cost))|| 
			  ((Options.Multi_Specify == 1) && (current_generated_state.cost < best_generated_state.cost)))
		  {
			for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
			{
			  if (Options.Multi_Grid[index_v] < EPS_DOUBLE)
				Options.Multi_Grid[index_v] = EPS_DOUBLE;
			}

			multi_test = 0;
			for (multi_index = 0; multi_index < Options.Multi_Number; ++multi_index) 
			{
			  multi_test_cmp = 0;
			  multi_test_dim = 0;
			  for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
			  {
				if (PARAMETER_RANGE_TOO_SMALL (index_v))
				  continue;
				++multi_test_dim;
				if (fabs (current_generated_state.parameter[index_v]  - Options.Multi_Params[multi_index][index_v])
					< Options.Multi_Grid[index_v] - EPS_DOUBLE)
				  ++multi_test_cmp;
			  }
			  if (multi_test_cmp == multi_test_dim)
				multi_test = 1;
			  if (Options.Multi_Specify == 1)
				break;
			}

			if (multi_test == 0) 
			{
			  multi_cost[Options.Multi_Number] = current_generated_state.cost;

			 for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
				multi_params[Options.Multi_Number][index_v] = current_generated_state.parameter[index_v];
	          
			 for (multi_index = 0; multi_index < Options.Multi_Number; ++multi_index) 
			  {
				multi_cost[multi_index] = Options.Multi_Cost[multi_index];
	   
				for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
				  multi_params[multi_index][index_v] =  Options.Multi_Params[multi_index][index_v];
			  }

			  qsort (multi_sort, Options.Multi_Number + 1, sizeof (int),  multi_compare);
			
			  for (multi_index = 0; multi_index < Options.Multi_Number; ++multi_index) 
			  {
				 Options.Multi_Cost[multi_index] = multi_cost[multi_sort[multi_index]];
	           
				 for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
				  Options.Multi_Params[multi_index][index_v] = multi_params[multi_sort[multi_index]][index_v];
			  }
			}
		  }
	}

		  /* CHECK FOR NEW MINIMUM */

		  if (current_generated_state.cost < best_generated_state.cost) 
			best_flag = 1;
		  else 
			best_flag = 0;

		if(multi_min)
		  {

			  if (((Options.Multi_Specify == 0) && (current_generated_state.cost <= best_generated_state.cost))|| ((Options.Multi_Specify == 1)
				  && (current_generated_state.cost < best_generated_state.cost)))
			  {
				  /* NEW MINIMUM FOUND */
				Options.Locate_Cost = -1;

				/* reset the recent acceptances and generated counts */
				recent_number_acceptances = recent_number_generated = 0;
			
				if (best_flag == 1) 
				{
				  best_number_generated_saved = m_inumber_generated;
				  best_number_accepted_saved = m_inumber_accepted;
				}
				index_cost_repeat = 0;

				/* copy the current state into the best_generated state */
				best_generated_state.cost = current_generated_state.cost;
				 for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
				 {
					if(m_bdropped_parameters)
					{
							  if (PARAMETER_RANGE_TOO_SMALL (index_v))
								continue;
					}
					  best_generated_state.parameter[index_v] = current_generated_state.parameter[index_v];
				}
			  }
		}
		else
		{
		  if (current_generated_state.cost < best_generated_state.cost)
		  {
			/* NEW MINIMUM FOUND */

			Options.Locate_Cost = -1;

			/* reset the recent acceptances and generated counts */
			recent_number_acceptances = recent_number_generated = 0;
			if (best_flag == 1) 
			{
			  best_number_generated_saved = m_inumber_generated;
			  best_number_accepted_saved = m_inumber_accepted;
			}
			index_cost_repeat = 0;

			/* copy the current state into the best_generated state */
			best_generated_state.cost = current_generated_state.cost;
	        
			for (int index_v = 0; index_v < m_inumber_parameters; ++index_v) 
			{
				if(m_bdropped_parameters)
				{
						  /* ignore parameters that have too small a range */
						  if (PARAMETER_RANGE_TOO_SMALL (index_v))
							continue;
				}
				  best_generated_state.parameter[index_v] = current_generated_state.parameter[index_v];
			}
		  
			/* printout the new minimum state and value */
			if(m_bASA_print)
				{
					fprintf (ptr_asa_out, "best...->cost=%-*.*g  *number_accepted=%ld  *number_generated=%ld\n",G_FIELD, G_PRECISION, best_generated_state.cost,
									 m_inumber_accepted, m_inumber_generated);
					if(m_basa_print_more)
					{
						if (best_flag == 1) 
						 fprintf (ptr_asa_out, "\nnew best\n");
					}
					
					if (best_flag == 1 && m_basa_print_more) 
					{
					  fprintf (ptr_asa_out, "Present Random Seed = %ld\n", m_iseed);

					  print_state ();
					}
					fflush (ptr_asa_out);
				}
		  }
		}

		if (Options.Immediate_Exit == TRUE) 
		{
		  exit_status = IMMEDIATE_EXIT;
		  asa_exit();
		  return -1;
		}

		/* PERIODIC TESTING/REANNEALING/PRINTING SECTION */

		if (Options.Acceptance_Frequency_Modulus == 0)
		  tmp_var_int1 = FALSE;
		else if ((int) (m_inumber_accepted %((LONG_INT) Options.Acceptance_Frequency_Modulus)) == 0 && number_acceptances_saved == m_inumber_accepted)
		  tmp_var_int1 = TRUE;
		else
		  tmp_var_int1 = FALSE;

		if (Options.Generated_Frequency_Modulus == 0)
		  tmp_var_int2 = FALSE;
		else if ((int) (m_inumber_generated %((LONG_INT) Options.Generated_Frequency_Modulus)) == 0)
		  tmp_var_int2 = TRUE;
		else
		  tmp_var_int2 = FALSE;

		if (tmp_var_int1 == TRUE || tmp_var_int2 == TRUE || (m_daccepted_to_generated_ratio < Options.Accepted_To_Generated_Ratio)) {
		  if (m_daccepted_to_generated_ratio < (Options.Accepted_To_Generated_Ratio))
			recent_number_acceptances = recent_number_generated = 0;

		  /* if best.cost repeats Options.Maximum_Cost_Repeat then exit */
		  if (Options.Maximum_Cost_Repeat != 0) 
		  {
			if (fabs (last_saved_state.cost - best_generated_state.cost) < Options.Cost_Precision) 
			{
			  ++index_cost_repeat;
			  if (index_cost_repeat == (Options.Maximum_Cost_Repeat)) 
			  {
				exit_status = COST_REPEATING;
				asa_exit();
				return -1;
			  }
			} 
			else {
			  index_cost_repeat = 0;
			}
		  }

		  if (Options.Reanneal_Parameters == TRUE) {
			Options.Locate_Cost = 3;       /* reanneal parameters */

			/* calculate m_dtangents, not curvatures, to reanneal */
			m_bcurvature_flag = FALSE;
	        
			cost_derivatives ();
	        
			if (exit_status == INVALID_COST_FUNCTION_DERIV)
			{
			  	asa_exit();
				return -1;
			}

		  }
		  
		if(!user_reanneal_cost)
		{
		  if (!(Options.Reanneal_Cost == 0 || Options.Reanneal_Cost == 1) )
		  {
			if (Options.Reanneal_Cost < -1) 
			{
			  tmp_var_int = -Options.Reanneal_Cost;
			} 
			else 
			{
			  tmp_var_int = Options.Reanneal_Cost;
			}
			tmp_var_db1 = 0.0;
			tmp_var_db2 = 0.0;

			for (int index_cost_constraint = 0; index_cost_constraint < tmp_var_int; ++index_cost_constraint) 
			{
			  Options.Locate_Cost = 4;     /* reanneal cost */

			  m_inumber_invalid_generated_states = 0;
			  m_irepeated_invalid_states = 0;
			  Options.Sequential_Parameters = start_sequence - 1;
			  do {
				++(m_inumber_invalid_generated_states);
				generate_new_state ();

				valid_state_generated_flag = TRUE;

				if(user_acceptance_test)
				{
					Options.User_Acceptance_Flag = TRUE;
					Options.Cost_Acceptance_Flag = FALSE;
				}

				tmp_var_db =  CalcCostFunc(current_generated_state.parameter);

				if (cost_function_test(tmp_var_db, current_generated_state.parameter) == 0) 
				{
				  exit_status = INVALID_COST_FUNCTION;
				  asa_exit();
				  return -1;
				}

				++m_irepeated_invalid_states;
				if (m_irepeated_invalid_states > Options.Limit_Invalid_Generated_States) 
				{
				  exit_status = TOO_MANY_INVALID_STATES;
				  asa_exit();
				  return -1;
				}
			  }
			  while (valid_state_generated_flag == FALSE);
			  --(m_inumber_invalid_generated_states);

			  tmp_var_db1 += tmp_var_db;
			  tmp_var_db2 += (tmp_var_db * tmp_var_db);
			}
			tmp_var_db1 /= (double) tmp_var_int;
			tmp_var_db2 /= (double) tmp_var_int;
			tmp_var_db = sqrt (fabs((tmp_var_db2 - tmp_var_db1 * tmp_var_db1) * ((double) tmp_var_int /
				((double) tmp_var_int - 1.0))));
	        
			if (Options.Reanneal_Cost < -1) 
			{
			  current_cost_temperature = initial_cost_temperature =  tmp_var_db + (double) EPS_DOUBLE;
			} 
			else 
			{
			  initial_cost_temperature = tmp_var_db + (double) EPS_DOUBLE;
			}
		  }
		}

		  reanneal ();

		  if(m_bASA_print && m_bASA_print_intermed)
			{
				  print_state ();
				  fprintf (ptr_asa_out, "\n");
				  fflush (ptr_asa_out);
			}
		}	

		return 0;
}


int ASA_Base::asa_exit ()
{
  int curvatureFlag, tmp_locate,multi_index;


  tmp_locate = Options.Locate_Cost;

  /* return final function minimum and associated parameters */
  final_cost = best_generated_state.cost;
  for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
    parameter_initial_final[index_v] = best_generated_state.parameter[index_v];

  Options.N_Accepted = best_number_accepted_saved;
  Options.N_Generated = best_number_generated_saved;

if(multi_min)
{
  for (multi_index = Options.Multi_Number - 1; multi_index >= 0;--multi_index) 
  {
	  best_generated_state.cost = Options.Multi_Cost[multi_index];
  
	  for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
		  best_generated_state.parameter[index_v] = Options.Multi_Params[multi_index][index_v];
	    
	  if(m_bASA_print)
			fprintf (ptr_asa_out, "\n\t\t multi_index = %d\n", multi_index);

		if (exit_status != TOO_MANY_INVALID_STATES && exit_status != IMMEDIATE_EXIT && exit_status != INVALID_USER_INPUT
				&& exit_status != INVALID_COST_FUNCTION && exit_status != INVALID_COST_FUNCTION_DERIV) 
		{
		  if (Options.Curvature_0 != TRUE)
			Options.Locate_Cost = 5;       /* calc curvatures while exiting asa */

		  /* calculate curvatures and tangents at best point */
		  curvatureFlag = TRUE;
		  cost_derivatives ();
		}
		if(m_bASA_print)
		{
			if (exit_status == INVALID_COST_FUNCTION_DERIV)
			  fprintf (ptr_asa_out, "\n\n  in asa_exit: INVALID_COST_FUNCTION_DERIV");

			if (exit_status != INVALID_USER_INPUT && exit_status != INVALID_COST_FUNCTION && exit_status != INVALID_COST_FUNCTION_DERIV)
			  print_state ();
		}
  }

  best_generated_state.cost = Options.Multi_Cost[0];

  for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
    best_generated_state.parameter[index_v] = Options.Multi_Params[0][index_v];
}
  else
  {
	 if (exit_status != TOO_MANY_INVALID_STATES && exit_status != IMMEDIATE_EXIT && exit_status != INVALID_USER_INPUT
			&& exit_status != INVALID_COST_FUNCTION && exit_status != INVALID_COST_FUNCTION_DERIV) 
		{
		  if (Options.Curvature_0 != TRUE)
			Options.Locate_Cost = 5;       /* calc curvatures while exiting asa */

		  /* calculate curvatures and tangents at best point */
		  curvatureFlag = TRUE;
		  cost_derivatives ();
		}
	 if(m_bASA_print)
		{
			if (exit_status == INVALID_COST_FUNCTION_DERIV)
			  fprintf (ptr_asa_out, "\n\n  in asa_exit: INVALID_COST_FUNCTION_DERIV");

			if (exit_status != INVALID_USER_INPUT && exit_status != INVALID_COST_FUNCTION && exit_status != INVALID_COST_FUNCTION_DERIV)
			  print_state ();
		}
  }

if(m_bASA_print)
{
  switch (exit_status) 
  {
	  case NORMAL_EXIT:
		fprintf (ptr_asa_out,"\n\n NORMAL_EXIT exit_status = %d\n", exit_status);
		break;
	  case P_TEMP_TOO_SMALL:
		fprintf (ptr_asa_out,"\n\n P_TEMP_TOO_SMALL exit_status = %d\n", exit_status);
		fprintf (ptr_asa_out,"current_user_parameter_temp[%ld] too small = %*.*g\n", index_exit_v,
				 G_FIELD, G_PRECISION, current_user_parameter_temp[index_exit_v]);
		break;
	  case C_TEMP_TOO_SMALL:
		fprintf (ptr_asa_out,"\n\n C_TEMP_TOO_SMALL exit_status = %d\n", exit_status);
		fprintf (ptr_asa_out,"*current_cost_temperature too small = %*.*g\n",G_FIELD, G_PRECISION, current_cost_temperature);
		break;
	  case COST_REPEATING:
		fprintf (ptr_asa_out,"\n\n COST_REPEATING exit_status = %d\n", exit_status);
		break;
	  case TOO_MANY_INVALID_STATES:
		fprintf (ptr_asa_out,"\n\n  TOO_MANY_INVALID_STATES exit_status = %d\n",exit_status);
		break;
	  case IMMEDIATE_EXIT:
		fprintf (ptr_asa_out,"\n\n  IMMEDIATE_EXIT exit_status = %d\n", exit_status);
		break;
	  case INVALID_USER_INPUT:
		fprintf (ptr_asa_out,"\n\n  INVALID_USER_INPUT exit_status = %d\n", exit_status);
		break;
	  case INVALID_COST_FUNCTION:
		fprintf (ptr_asa_out,"\n\n  INVALID_COST_FUNCTION exit_status = %d\n", exit_status);
		break;
	  case INVALID_COST_FUNCTION_DERIV:
		fprintf (ptr_asa_out,"\n\n  INVALID_COST_FUNCTION_DERIV exit_status = %d\n", exit_status);
		break;
	  default:
		fprintf (ptr_asa_out, "\n\n ERR: no exit code available = %d\n", exit_status);
  }

  switch (Options.Locate_Cost)
  {
	  case 0:
		fprintf (ptr_asa_out, " Locate_Cost = %d, initial cost temperature\n",Options.Locate_Cost);
		break;
	  case 1:
		fprintf (ptr_asa_out," Locate_Cost = %d, initial cost value\n", Options.Locate_Cost);
		break;
	  case 2:
		fprintf (ptr_asa_out, " Locate_Cost = %d, new generated state\n", Options.Locate_Cost);
		break;
	  case 12:
		fprintf (ptr_asa_out," Locate_Cost = %d, new generated state just after a new best state\n", Options.Locate_Cost);
		break;
	  case 3:
		fprintf (ptr_asa_out," Locate_Cost = %d, cost derivatives, reannealing parameters\n",Options.Locate_Cost);
		break;
	  case 4:
		fprintf (ptr_asa_out," Locate_Cost = %d, reannealing cost temperature\n", Options.Locate_Cost);
		break;
	  case 5:
		fprintf (ptr_asa_out," Locate_Cost = %d, calculating curvatures while exiting asa ()\n", Options.Locate_Cost);
		break;
	  case -1:
		fprintf (ptr_asa_out," Locate_Cost = %d, exited main asa () loop by user-defined OPTIONS\n",Options.Locate_Cost);
		break;
	  default:
		fprintf (ptr_asa_out," Locate_Cost = %d, no index available for Locate_Cost\n",Options.Locate_Cost);
  }

  if (exit_status != INVALID_USER_INPUT && exit_status != INVALID_COST_FUNCTION && exit_status != INVALID_COST_FUNCTION_DERIV) 
  {
    fprintf (ptr_asa_out,"final_cost = best_generated_state->cost = %-*.*g\n", G_FIELD, G_PRECISION, final_cost);
    fprintf (ptr_asa_out, "*number_accepted at best_generated_state->cost = %ld\n",best_number_accepted_saved);
    fprintf (ptr_asa_out, "*number_generated at best_generated_state->cost = %ld\n", best_number_generated_saved);
  }
}


  /* reset OPTIONS->Sequential_Parameters */
 // Options.Sequential_Parameters = start_sequence;


 // if(m_bASA_print)
	//{
	//	fprintf (ptr_asa_out, "\n\n\n");
	//	fflush (ptr_asa_out);
	//	fclose (ptr_asa_out);
	//}
return (0);
}

//Ughhh, this is an ugly hack!

int multi_compare (const void *ii, const void *jj)
{
  int i;
  int j;

  i = *(int *) ii;
  j = *(int *) jj;

  if (ptr->multi_cost_qsort[i] > ptr->multi_cost_qsort[j] + (double) EPS_DOUBLE)
    return (1);
  else if (ptr->multi_cost_qsort[i] < ptr->multi_cost_qsort[j] - (double) EPS_DOUBLE)
    return (-1);
  else
    return (0);
}


//Annealing functions
/***********************************************************************
* accept_new_state
*	This procedure accepts or rejects a newly generated state,
*	depending on whether the difference between new and old
*	cost functions passes a statistical test. If accepted,
*	the current state is updated.
***********************************************************************/

void ASA_Base::accept_new_state ()
{
  double delta_cost, q, prob_test, unif_test, curr_cost_temp, weight_param_ind, weight_aver, range;;
  LONG_INT index_v, active_params;

  /* update accepted and generated count */
  ++number_acceptances_saved;
  ++recent_number_generated;	
  ++m_inumber_generated;
  Options.N_Generated = m_inumber_generated;

  /* increment the parameter index generation for each parameter */
  if (Options.Sequential_Parameters >= 0) 
  {
    if (!PARAMETER_RANGE_TOO_SMALL (Options.Sequential_Parameters))
      ++index_parameter_generations[Options.Sequential_Parameters];
  }
  else 
  {
    for (index_v = 0; index_v < m_inumber_parameters; ++index_v)
      if (!PARAMETER_RANGE_TOO_SMALL (index_v))
        ++index_parameter_generations[index_v];
  }

  /* effective cost function for testing acceptance criteria,
     calculate the cost difference and divide by the temperature */
  curr_cost_temp = current_cost_temperature;
if(user_acceptance_test)
  {
	  if (Options.Cost_Acceptance_Flag == TRUE)
	  {
		if (Options.User_Acceptance_Flag == TRUE) {
		  unif_test = 0.0;
		  Options.User_Acceptance_Flag = FALSE;
		  Options.Cost_Acceptance_Flag = FALSE;
		} else {
		  unif_test = 1.0;
		  Options.Cost_Acceptance_Flag = FALSE;
		}
	  } 
	  else 
	  {
		Options.Acceptance_Test (current_generated_state.cost, m_dparameter_minimum, m_dparameter_maximum,
			m_inumber_parameters, &Options);
		if (Options.User_Acceptance_Flag == TRUE) {
		  unif_test = 0.0;
		  Options.User_Acceptance_Flag = FALSE;
		} else {
		  unif_test = 1.0;
		}
	  }
	  prob_test = Options.Prob_Bias;
}
else /* USER_ACCEPTANCE_TEST */
{
	if(user_cost_schedule)
		  curr_cost_temp =(Options.Cost_Schedule (current_cost_temperature, &Options) + (double) EPS_DOUBLE);
	
	delta_cost = (current_generated_state.cost - last_saved_state.cost)/ (curr_cost_temp + (double) EPS_DOUBLE);

	if(m_buser_accept_asymp_exp)
	{
		  q = Options.Asymp_Exp_Param;
		  if (fabs (1.0 - q) < (double) EPS_DOUBLE)
			prob_test = MIN (1.0, (exp (EXPONENT_CHECK (-delta_cost))));
		  else if ((1.0 - (1.0 - q) * delta_cost) < (double) EPS_DOUBLE)
			prob_test = MIN (1.0, (exp (EXPONENT_CHECK (-delta_cost))));
		  else
			prob_test = MIN (1.0, pow ((1.0 - (1.0 - q) * delta_cost),
										 (1.0 / (1.0 - q))));
	}
	else
	{

		if(m_buser_accept_threshold)       /* USER_ACCEPT_THRESHOLD */
		  prob_test = delta_cost <= 1.0 ? 1.0 : 0.0;
		else
		  prob_test = MIN (1.0, (exp (EXPONENT_CHECK (-delta_cost))));
	}

	unif_test = randflt(&m_iseed);
}

if(m_basa_sample)
{
  weight_aver = active_params = 0;
  
  for (index_v = 0; index_v < m_inumber_parameters; ++index_v)
  {
    /* ignore parameters with too small a range */
    if (PARAMETER_RANGE_TOO_SMALL (index_v))
      continue;
    ++active_params;
    range = m_dparameter_maximum[index_v] - m_dparameter_minimum[index_v];
    weight_param_ind = 2.0 * (fabs ((last_saved_state.parameter[index_v]-current_generated_state.parameter[index_v]) / 
					range)+ current_user_parameter_temp[index_v])* log (1.0 + 1.0 / current_user_parameter_temp[index_v]);
    weight_aver += weight_param_ind;
    Options.Bias_Generated[index_v] = 1.0 / weight_param_ind;
  }
  weight_aver /= (double) active_params;
  Options.Average_Weights = weight_aver;
  
  if (prob_test >= unif_test) 
  {
    Options.Bias_Acceptance = prob_test;
  }
  else 
  {
    Options.Bias_Acceptance = 1.0 - prob_test;
  }

  if(m_bASA_print)
		{
		  if (Options.Limit_Weights < Options.Average_Weights)
		  {
			fprintf (ptr_asa_out, ":SAMPLE#\n");
			if (prob_test >= unif_test) {
			  fprintf (ptr_asa_out, ":SAMPLE+ %10ld %*.*g %*.*g %*.*g %*.*g\n",  Options.N_Accepted,
					   G_FIELD, G_PRECISION, current_generated_state.cost, G_FIELD, G_PRECISION, current_cost_temperature,
					   G_FIELD, G_PRECISION, Options.Bias_Acceptance, G_FIELD, G_PRECISION, Options.Average_Weights);
			  for (index_v = 0; index_v < m_inumber_parameters; ++index_v)
			  {
				/* ignore parameters with too small a range */
				if (PARAMETER_RANGE_TOO_SMALL (index_v))
				  continue;
				range = m_dparameter_maximum[index_v] - m_dparameter_minimum[index_v];
				fprintf (ptr_asa_out,":SAMPLE %11ld %*.*g %*.*g %*.*g %*.*g\n",index_v, G_FIELD, G_PRECISION,
						 current_generated_state.parameter[index_v], G_FIELD,G_PRECISION, current_user_parameter_temp[index_v],
						 G_FIELD, G_PRECISION, Options.Bias_Generated[index_v], G_FIELD, G_PRECISION, range);
			  }
			}
			else 
			{
			  fprintf (ptr_asa_out,":SAMPLE %11ld %*.*g %*.*g %*.*g %*.*g\n", Options.N_Accepted, G_FIELD, G_PRECISION, 
					last_saved_state.cost, G_FIELD, G_PRECISION, current_cost_temperature, G_FIELD, G_PRECISION, Options.Bias_Acceptance,
					   G_FIELD, G_PRECISION, Options.Average_Weights);
		      
			  for (index_v = 0; index_v < m_inumber_parameters; ++index_v)
			  {
				/* ignore parameters with too small a range */
				if (PARAMETER_RANGE_TOO_SMALL (index_v))
				  continue;
				range = m_dparameter_maximum[index_v] - m_dparameter_minimum[index_v];
				fprintf (ptr_asa_out, ":SAMPLE %11ld %*.*g %*.*g %*.*g %*.*g\n", index_v, G_FIELD, G_PRECISION,
						 last_saved_state.parameter[index_v], G_FIELD,G_PRECISION, current_user_parameter_temp[index_v],
						 G_FIELD, G_PRECISION, Options.Bias_Generated[index_v], G_FIELD, G_PRECISION, range);
			  }
			}
		  }
		}
}

  /* accept/reject the new state */
  if (prob_test >= unif_test) {
    /* copy current state to the last saved state */

    last_saved_state.cost = current_generated_state.cost;
    for (index_v = 0; index_v < m_inumber_parameters; ++index_v)
	{
      /* ignore parameters with too small a range */
      if (PARAMETER_RANGE_TOO_SMALL (index_v))
        continue;
      last_saved_state.parameter[index_v] = current_generated_state.parameter[index_v];
    }

    /* update acceptance counts */
    ++recent_number_acceptances;
    ++m_inumber_accepted;
    ++index_cost_acceptances;
    number_acceptances_saved = m_inumber_accepted;
    Options.N_Accepted = m_inumber_accepted;
  }
}


/***********************************************************************
* generate_new_state
*       Generates a valid new state from the old state
***********************************************************************/

void ASA_Base::generate_new_state ()
{
  LONG_INT index_v;
  double x,parameter_v, min_parameter_v, max_parameter_v, temperature_v, parameter_range_v,init_param_temp_v,
		temp_scale_params_v;

  /* generate a new value for each parameter */
   for (index_v = 0; index_v < m_inumber_parameters; ++index_v)
  {
    if (Options.Sequential_Parameters >= -1)
	{
      ++Options.Sequential_Parameters;
      if (Options.Sequential_Parameters == m_inumber_parameters)
        Options.Sequential_Parameters = 0;
      index_v = Options.Sequential_Parameters;
    }
    min_parameter_v = m_dparameter_minimum[index_v];
    max_parameter_v = m_dparameter_maximum[index_v];
    parameter_range_v = max_parameter_v - min_parameter_v;

    /* ignore parameters that have too small a range */
    if (fabs (parameter_range_v) < (double) EPS_DOUBLE)
      continue;

    temperature_v = current_user_parameter_temp[index_v];
	if(user_generating_function)
	{
		init_param_temp_v = initial_user_parameter_temp[index_v];
		temp_scale_params_v = temperature_scale_parameters[index_v];
	}
    parameter_v = last_saved_state.parameter[index_v];

    /* Handle discrete parameters. */

    if (INTEGER_PARAMETER (index_v)) 
	{
        min_parameter_v -= 0.5;
        max_parameter_v += 0.5;
        parameter_range_v = max_parameter_v - min_parameter_v;
      }

    /* generate a new state x within the parameter bounds */
    for (;;) 
	{
		 if(user_generating_function)
			  x = Options.Generating_Distrib (&m_iseed,m_inumber_parameters,index_v,temperature_v,init_param_temp_v,
											   temp_scale_params_v, parameter_v, parameter_range_v, last_saved_state.parameter, &Options);
		 else
			  x = parameter_v + generate_asa_state (temperature_v)* parameter_range_v;

		  /* exit the loop if within its valid parameter range */
		  if (x <= max_parameter_v - (double) EPS_DOUBLE && x >= min_parameter_v + (double) EPS_DOUBLE)
			break;
    }

    /* Handle discrete parameters.
       You might have to check rounding on your machine. */
    if (INTEGER_PARAMETER (index_v))
	{
        if (x < min_parameter_v + 0.5)
          x = min_parameter_v + 0.5 + (double) EPS_DOUBLE;
        if (x > max_parameter_v - 0.5)
          x = max_parameter_v - 0.5 + (double) EPS_DOUBLE;

        if (x + 0.5 > 0.0) 
		{
          x = (double) ((LONG_INT) (x + 0.5));
        } else 
		{
          x = (double) ((LONG_INT) (x - 0.5));
        }
        
		if (x > m_dparameter_maximum[index_v])
          x = m_dparameter_maximum[index_v];
        if (x < m_dparameter_minimum[index_v])
          x = m_dparameter_minimum[index_v];
      }
    /* save the newly generated value */
    current_generated_state.parameter[index_v] = x;

    if (Options.Sequential_Parameters >= 0)
      break;
  }
}


/***********************************************************************
* reanneal
*	Readjust temperatures of generating and acceptance functions
***********************************************************************/

void ASA_Base::reanneal ()
{
  LONG_INT index_v;
  int cost_test;
  double tmp_var_db3, new_temperature, log_new_temperature_ratio, log_init_cur_temp_ratio,temperature_rescale_power,
	cost_best, cost_last, tmp_dbl, tmp_dbl1;

  //double xnumber_parameters[1];

  cost_test = cost_function_test (last_saved_state.cost, last_saved_state.parameter);

  if (Options.Reanneal_Parameters == TRUE) 
  {
    for (index_v = 0; index_v < m_inumber_parameters; ++index_v)
	{
      if (NO_REANNEAL (index_v))
        continue;

      /* use the temp double to prevent overflow */
      tmp_dbl = (double) index_parameter_generations[index_v];

      /* skip parameters with too small range or integer parameters */
      if (Options.Include_Integer_Parameters == TRUE) 
	  {
        if (PARAMETER_RANGE_TOO_SMALL (index_v))
          continue;
      }
	  else 
	  {
        if (PARAMETER_RANGE_TOO_SMALL (index_v) || INTEGER_PARAMETER (index_v))
          continue;
      }

      /* ignore parameters with too small tangents */
      if (fabs (m_dtangents[index_v]) < (double) EPS_DOUBLE)
        continue;

      /* reset the index of parameter generations appropriately */
	if(m_buser_reanneal_parameters)
		  new_temperature =fabs (Options.Reanneal_Params_Function (current_user_parameter_temp[index_v],
						m_dtangents[index_v], maximum_tangent, (void*)(&Options)));
	else
		  new_temperature = fabs (FUNCTION_REANNEAL_PARAMS(current_user_parameter_temp[index_v], m_dtangents[index_v],
											maximum_tangent));

      if (new_temperature < initial_user_parameter_temp[index_v]) 
	  {
        log_init_cur_temp_ratio = fabs (log(((double) EPS_DOUBLE + initial_user_parameter_temp[index_v])/ ((double) EPS_DOUBLE + new_temperature)));

		if(quench_parameters)
		{
			tmp_dbl = (double) EPS_DOUBLE+ pow(log_init_cur_temp_ratio/ temperature_scale_parameters[index_v],
                   m_dxnumber_parameters / Options.User_Quench_Param_Scale[index_v]);
		}
		else
		{
			tmp_dbl = (double) EPS_DOUBLE+ pow(log_init_cur_temp_ratio/ temperature_scale_parameters[index_v],
                   m_dxnumber_parameters);
		}

      } 
	  else 
	  {
        tmp_dbl = 1.0;
      }

      /* Reset index_parameter_generations if index reset too large,
         and also reset the initial_user_parameter_temp, to achieve
         the same new temperature. */
      while (tmp_dbl > ((double) maximum_reanneal_index)) 
	  {
		  if(quench_parameters)
		  {
			log_new_temperature_ratio = -temperature_scale_parameters[index_v] * pow(tmp_dbl,
				Options.User_Quench_Param_Scale[index_v] / m_dxnumber_parameters);
		  }
		  else
		  {
			log_new_temperature_ratio = -temperature_scale_parameters[index_v] * pow(tmp_dbl,1.0 / m_dxnumber_parameters);
		  }
                                                         
        log_new_temperature_ratio = EXPONENT_CHECK (log_new_temperature_ratio);
        new_temperature = initial_user_parameter_temp[index_v] *exp(log_new_temperature_ratio);
        tmp_dbl /= (double) reanneal_scale;

		if(quench_parameters)
		{
			temperature_rescale_power = 1.0 / pow((double) reanneal_scale, Options.User_Quench_Param_Scale[index_v]/ m_dxnumber_parameters);
		}
		else
		{
			temperature_rescale_power = 1.0 / pow((double)reanneal_scale, 1.0/ m_dxnumber_parameters);
		}
		
       initial_user_parameter_temp[index_v] = new_temperature * pow(initial_user_parameter_temp[index_v] / new_temperature,
                                   temperature_rescale_power);
      }
      /* restore from temporary double */
      index_parameter_generations[index_v] = (LONG_INT) tmp_dbl;
    }
  }

  if (Options.Reanneal_Cost == 0) 
    ;
  else if (Options.Reanneal_Cost < -1) 
	  index_cost_acceptances = 1;
  else 
  {
    /* reanneal : Reset the current cost temp and rescale the
       index of cost acceptances. */

    cost_best = best_generated_state.cost;
    cost_last = last_saved_state.cost;
	if(user_reanneal_cost)
	{
		cost_test = Options.Reanneal_Cost_Function (&cost_best, &cost_last, &initial_cost_temperature,&current_cost_temperature,
													 (void*)(&Options));
		tmp_dbl1 = current_cost_temperature;
	}
	else
	{
		cost_test = TRUE;
		if (Options.Reanneal_Cost == 1) 
		{
		  /* (re)set the initial cost_temperature */
		  tmp_dbl = MAX (fabs (cost_last), fabs (cost_best));
		  tmp_dbl = MAX (tmp_dbl, fabs (cost_best - cost_last));
		  tmp_dbl = MAX ((double) EPS_DOUBLE, tmp_dbl);
		  initial_cost_temperature = MIN (initial_cost_temperature, tmp_dbl);
		}

		//Huh?
		tmp_dbl = (double) index_cost_acceptances;

		tmp_dbl1 = MAX (fabs (cost_last - cost_best), current_cost_temperature);
		tmp_dbl1 = MAX ((double) EPS_DOUBLE, tmp_dbl1);
		tmp_dbl1 = MIN (tmp_dbl1, initial_cost_temperature);
	}   
	
	if (cost_test == TRUE && (current_cost_temperature > tmp_dbl1)) 
	{
      tmp_var_db3 = fabs (log(((double) EPS_DOUBLE + initial_cost_temperature) /(tmp_dbl1)));

		  if(m_bquench_cost)
		  {
			  tmp_dbl = (double) EPS_DOUBLE + pow(tmp_var_db3/ temperature_scale_cost, m_dxnumber_parameters/Options.User_Quench_Cost_Scale);
		  }
		  else
		  {
				tmp_dbl = (double) EPS_DOUBLE + pow(tmp_var_db3/ temperature_scale_cost, m_dxnumber_parameters);
		  }
    } 
	else 
	{
      log_init_cur_temp_ratio = fabs (log(((double) EPS_DOUBLE + initial_cost_temperature) /((double) EPS_DOUBLE + current_cost_temperature)));
	  if(m_bquench_cost)
	  {
			tmp_dbl = (double) EPS_DOUBLE + pow(log_init_cur_temp_ratio/ temperature_scale_cost,
				m_dxnumber_parameters/ Options.User_Quench_Cost_Scale);
	  }
	  else
	  {
		  tmp_dbl = (double) EPS_DOUBLE + pow(log_init_cur_temp_ratio/ temperature_scale_cost,m_dxnumber_parameters);
	  }
    }

    /* reset index_cost_temperature if index reset too large */
    while (tmp_dbl > ((double) maximum_reanneal_index))
	{
		if(m_bquench_cost)
		{
			log_new_temperature_ratio = -temperature_scale_cost * pow(tmp_dbl,Options.User_Quench_Cost_Scale/ m_dxnumber_parameters);
		}
		else
		{
			log_new_temperature_ratio = -temperature_scale_cost * pow(tmp_dbl,1.0/ m_dxnumber_parameters);
		}

      log_new_temperature_ratio = EXPONENT_CHECK (log_new_temperature_ratio);
      new_temperature = initial_cost_temperature * exp(log_new_temperature_ratio);
      tmp_dbl /= (double) reanneal_scale;

	  if(m_bquench_cost)
	  {
		  temperature_rescale_power = 1.0 / pow((double) reanneal_scale, Options.User_Quench_Cost_Scale/ m_dxnumber_parameters);
	  }
	  else
	  {
		 temperature_rescale_power = 1.0 / pow((double) reanneal_scale, 1.0/ m_dxnumber_parameters);
	  }
     
	  initial_cost_temperature = new_temperature * pow(initial_cost_temperature/new_temperature, temperature_rescale_power);
    }
    index_cost_acceptances = (LONG_INT) tmp_dbl;
	if(user_acceptance_test)
		 Options.Cost_Temp_Init = initial_cost_temperature;
  }
}

/***********************************************************************
* generate_asa_state
*       This function generates a single value according to the
*       ASA generating function and the passed temperature
***********************************************************************/

double ASA_Base::generate_asa_state (double temp)
{
  double x, y, z;

  x = randflt(&m_iseed);
  y = x < 0.5 ? -1.0 : 1.0;
  z = y * temp * (pow((1.0 + 1.0 / temp), fabs (2.0 * x - 1.0)) - 1.0);

  return (z);
}

/***********************************************************************
* cost_derivatives
*	This procedure calculates the derivatives of the cost function
*	with respect to its parameters.  The first derivatives are
*	used as a sensitivity measure for reannealing.  The second
*	derivatives are calculated only if *curvature_flag=TRUE;
*	these are a measure of the covariance of the fit when a
*	minimum is found.
***********************************************************************/
  /* Calculate the numerical derivatives of the best
     generated state found so far */

  /* In this implementation of ASA, no checks are made for
   *valid_state_generated_flag=FALSE for differential neighbors
   to the current best state. */

  /* Assuming no information is given about the metric of the parameter
     space, use simple Cartesian space to calculate curvatures. */

void ASA_Base::cost_derivatives ()
{
  LONG_INT index_v, index_vv, index_v_vv, index_vv_v, saved_num_invalid_gen_states, tmp_saved;
  double parameter_v, parameter_vv, parameter_v_offset, parameter_vv_offset, recent_best_cost;
  double new_cost_state_1, new_cost_state_2, new_cost_state_3, delta_parameter_v, delta_parameter_vv;
  int immediate_flag;


  if (Options.Curvature_0 == TRUE)
    m_bcurvature_flag  = FALSE;
  if (Options.Curvature_0 == -1)
    m_bcurvature_flag = TRUE;

  /* save Immediate_Exit flag */
  immediate_flag = Options.Immediate_Exit;

  /* save the best cost */
  recent_best_cost = best_generated_state.cost;

  /* copy the best state into the current state */
  for (index_v = 0; index_v < m_inumber_parameters; ++index_v)
  {
    /* ignore parameters with too small ranges */
    if (PARAMETER_RANGE_TOO_SMALL (index_v))
      continue;
    current_generated_state.parameter[index_v] = best_generated_state.parameter[index_v];
  }

  saved_num_invalid_gen_states = m_inumber_invalid_generated_states;

  /* set parameters (& possibly constraints) to best state */
  valid_state_generated_flag = TRUE;

   if(user_acceptance_test)
	{
	  Options.User_Acceptance_Flag = TRUE;
	  Options.Cost_Acceptance_Flag = FALSE;
	}

   current_generated_state.cost = CalcCostFunc(current_generated_state.parameter);

  if (cost_function_test (current_generated_state.cost, current_generated_state.parameter) == 0) 
  {
    exit_status = INVALID_COST_FUNCTION_DERIV;
    return;
  }
  if (valid_state_generated_flag == FALSE)
    ++(m_inumber_invalid_generated_states);

  if (Options.User_Tangents == TRUE) 
  {
    valid_state_generated_flag = FALSE;
	if(user_acceptance_test)
	{
		Options.User_Acceptance_Flag = TRUE;
		Options.Cost_Acceptance_Flag = FALSE;
	}
	current_generated_state.cost = CalcCostFunc(current_generated_state.parameter);
   
	if (cost_function_test (current_generated_state.cost, current_generated_state.parameter) == 0) 
	{
      exit_status = INVALID_COST_FUNCTION_DERIV;
      return;
    }
    
	if (valid_state_generated_flag == FALSE)
      ++(m_inumber_invalid_generated_states);
  } 
  else 
  {
    /* calculate tangents */
    for (index_v = 0; index_v < m_inumber_parameters; ++index_v)
	{
      if (NO_REANNEAL (index_v)) 
	  {
        m_dtangents[index_v] = 0.0;
        continue;
      }
      /* skip parameters with too small range or integer parameters */
      if (Options.Include_Integer_Parameters == TRUE) 
	  {
        if (PARAMETER_RANGE_TOO_SMALL (index_v)) 
		{
          m_dtangents[index_v] = 0.0;
          continue;
        }
      } 
	  else 
	  {
        if (PARAMETER_RANGE_TOO_SMALL (index_v) || INTEGER_PARAMETER (index_v)) 
		{
          m_dtangents[index_v] = 0.0;
          continue;
        }
      }
	  if(delta_parameters)
		delta_parameter_v = Options.User_Delta_Parameter[index_v];
	  else
	      delta_parameter_v = Options.Delta_X;

      if (delta_parameter_v < SMALL_FLOAT) 
	  {
        m_dtangents[index_v] = 0;
        continue;
      }

      /* save the v_th parameter and delta_parameter */
      parameter_v = best_generated_state.parameter[index_v];

      parameter_v_offset = (1.0 + delta_parameter_v) * parameter_v;
      
	  if (parameter_v_offset > m_dparameter_maximum[index_v] || parameter_v_offset < m_dparameter_minimum[index_v]) 
	  {
        delta_parameter_v = -delta_parameter_v;
        parameter_v_offset = (1.0 + delta_parameter_v) * parameter_v;
      }

      /* generate the first sample point */
      current_generated_state.parameter[index_v] = parameter_v_offset;
      valid_state_generated_flag = TRUE;

	  if(user_acceptance_test)
	  {
			  Options.User_Acceptance_Flag = TRUE;
			  Options.Cost_Acceptance_Flag = FALSE;
	  }
      
	  current_generated_state.cost = CalcCostFunc(current_generated_state.parameter);

	  if (cost_function_test(current_generated_state.cost,current_generated_state.parameter) == 0) 
	  {
        exit_status = INVALID_COST_FUNCTION_DERIV;
        return;
      }
      
	  if (valid_state_generated_flag == FALSE)
        ++m_inumber_invalid_generated_states;
      
	  new_cost_state_1 = current_generated_state.cost;

      /* restore the parameter state */
      current_generated_state.parameter[index_v] = parameter_v;

      /* calculate the numerical derivative */
      m_dtangents[index_v] = (new_cost_state_1 - recent_best_cost)/ (delta_parameter_v * parameter_v + (double) EPS_DOUBLE);
    }
  }

  /* find the maximum |tangent| from all tangents */
  maximum_tangent = 0;
  for (index_v = 0; index_v < m_inumber_parameters; ++index_v)
  {
    if (NO_REANNEAL (index_v))
      continue;

    /* ignore too small ranges and integer parameters types */
    if (Options.Include_Integer_Parameters == TRUE) 
	{
      if (PARAMETER_RANGE_TOO_SMALL (index_v))
        continue;
    }
	else
	{
      if (PARAMETER_RANGE_TOO_SMALL (index_v) || INTEGER_PARAMETER (index_v))
        continue;
    }

    /* find the maximum |tangent| (from all tangents) */
    if (fabs (m_dtangents[index_v]) > maximum_tangent) 
	{
      maximum_tangent = fabs (m_dtangents[index_v]);
    }
  }

  if (m_bcurvature_flag == TRUE || m_bcurvature_flag == -1) 
  {
    /* calculate diagonal curvatures */
    for (index_v = 0; index_v < m_inumber_parameters; ++index_v)
	{
      /* index_v_vv: row index_v, column index_v */
      index_v_vv = ROW_COL_INDEX (index_v, index_v);

      if (NO_REANNEAL (index_v)) 
	  {
        m_dcurvature[index_v_vv] = 0.0;
        continue;
      }
      /* skip parameters with too small range or integer parameters */
      if (Options.Include_Integer_Parameters == TRUE) 
	  {
        if (PARAMETER_RANGE_TOO_SMALL (index_v)) 
		{
          m_dcurvature[index_v_vv] = 0.0;
          continue;
        }
      } 
	  else
	  {
        if (PARAMETER_RANGE_TOO_SMALL (index_v) ||  INTEGER_PARAMETER (index_v)) 
		{
          m_dcurvature[index_v_vv] = 0.0;
          continue;
        }
      }
	  
	if(delta_parameters)
      delta_parameter_v = Options.User_Delta_Parameter[index_v];
	else
      delta_parameter_v = Options.Delta_X;

      if (delta_parameter_v < SMALL_FLOAT) 
	  {
        m_dcurvature[index_v_vv] = 0.0;
        continue;
      }

      /* save the v_th parameter and delta_parameter */
      parameter_v = best_generated_state.parameter[index_v];

      if (parameter_v + delta_parameter_v * fabs (parameter_v)> m_dparameter_maximum[index_v]) 
	  {
        /* generate the first sample point */
        current_generated_state.parameter[index_v] = parameter_v - 2.0 * delta_parameter_v * fabs (parameter_v);
        valid_state_generated_flag = TRUE;

		if(user_acceptance_test)
		{
				Options.User_Acceptance_Flag = TRUE;
				Options.Cost_Acceptance_Flag = FALSE;
		}

		current_generated_state.cost = CalcCostFunc(current_generated_state.parameter);
        
		if (cost_function_test (current_generated_state.cost,current_generated_state.parameter) == 0) 
		{
          exit_status = INVALID_COST_FUNCTION_DERIV;
          return;
        }
        if (valid_state_generated_flag == FALSE)
          ++m_inumber_invalid_generated_states;
        
		new_cost_state_1 = current_generated_state.cost;

        /* generate the second sample point */
        current_generated_state.parameter[index_v] = parameter_v - delta_parameter_v * fabs (parameter_v);

        valid_state_generated_flag = TRUE;
		
		if(user_acceptance_test)
		{
				Options.User_Acceptance_Flag = TRUE;
				Options.Cost_Acceptance_Flag = FALSE;
		}
       current_generated_state.cost = CalcCostFunc(current_generated_state.parameter);

        if (cost_function_test (current_generated_state.cost,current_generated_state.parameter) == 0) 
		{
          exit_status = INVALID_COST_FUNCTION_DERIV;
          return;
        }
        if (valid_state_generated_flag == FALSE)
          ++m_inumber_invalid_generated_states;
        new_cost_state_2 = current_generated_state.cost;

        /* restore the parameter state */
        current_generated_state.parameter[index_v] = parameter_v;

        /* calculate and store the curvature */
        m_dcurvature[index_v_vv] =(recent_best_cost - 2.0 * new_cost_state_2 + new_cost_state_1) / (delta_parameter_v * delta_parameter_v
                                  * parameter_v * parameter_v + (double) EPS_DOUBLE);
      }
	  else if (parameter_v - delta_parameter_v * fabs (parameter_v) < m_dparameter_minimum[index_v]) 
	  {
        /* generate the first sample point */
        current_generated_state.parameter[index_v] = parameter_v + 2.0 * delta_parameter_v * fabs (parameter_v);
        valid_state_generated_flag = TRUE;
		if(user_acceptance_test)
		{
				Options.User_Acceptance_Flag = TRUE;
				Options.Cost_Acceptance_Flag = FALSE;
		}
		
		current_generated_state.cost = CalcCostFunc(current_generated_state.parameter);
        
		if (cost_function_test (current_generated_state.cost, current_generated_state.parameter) == 0) 
		{
          exit_status = INVALID_COST_FUNCTION_DERIV;
          return;
        }
        if (valid_state_generated_flag == FALSE)
          ++m_inumber_invalid_generated_states;
        new_cost_state_1 = current_generated_state.cost;

        /* generate the second sample point */
        current_generated_state.parameter[index_v] = parameter_v + delta_parameter_v * fabs (parameter_v);

        valid_state_generated_flag = TRUE;
		if(user_acceptance_test)
		{
			Options.User_Acceptance_Flag = TRUE;
			Options.Cost_Acceptance_Flag = FALSE;
		}
      
		current_generated_state.cost = CalcCostFunc(current_generated_state.parameter);

        if (cost_function_test (current_generated_state.cost,current_generated_state.parameter) == 0) 
		{
          exit_status = INVALID_COST_FUNCTION_DERIV;
          return;
        }
        if (valid_state_generated_flag == FALSE)
          ++m_inumber_invalid_generated_states;
        new_cost_state_2 = current_generated_state.cost;

        /* restore the parameter state */
        current_generated_state.parameter[index_v] = parameter_v;

        /* index_v_vv: row index_v, column index_v */
        index_v_vv = ROW_COL_INDEX (index_v, index_v);

        /* calculate and store the curvature */
        m_dcurvature[index_v_vv] =(recent_best_cost - 2.0 * new_cost_state_2 + new_cost_state_1) / (delta_parameter_v * delta_parameter_v
                                  * parameter_v * parameter_v + (double) EPS_DOUBLE);
      } 
	  else
	  {
        /* generate the first sample point */
        parameter_v_offset = (1.0 + delta_parameter_v) * parameter_v;
        current_generated_state.parameter[index_v] = parameter_v_offset;
        valid_state_generated_flag = TRUE;
		if(user_acceptance_test)
		{	
			Options.User_Acceptance_Flag = TRUE;
			Options.Cost_Acceptance_Flag = FALSE;
		}
      
		current_generated_state.cost = CalcCostFunc(current_generated_state.parameter);

       /* if (cost_function_test (current_generated_state.cost, current_generated_state.parameter) == 0) 
		{
          exit_status = INVALID_COST_FUNCTION_DERIV;
          return;
        }*/

        if (valid_state_generated_flag == FALSE)
          ++m_inumber_invalid_generated_states;
        new_cost_state_1 = current_generated_state.cost;

        /* generate the second sample point */
        current_generated_state.parameter[index_v] = (1.0 - delta_parameter_v) * parameter_v;

        valid_state_generated_flag = TRUE;
		if(user_acceptance_test)
		{
				Options.User_Acceptance_Flag = TRUE;
				Options.Cost_Acceptance_Flag = FALSE;
		}

		current_generated_state.cost = CalcCostFunc(current_generated_state.parameter);


        if (cost_function_test (current_generated_state.cost,current_generated_state.parameter) == 0) 
		{
          exit_status = INVALID_COST_FUNCTION_DERIV;
          return;
        }
        if (valid_state_generated_flag == FALSE)
          ++m_inumber_invalid_generated_states;
        new_cost_state_2 = current_generated_state.cost;

        /* restore the parameter state */
        current_generated_state.parameter[index_v] = parameter_v;

        /* calculate and store the curvature */
        m_dcurvature[index_v_vv] =(new_cost_state_2 - 2.0 * recent_best_cost + new_cost_state_1) / (delta_parameter_v * delta_parameter_v
                                  * parameter_v * parameter_v +(double) EPS_DOUBLE);
      }
    }

    /* calculate off-diagonal curvatures */
    for (index_v = 0; index_v < m_inumber_parameters; ++index_v)
	{
		if(delta_parameters)
			delta_parameter_v = Options.User_Delta_Parameter[index_v];
		else
			delta_parameter_v = Options.Delta_X;

      if (delta_parameter_v < SMALL_FLOAT) {
        for (index_vv = 0; index_vv < m_inumber_parameters; ++index_vv)
		{
          /* index_v_vv: row index_v, column index_vv */
          index_v_vv = ROW_COL_INDEX (index_v, index_vv);
          index_vv_v = ROW_COL_INDEX (index_vv, index_v);
          m_dcurvature[index_vv_v] = m_dcurvature[index_v_vv] = 0.0;
        }
        continue;
      }

      /* save the v_th parameter and delta_x */
      parameter_v = current_generated_state.parameter[index_v];

      for (index_vv = 0; index_vv < m_inumber_parameters; ++index_vv)
	  {
        /* index_v_vv: row index_v, column index_vv */
        index_v_vv = ROW_COL_INDEX (index_v, index_vv);
        index_vv_v = ROW_COL_INDEX (index_vv, index_v);

        if (NO_REANNEAL (index_vv) || NO_REANNEAL (index_v)) 
		{
          m_dcurvature[index_vv_v] = m_dcurvature[index_v_vv] = 0.0;
          continue;
        }
        /* calculate only the upper diagonal */
        if (index_v <= index_vv) 
		{
          continue;
        }
        /* skip parms with too small range or integer parameters */
        if (Options.Include_Integer_Parameters == TRUE) 
		{
          if (PARAMETER_RANGE_TOO_SMALL (index_v) ||PARAMETER_RANGE_TOO_SMALL (index_vv)) 
		  {
            m_dcurvature[index_vv_v] = m_dcurvature[index_v_vv] = 0.0;
            continue;
          }
        } 
		else 
		{
          if (INTEGER_PARAMETER (index_v) || INTEGER_PARAMETER (index_vv) || PARAMETER_RANGE_TOO_SMALL (index_v) ||
              PARAMETER_RANGE_TOO_SMALL (index_vv)) 
		  {
            m_dcurvature[index_vv_v] = m_dcurvature[index_v_vv] = 0.0;
            continue;
          }
        }
		if(delta_parameters)
	        delta_parameter_vv = Options.User_Delta_Parameter[index_vv];
		else
	        delta_parameter_vv = Options.Delta_X;

        if (delta_parameter_vv < SMALL_FLOAT)
		{
          m_dcurvature[index_vv_v] = m_dcurvature[index_v_vv] = 0.0;
          continue;
        }

        /* save the vv_th parameter and delta_parameter */
        parameter_vv = current_generated_state.parameter[index_vv];

        /* generate first sample point */
        parameter_v_offset = current_generated_state.parameter[index_v] = (1.0 + delta_parameter_v) * parameter_v;
        parameter_vv_offset = current_generated_state.parameter[index_vv] =(1.0 + delta_parameter_vv) * parameter_vv;
        
		if (parameter_v_offset > m_dparameter_maximum[index_v] || parameter_v_offset < m_dparameter_minimum[index_v]) 
		{
          delta_parameter_v = -delta_parameter_v;
          current_generated_state.parameter[index_v] =(1.0 + delta_parameter_v) * parameter_v;
        }
        if (parameter_vv_offset > m_dparameter_maximum[index_vv] || parameter_vv_offset < m_dparameter_minimum[index_vv]) 
		{
          delta_parameter_vv = -delta_parameter_vv;
          current_generated_state.parameter[index_vv] = (1.0 + delta_parameter_vv) * parameter_vv;
        }

        valid_state_generated_flag = TRUE;
		if(user_acceptance_test)
		{
			Options.User_Acceptance_Flag = TRUE;
			Options.Cost_Acceptance_Flag = FALSE;
		}       
		
		current_generated_state.cost = CalcCostFunc(current_generated_state.parameter);

        if (cost_function_test (current_generated_state.cost,current_generated_state.parameter) == 0) 
		{
          exit_status = INVALID_COST_FUNCTION_DERIV;
          return;
        }
        if (valid_state_generated_flag == FALSE)
          ++m_inumber_invalid_generated_states;
        new_cost_state_1 = current_generated_state.cost;

        /* restore the v_th parameter */
        current_generated_state.parameter[index_v] = parameter_v;

        /* generate second sample point */
        valid_state_generated_flag = TRUE;
		if(user_acceptance_test)
		{
			Options.User_Acceptance_Flag = TRUE;
			Options.Cost_Acceptance_Flag = FALSE;
		}      
		
		current_generated_state.cost = CalcCostFunc(current_generated_state.parameter);

        if (cost_function_test (current_generated_state.cost,current_generated_state.parameter) == 0) 
		{
          exit_status = INVALID_COST_FUNCTION_DERIV;
          return;
        }
        if (valid_state_generated_flag == FALSE)
          ++m_inumber_invalid_generated_states;
        new_cost_state_2 = current_generated_state.cost;

        /* restore the vv_th parameter */
        current_generated_state.parameter[index_vv] = parameter_vv;

        /* generate third sample point */
        current_generated_state.parameter[index_v] =(1.0 + delta_parameter_v) * parameter_v;
        valid_state_generated_flag = TRUE;

		if(user_acceptance_test)
		{
			Options.User_Acceptance_Flag = TRUE;
			Options.Cost_Acceptance_Flag = FALSE;
		}
      
		current_generated_state.cost = CalcCostFunc(current_generated_state.parameter);

        if (cost_function_test (current_generated_state.cost, current_generated_state.parameter) == 0) 
		{
          exit_status = INVALID_COST_FUNCTION_DERIV;
          return;
        }
        if (valid_state_generated_flag == FALSE)
          ++m_inumber_invalid_generated_states;
        new_cost_state_3 = current_generated_state.cost;

        /* restore the v_th parameter */
        current_generated_state.parameter[index_v] = parameter_v;

        /* calculate and store the curvature */
        m_dcurvature[index_vv_v] = m_dcurvature[index_v_vv] = (new_cost_state_1 - new_cost_state_2 - new_cost_state_3 + recent_best_cost)
				/ (delta_parameter_v * delta_parameter_vv * parameter_v * parameter_vv + (double) EPS_DOUBLE);
      }
    }
  }

  /* restore Immediate_Exit flag */
  Options.Immediate_Exit = immediate_flag;

  /* restore the best cost function value */
  current_generated_state.cost = recent_best_cost;
  if(m_bASA_print)
	{
	  tmp_saved = m_inumber_invalid_generated_states - saved_num_invalid_gen_states;
	  if (tmp_saved > 0)
		fprintf (ptr_asa_out,"Generated %ld invalid states when calculating the derivatives\n", tmp_saved);
	}

  m_inumber_invalid_generated_states = saved_num_invalid_gen_states;

  if(user_acceptance_test)
	{
		Options.User_Acceptance_Flag = TRUE;
		Options.Cost_Acceptance_Flag = FALSE;
	}
 
}




//Printing functions
void ASA_Base::ASA_Output_System()
{
	m_inumber_asa_open++;
	
if(!m_sfile_name.empty())
{
	if(m_bASAOpen == false)
	{
		m_bASAOpen = true;

		if(m_bASA_print)
		{
			if(m_sfile_name == L"STDOUT")
			{
				if(m_bincl_stdout)
					ptr_asa_out = stdout;
			}
			else
			{
				if(m_bASA_recursive)
					ptr_asa_out = _wfopen(m_sfile_name.c_str(), L"a");
				else
					ptr_asa_out = _wfopen(m_sfile_name.c_str(), L"w");
			}
		}
	}

}
else
{
	m_bASA_print = false;
	m_bASA_print_intermed = false;
	m_basa_print_more = false;
	m_bincl_stdout = false;
}

	if(m_bASA_print && m_bASA_recursive == true)
		fprintf (ptr_asa_out, "\n\n\t\t number_asa_open = %d\n",m_inumber_asa_open);
}

void ASA_Base::ASA_File_Close()
{
	m_bASAOpen = false;

	if(m_bASA_print && ptr_asa_out != NULL)
	{
		fprintf (ptr_asa_out, "\n\n\n");
		fflush (ptr_asa_out);
		fclose (ptr_asa_out);
	}
}

/***********************************************************************
* print_state
*	Prints a description of the current state of the system
***********************************************************************/
void ASA_Base::print_state()
{
  LONG_INT index_v, index_vv, index_v_vv;

  fprintf (ptr_asa_out, "\n");

  if (Options.Curvature_0 == TRUE)
    m_bcurvature_flag = FALSE;
  if (Options.Curvature_0 == -1)
    m_bcurvature_flag = TRUE;

  fprintf (ptr_asa_out, "*index_cost_acceptances = %ld, *current_cost_temperature = %*.*g\n", index_cost_acceptances,
           G_FIELD, G_PRECISION, current_cost_temperature);
  fprintf (ptr_asa_out,"*accepted_to_generated_ratio = %*.*g, *number_invalid... = %ld\n", G_FIELD, G_PRECISION, m_daccepted_to_generated_ratio,
           m_inumber_invalid_generated_states);
  fprintf (ptr_asa_out, "*number_generated = %ld, *number_accepted = %ld\n",m_inumber_generated, m_inumber_accepted);
  fprintf (ptr_asa_out, "best...->cost = %*.*g, last...->cost = %*.*g\n", G_FIELD, G_PRECISION, best_generated_state.cost, G_FIELD,
           G_PRECISION, last_saved_state.cost);

  /* Note that tangents will not be calculated until reanneal
     is called, and therefore their listing in the printout only
     is relevant then */
  fprintf (ptr_asa_out, "index_v  best...->parameter current_parameter_temp\ttangent\n");
  
  for (index_v = 0; index_v < m_inumber_parameters; ++index_v)
  {
	if(m_bdropped_parameters)
	{
		if (PARAMETER_RANGE_TOO_SMALL (index_v))
		  continue;
	}
    fprintf (ptr_asa_out,"%ld\t%*.*g\t\t%*.*g\t%*.*g\n",index_v,G_FIELD, G_PRECISION, best_generated_state.parameter[index_v],
             G_FIELD, G_PRECISION, current_user_parameter_temp[index_v], G_FIELD, G_PRECISION, m_dtangents[index_v]);
  }

  if (m_bcurvature_flag == TRUE) 
  {
    /* print curvatures */
    for (index_v = 0; index_v < m_inumber_parameters; ++index_v)
	{
      if (PARAMETER_RANGE_TOO_SMALL (index_v))
        continue;
      fprintf (ptr_asa_out, "\n");

      for (index_vv = 0; index_vv < m_inumber_parameters; ++index_vv)
	  {
        /* only print upper diagonal of matrix */
        if (index_v < index_vv)
          continue;
        if (PARAMETER_RANGE_TOO_SMALL (index_vv))
          continue;

        /* index_v_vv: row index_v, column index_vv */
        index_v_vv = ROW_COL_INDEX (index_v, index_vv);

        if (index_v == index_vv) 
          fprintf (ptr_asa_out, "curvature[%ld][%ld] = %*.*g\n",index_v, index_vv, G_FIELD, G_PRECISION, m_dcurvature[index_v_vv]);
		else
          fprintf (ptr_asa_out,"curvature[%ld][%ld] = %*.*g \t = curvature[%ld][%ld]\n", index_v, index_vv,
                   G_FIELD, G_PRECISION, m_dcurvature[index_v_vv], index_vv, index_v);
      }
    }
  }
  fprintf (ptr_asa_out, "\n");
  fflush (ptr_asa_out);
}

void ASA_Base::print_starting_parameters()
{
	if(m_bASA_print)
		{
		  fprintf (ptr_asa_out, "Initial Random Seed = %ld\n\n", m_iseed);

		  fprintf (ptr_asa_out,"*number_parameters = %ld\n\n", m_inumber_parameters);

		  /* print the min, max, current values, and types of parameters */
		  fprintf (ptr_asa_out, "index_v parameter_minimum parameter_maximum parameter_value parameter_type \n");

		  if(m_bASA_print_intermed)
			{
				for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
					fprintf (ptr_asa_out, " %-8ld %-*.*g \t\t %-*.*g \t %-*.*g %-7d\n",index_v,G_FIELD, G_PRECISION, 
						m_dparameter_minimum[index_v], G_FIELD, G_PRECISION, m_dparameter_maximum[index_v], G_FIELD, G_PRECISION,
						current_generated_state.parameter[index_v],  parameter_type[index_v]);

				fprintf (ptr_asa_out, "\n\n");
			}
		  /* Print out user-defined OPTIONS */

			if(delta_parameters)
			{
			  for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
				  fprintf (ptr_asa_out, "OPTIONS->User_Delta_Parameter[%ld] = %*.*g\n",index_v,G_FIELD, G_PRECISION,
									  Options.User_Delta_Parameter[index_v]);
			  fprintf (ptr_asa_out, "\n");
			}

			if(quench_parameters)
			{
			  for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
				  fprintf (ptr_asa_out,"OPTIONS->User_Quench_Param_Scale[%ld] = %*.*g\n", index_v,
									  G_FIELD, G_PRECISION, Options.User_Quench_Param_Scale[index_v]);
			}
			if(m_bquench_cost)
			  fprintf (ptr_asa_out,"\nOPTIONS->User_Quench_Cost_Scale = %*.*g\n\n", G_FIELD, G_PRECISION, Options.User_Quench_Cost_Scale);

			if(user_initial_parameters_temps)
			{
			  for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
				  fprintf (ptr_asa_out,"OPTIONS->User_Parameter_Temperature[%ld] = %*.*g\n", index_v,
									  G_FIELD, G_PRECISION, initial_user_parameter_temp[index_v]);
			}

			if(ratio_temperature_scales)
			{
			  for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
				  fprintf (ptr_asa_out, "OPTIONS->User_Temperature_Ratio[%ld] = %*.*g\n", index_v,
									  G_FIELD, G_PRECISION,  Options.User_Temperature_Ratio[index_v]);
			}

			if(user_initial_cost_temp)
			  fprintf (ptr_asa_out,"OPTIONS->User_Cost_Temperature[0] = %*.*g\n",G_FIELD, G_PRECISION, initial_cost_temperature);


			  fflush (ptr_asa_out);
		

		if(multi_min)
		{
			  fprintf (ptr_asa_out, "\n");
			  fprintf (ptr_asa_out, "Multi_Number = %d\n", Options.Multi_Number);
			  fprintf (ptr_asa_out, "Multi_Specify = %d\n", Options.Multi_Specify);

			  for (int index_v = 0; index_v < m_inumber_parameters; ++index_v)
				fprintf (ptr_asa_out,"Multi_Grid[%ld] = %*.*g\n", index_v, G_FIELD, G_PRECISION, Options.Multi_Grid[index_v]);
			 
			  fprintf (ptr_asa_out, "\n");
			  fflush (ptr_asa_out);
		}

		if(m_basa_sample)
		{
			  fprintf (ptr_asa_out, "OPTIONS->Limit_Weights = %*.*g\n\n", G_FIELD, G_PRECISION, Options.Limit_Weights);
		}
	}

	
}
/***********************************************************************
* print_asa_Options
*	Prints user's selected Options
***********************************************************************/
void ASA_Base::print_asa_options()
{
	if(ptr_asa_out != NULL)
	{
	  fprintf (ptr_asa_out, "\t\tADAPTIVE SIMULATED ANNEALING\n\n");
	  fprintf (ptr_asa_out, "%s\n\n", ASA_ID);
	  fprintf (ptr_asa_out, "SMALL_FLOAT = %*.*g\n", G_FIELD, G_PRECISION, (double) SMALL_FLOAT);
	  fprintf (ptr_asa_out, "MIN_DOUBLE = %*.*g\n", G_FIELD, G_PRECISION, (double) MIN_DOUBLE);
	  fprintf (ptr_asa_out, "MAX_DOUBLE = %*.*g\n",G_FIELD, G_PRECISION, (double) MAX_DOUBLE);
	  fprintf (ptr_asa_out, "EPS_DOUBLE = %*.*g\n",G_FIELD, G_PRECISION, (double) EPS_DOUBLE);
	  fprintf (ptr_asa_out, "CHECK_EXPONENT = %d\n", (int) CHECK_EXPONENT);
//	  fprintf (ptr_asa_out, "NO_PARAM_TEMP_TEST = %d\n", (int) no_param_temp_test);
//	  fprintf (ptr_asa_out, "NO_COST_TEMP_TEST = %d\n", (int) no_cost_temp_test);
//	  fprintf (ptr_asa_out, "SELF_OPTIMIZE = %d\n", (int) self_optimize);
//	  fprintf (ptr_asa_out, "USER_INITIAL_COST_TEMP = %d\n",(int) user_initial_cost_temp);
//	  fprintf (ptr_asa_out, "RATIO_TEMPERATURE_SCALES = %d\n",(int) ratio_temperature_scales);
//	  fprintf (ptr_asa_out, "USER_INITIAL_PARAMETERS_TEMPS = %d\n",(int) user_initial_parameters_temps);
//	  fprintf (ptr_asa_out, "DELTA_PARAMETERS = %d\n", (int) delta_parameters);
//	  fprintf (ptr_asa_out, "OPTIONAL_DATA_DBL = %d\n", (int) optional_data_dbl);
//	  fprintf (ptr_asa_out, "OPTIONAL_DATA_PTR = %d\n", (int) optional_data_ptr);
//	  fprintf (ptr_asa_out, "USER_COST_SCHEDULE = %d\n",(int) user_cost_schedule);
//	  fprintf (ptr_asa_out, "USER_ACCEPT_ASYMP_EXP = %d\n",(int) user_accept_asymp_exp);
//	  fprintf (ptr_asa_out, "USER_ACCEPT_THRESHOLD = %d\n",(int) user_accept_threshold);
//	  fprintf (ptr_asa_out, "USER_ACCEPTANCE_TEST = %d\n",(int) user_acceptance_test);
//	  fprintf (ptr_asa_out, "USER_GENERATING_FUNCTION = %d\n",(int) user_generating_function);
//	  fprintf (ptr_asa_out, "USER_REANNEAL_COST = %d\n",(int)user_reanneal_cost);
//	  fprintf (ptr_asa_out, "USER_REANNEAL_PARAMETERS = %d\n",(int)user_reanneal_parameters);
//	  fprintf (ptr_asa_out, "REANNEAL_SCALE = %*.*g\n",G_FIELD, G_PRECISION, (double) reanneal_scale);
//	  fprintf (ptr_asa_out, "ASA_SAMPLE = %d\n", (int) asa_sample);
//	  fprintf (ptr_asa_out, "MULTI_MIN = %d\n", (int) multi_min);
	  fprintf (ptr_asa_out, "ASA_PRINT = %d\n", (int) m_bASA_print);
	  fprintf (ptr_asa_out, "ASA_OUT = %s\n", m_sfile_name.c_str());
	  fprintf (ptr_asa_out, "ASA_PRINT_INTERMED = %d\n", (int) m_bASA_print_intermed);
	  fprintf (ptr_asa_out, "ASA_PRINT_MORE = %d\n", (int) m_basa_print_more);
	  fprintf (ptr_asa_out, "INCL_STDOUT = %d\n", (int) m_bincl_stdout);
	  fprintf (ptr_asa_out, "G_FIELD = %d\n", (int) G_FIELD);
	  fprintf (ptr_asa_out, "G_PRECISION = %d\n", (int) G_PRECISION);
	  fprintf (ptr_asa_out, "Options.Limit_Acceptances = %ld\n",(LONG_INT) Options.Limit_Acceptances);
	  fprintf (ptr_asa_out, "Options.Limit_Generated = %ld\n",(LONG_INT) Options.Limit_Generated);
	  fprintf (ptr_asa_out, "Options.Limit_Invalid_Generated_States = %d\n", Options.Limit_Invalid_Generated_States);
	  fprintf (ptr_asa_out, "Options.Accepted_To_Generated_Ratio = %*.*g\n\n", G_FIELD, G_PRECISION, Options.Accepted_To_Generated_Ratio);
	  fprintf (ptr_asa_out, "Options.Cost_Precision = %*.*g\n", G_FIELD, G_PRECISION, Options.Cost_Precision);
	  fprintf (ptr_asa_out, "Options.Maximum_Cost_Repeat = %d\n", Options.Maximum_Cost_Repeat);
	  fprintf (ptr_asa_out, "Options.Number_Cost_Samples = %d\n",Options.Number_Cost_Samples);
	  fprintf (ptr_asa_out, "Options.Temperature_Ratio_Scale = %*.*g\n", G_FIELD, G_PRECISION, Options.Temperature_Ratio_Scale);
	  fprintf (ptr_asa_out, "Options.Cost_Parameter_Scale_Ratio = %*.*g\n",G_FIELD, G_PRECISION, Options.Cost_Parameter_Scale_Ratio);
	  fprintf (ptr_asa_out, "Options.Temperature_Anneal_Scale = %*.*g\n",G_FIELD, G_PRECISION, Options.Temperature_Anneal_Scale);
	  fprintf (ptr_asa_out, "Options.Include_Integer_Parameters = %d\n", Options.Include_Integer_Parameters);
	  fprintf (ptr_asa_out, "Options.User_Initial_Parameters = %d\n",Options.User_Initial_Parameters);
	  fprintf (ptr_asa_out, "Options.Sequential_Parameters = %ld\n", (LONG_INT) Options.Sequential_Parameters);
	  fprintf (ptr_asa_out, "Options.Initial_Parameter_Temperature = %*.*g\n", G_FIELD, G_PRECISION, Options.Initial_Parameter_Temperature);
	  fprintf (ptr_asa_out, "Options.Acceptance_Frequency_Modulus = %d\n", Options.Acceptance_Frequency_Modulus);
	  fprintf (ptr_asa_out, "Options.Generated_Frequency_Modulus = %d\n", Options.Generated_Frequency_Modulus);
	  fprintf (ptr_asa_out, "Options.Reanneal_Cost = %d\n",Options.Reanneal_Cost);
	  fprintf (ptr_asa_out, "Options.Reanneal_Parameters = %d\n\n",Options.Reanneal_Parameters);
	  fprintf (ptr_asa_out, "Options.Delta_X = %*.*g\n", G_FIELD, G_PRECISION, Options.Delta_X);
	  fprintf (ptr_asa_out, "Options.User_Tangents = %d\n",Options.User_Tangents);
	  fprintf (ptr_asa_out, "Options.Curvature_0 = %d\n", Options.Curvature_0);
	  fprintf (ptr_asa_out, "Options.Asa_Recursive_Level = %d\n\n",Options.Asa_Recursive_Level);
	  fprintf (ptr_asa_out, "\n");
	}
}

/***********************************************************************
* print_string
*	This prints the designated string
***********************************************************************/
void ASA_Base::print_string (string line)
{
	if(m_bincl_stdout)
		printf ("\n\n%s\n\n", line.c_str());
	
	if(m_bASA_print)
		fprintf (ptr_asa_out, "\n\n%s\n\n", line.c_str());
}

/***********************************************************************
* print_string_index
*	This prints the designated string and index
***********************************************************************/
void ASA_Base::print_string_index (string line, LONG_INT index)
{
	if(m_bincl_stdout)
		printf ("\n\n%s index = %ld\n\n", line.c_str(), index);

	if(m_bASA_print)
		fprintf (ptr_asa_out, "\n\n%s index = %ld\n\n", line.c_str(), index);
}

void ASA_Base::print_cost_parameters(FILE* pointer)
{
	for (int n_param = 0; n_param < m_inumber_parameters; ++n_param) 
	 {
		 fprintf (pointer, "%ld\t\t%12.7g\n",   n_param, parameter_initial_final[n_param]);
	 }
}

void ASA_Base::print_multi_min_parameters(FILE* pointer)
{
	  fprintf (pointer, "Multi_Specify = %d\n",  Options.Multi_Specify);
      fprintf (pointer, "N_Accepted = %ld\n",  Options.N_Accepted);

	  for (int n_param = 0; n_param < m_inumber_parameters; ++n_param) 
        fprintf (pointer,"Multi_Grid[%ld] = %12.7g\n", n_param, Options.Multi_Grid[n_param]);
      
	  fprintf (pointer, "\n");
      
	  for (int multi_index = 0; multi_index <  Options.Multi_Number;++multi_index) 
	  {
        fprintf (pointer, "\n");
        fprintf (pointer, "Multi_Cost[%d] = %12.7g\n", multi_index, Options.Multi_Cost[multi_index]);
      
		for (int n_param = 0; n_param < m_inumber_parameters; ++n_param) 
          fprintf (pointer, "Multi_Params[%d][%ld] = %12.7g\n",multi_index, n_param, Options.Multi_Params[multi_index][n_param]);
      
	  }
      fprintf (pointer, "\n");
      fflush (pointer);

     
 
	  for (int n_param = 0; n_param < m_inumber_parameters; ++n_param) 
		  parameter_initial_final[n_param] =  Options.Multi_Params[0][n_param];

}
//Test functions

/***********************************************************************
* asa_test_asa_Options
*       Tests user's selected Options
***********************************************************************/

int ASA_Base::asa_test_asa_options ()
{
  int invalid, index_v;
  invalid = 0;

  if (m_iseed == NULL) 
  {
    exit_msg = "*** seed == NULL ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (parameter_initial_final == NULL) 
  {
    exit_msg = "*** parameter_initial_final == NULL ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (m_dparameter_minimum == NULL) 
  {
    exit_msg = "*** parameter_minimum == NULL ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (m_dparameter_maximum == NULL) 
  {
    exit_msg = "*** parameter_maximum == NULL ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (m_dtangents == NULL) 
  {
    exit_msg = "*** tangents == NULL ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (Options.Curvature_0 == FALSE || Options.Curvature_0 == -1) 
  {
    if (m_dcurvature == NULL) 
	{
      exit_msg = "*** curvature == NULL ***";
      print_string (exit_msg);
      ++invalid;
    }
  }
  if (m_inumber_parameters == NULL) {
    exit_msg = "*** m_inumber_parameters == NULL ***";
    print_string (exit_msg);
    ++invalid;
  }
 /* if (parameter_type == NULL) {
    exit_msg = "*** parameter_type == NULL ***";
    print_string (exit_msg);
    ++invalid;
  }*/
  if (valid_state_generated_flag == NULL) {
    exit_msg = "*** valid_state_generated_flag == NULL ***";
    print_string (exit_msg);
    ++invalid;
  }
  for (index_v = 0; index_v < m_inumber_parameters; ++index_v)
	 if (m_dparameter_minimum[index_v] > m_dparameter_maximum[index_v]) 
	 {
		exit_msg = "*** parameter_minimum[] > parameter_maximum[] ***";
		print_string_index (exit_msg, index_v);
		++invalid;
	 }
  for (index_v = 0; index_v < m_inumber_parameters; ++index_v)
    if (parameter_initial_final[index_v] < m_dparameter_minimum[index_v]) 
	{
		if (PARAMETER_RANGE_TOO_SMALL (index_v))
		  continue;
		exit_msg = "*** parameter_initial[] < parameter_minimum[] ***";
		print_string_index (exit_msg, index_v);
		++invalid;
	}
  for (index_v = 0; index_v < m_inumber_parameters; ++index_v)
    if (parameter_initial_final[index_v] > m_dparameter_maximum[index_v]) 
	{
		if (PARAMETER_RANGE_TOO_SMALL (index_v))
		  continue;
		exit_msg = "*** parameter_initial[] > parameter_maximum[] ***";
		print_string_index (exit_msg,index_v);
		++invalid;
	}
  if (m_inumber_parameters < 1)
  {
    exit_msg = "*** *m_inumber_parameters < 1 ***";
    print_string (exit_msg);
    ++invalid;
  }
  /*for (index_v = 0; index_v < m_inumber_parameters; ++index_v)
    if (parameter_type[index_v] != -2 && parameter_type[index_v] != 2&& parameter_type[index_v] != -1 && parameter_type[index_v] != 1) 
	{
		exit_msg,"*** parameter_type[] != -2 && parameter_type[] != 2 && parameter_type[] != -1 && parameter_type[] != 1 ***";
		print_string_index (exit_msg, index_v);
		++invalid;
	}
  if (maximum_reanneal_index < 1) {
    exit_msg = "*** MAXIMUM_REANNEAL_INDEX < 1 ***";
    print_string (exit_msg);
    ++invalid;
  }*/
  //if (reanneal_scale < 0) {
  //  exit_msg = "*** REANNEAL_SCALE < 0.0 ***";
  //  print_string (exit_msg);
  //  ++invalid;
  //}

if(multi_min)
{
  if (Options.Multi_Number <= 0) {
    exit_msg = "*** Options.Multi_Number <= 0 ***";
    print_string (exit_msg);
    ++invalid;
  }
 for (index_v = 0; index_v < m_inumber_parameters; ++index_v)
 {
    if (((Options.Multi_Grid[index_v]) != (Options.Multi_Grid[index_v])) || Options.Multi_Grid[index_v] < 0) 
	{
      exit_msg,"*** (Options.Multi_Grid[]) != (Options.Multi_Grid[]) || Options.Multi_Grid[] < 0 ***";
      print_string_index (exit_msg,index_v);
      ++invalid;
    }
  }
  if (Options.Multi_Specify != 0 && Options.Multi_Specify != 1) 
  {
    exit_msg =  "*** Options.Multi_Specify != 0 && Options.Multi_Specify != 1 ***";
    print_string (exit_msg);
    ++invalid;
  }
}
   if (G_FIELD < 0) {
    exit_msg = "*** G_FIELD < 0 ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (G_PRECISION < 0) {
    exit_msg = "*** G_PRECISION < 0 ***";
    print_string (exit_msg);
    ++invalid;
  }

  if (Options.Limit_Acceptances < 0) {
    exit_msg = "*** Limit_Acceptances < 0 ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (Options.Limit_Generated < 0) {
    exit_msg = "*** Limit_Generated < 0 ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (Options.Limit_Invalid_Generated_States < 0) {
    exit_msg = "*** Limit_Invalid_Generated_States < 0 ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (Options.Accepted_To_Generated_Ratio <= 0.0) {
    exit_msg = "*** Accepted_To_Generated_Ratio <= 0.0 ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (Options.Cost_Precision <= 0.0) {
    exit_msg = "*** Cost_Precision <= 0.0 ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (Options.Maximum_Cost_Repeat < 0) {
    exit_msg = "*** Maximum_Cost_Repeat < 0 ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (Options.Number_Cost_Samples == 0 || Options.Number_Cost_Samples == -1) {
    exit_msg,
            "*** Number_Cost_Samples == 0 || Number_Cost_Samples == -1 ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (Options.Temperature_Ratio_Scale <= 0.0) {
    exit_msg = "*** Temperature_Ratio_Scale <= 0.0 ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (Options.Cost_Parameter_Scale_Ratio <= 0.0) {
    exit_msg = "*** Cost_Parameter_Scale_Ratio <= 0.0 ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (Options.Temperature_Anneal_Scale <= 0.0) {
    exit_msg = "*** Temperature_Anneal_Scale <= 0.0 ***";
    print_string (exit_msg);
    ++invalid;
  }
if(user_initial_cost_temp)
{
  if (Options.User_Cost_Temperature <= 0.0) {
    exit_msg = "*** User_Cost_Temperature[0] <= 0.0 ***";
    print_string (exit_msg);
    ++invalid;
  }
}
  if (Options.Include_Integer_Parameters != FALSE  && Options.Include_Integer_Parameters != TRUE)
  {
    exit_msg = "";
    print_string (exit_msg);
    ++invalid;
  }
  if (Options.User_Initial_Parameters != FALSE && Options.User_Initial_Parameters != TRUE) 
  {
    exit_msg = "*** User_Initial_Parameters != FALSE && User_Initial_Parameters != TRUE ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (Options.Sequential_Parameters >= m_inumber_parameters) 
  {
    exit_msg = "*** Sequential_Parameters >= m_inumber_parameters ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (Options.Initial_Parameter_Temperature <= 0.0) 
  {
    exit_msg = "*** Initial_Parameter_Temperature <= 0.0 ***";
    print_string (exit_msg);
    ++invalid;
  }
	if(ratio_temperature_scales)
	{
	  for (index_v = 0; index_v < m_inumber_parameters; ++index_v)
		  if (Options.User_Temperature_Ratio[index_v] <= 0.0) 
		  {
			exit_msg = "*** User_Temperature_Ratio[] <= 0.0 ***";
			print_string_index (exit_msg,index_v);
			++invalid;
		  }
	}

	if(user_initial_parameters_temps)
	{
	  for (index_v = 0; index_v < m_inumber_parameters; ++index_v)
		  if (Options.User_Parameter_Temperature[index_v] <= 0.0) 
		  {
			exit_msg = "*** User_Parameter_Temperature[] <= 0.0 ***";
			print_string_index (exit_msg,index_v);
			++invalid;
		  }
	}
  if (Options.Acceptance_Frequency_Modulus < 0) {
    exit_msg = "*** Acceptance_Frequency_Modulus < 0 ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (Options.Generated_Frequency_Modulus < 0) {
    exit_msg = "*** Generated_Frequency_Modulus < 0 ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (Options.Reanneal_Cost == -1) {
    exit_msg = "*** Reanneal_Cost == -1 ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (Options.Reanneal_Parameters != FALSE
      && Options.Reanneal_Parameters != TRUE) {
    exit_msg,"*** Reanneal_Parameters != FALSE && Reanneal_Parameters != TRUE ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (Options.Delta_X < 0.0) {
    exit_msg = "*** Delta_X < 0.0 ***";
    print_string (exit_msg);
    ++invalid;
  }
if(delta_parameters)
{
  for (index_v = 0; index_v < m_inumber_parameters; ++index_v)
	  if (Options.User_Delta_Parameter[index_v] < 0.0) 
	  {
		exit_msg = "*** User_Delta_Parameter[] < 0.0 ***";
		print_string_index (exit_msg,index_v);
		++invalid;
	   }
}
  if (Options.User_Tangents != FALSE && Options.User_Tangents != TRUE) {
    exit_msg,
            "*** User_Tangents != FALSE && User_Tangents != TRUE ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (Options.Curvature_0 != -1 && Options.Curvature_0 != FALSE
      && Options.Curvature_0 != TRUE) {
    exit_msg =  "*** Curvature_0 -1 && Curvature_0 != FALSE && Curvature_0 != TRUE ***";
    print_string (exit_msg);
    ++invalid;
  }
if(optional_data_dbl)
{
  if (Options.Asa_Data_Dim_Dbl < 1) {
    exit_msg = "*** Asa_Data_Dim_Dbl < 1 ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (Options.Asa_Data_Dbl == NULL) {
    exit_msg = "*** Asa_Data_Dbl == NULL ***";
    print_string (exit_msg);
    ++invalid;
  }
}

if(optional_data_ptr)
{
  if (Options.Asa_Data_Dim_Ptr < 1) {
    exit_msg = "*** Asa_Data_Dim_Ptr < 1 ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (Options.Asa_Data_Ptr == NULL) {
    exit_msg = "*** Asa_Data_Ptr == NULL ***";
    print_string (exit_msg);
    ++invalid;
  }
}

if(user_cost_schedule)
  if (Options.Cost_Schedule == NULL) {
    exit_msg = "*** Cost_Schedule == NULL ***";
    print_string (exit_msg);
    ++invalid;
  }
if(user_acceptance_test)
{
  if (Options.Acceptance_Test == NULL) {
    exit_msg = "*** Acceptance_Test == NULL ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (Options.User_Acceptance_Flag != FALSE&& Options.User_Acceptance_Flag != TRUE) 
  {
    exit_msg = "*** User_Acceptance_Flag != FALSE && User_Acceptance_Flag != TRUE ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (Options.Cost_Acceptance_Flag != FALSE && Options.Cost_Acceptance_Flag != TRUE)
  {
    exit_msg = "*** Cost_Acceptance_Flag != FALSE && Cost_Acceptance_Flag != TRUE ***";
    print_string (exit_msg);
    ++invalid;
  }
}
if(user_generating_function)
  if (Options.Generating_Distrib == NULL) {
    exit_msg = "*** Generating_Distrib == NULL ***";
    print_string (exit_msg);
    ++invalid;
  }

 if(user_reanneal_cost)
  if (Options.Reanneal_Cost_Function == NULL) {
    exit_msg = "*** Reanneal_Cost_Function == NULL ***";
    print_string (exit_msg);
    ++invalid;
  }
 if(m_buser_reanneal_parameters)
  if (Options.Reanneal_Params_Function == NULL) {
    exit_msg = "*** Reanneal_Params_Function == NULL ***";
    print_string (exit_msg);
    ++invalid;
  }

if(m_basa_sample)
{
  if (Options.Bias_Generated == NULL) {
    exit_msg = "*** Bias_Generated == NULL ***";
    print_string (exit_msg);
    ++invalid;
  }
  if (Options.Limit_Weights < 0.0) {
    exit_msg = "*** Limit_Weights < 0.0 ***";
    print_string (exit_msg);
    ++invalid;
  }
}

  return (invalid);
}

/***********************************************************************
* cost_function_test
*       Tests user's returned cost function values and parameters
***********************************************************************/

int ASA_Base::cost_function_test (double cost, double* parameter)
{
  LONG_INT index_v;
  int test_flag = 1;

  if (((cost) != (cost)) || (cost < -MAX_DOUBLE || cost > MAX_DOUBLE))
    test_flag = 0;

  m_dxnumber_parameters = (double)m_inumber_parameters;

  if(Options.Locate_Cost != 3 && Options.Locate_Cost != 4)
  {
	  for (index_v = 0; index_v < m_inumber_parameters; ++index_v)
	  {
		if (PARAMETER_RANGE_TOO_SMALL (index_v)) 
		{
		  m_dxnumber_parameters -= 1.0;
		  continue;
		}
		if (parameter[index_v] < m_dparameter_minimum[index_v] || parameter[index_v] > m_dparameter_maximum[index_v]) 
		  test_flag = 0;
	  }
  }
  return (test_flag);
}

//Helper methods
double ASA_Base::FUNCTION_REANNEAL_PARAMS(double temperature, double tangent, double max_tangent) 
{
	return(temperature * (max_tangent / tangent));
}

/* IABS(i) absolute value for integers, in stdlib.h on _some_ machines */
bool ASA_Base::IABS(int i)
{
	return ((i) < 0? -(i) : (i));
}

bool ASA_Base::PARAMETER_RANGE_TOO_SMALL(int index)
{
	return (fabs(m_dparameter_minimum[index] - m_dparameter_maximum[index]) < (double) EPS_DOUBLE);
}

//Local fitting

double ASA_Base::fitloc()
{
	LocalFitting locfit(this);

	return locfit.fitloc();
}