#include "stdafx.h"
#include "asa_base.h"
#include "asa_class.h"

ASA::ASA(string filename, int paramcount):ASA_Base(filename,paramcount), funcevals(0),m_basa_test_point(false)
{}

void ASA::Initialize()
{

	//Allocate and set all of our variables
	asa_alloc();
	
	//Call the base class's initializer	
	asa_init();

}

int ASA::asa_exit()
{
	ASA_Base::asa_exit();

	if(m_bself_optimize)
	{
		if (Options.Asa_Data_Dbl[0] > (double) MIN_DOUBLE)
			Options.Asa_Data_Dbl[1] = (double) GetBestNumberGeneratedSaved();
	}


	return 0;
}

double ASA::asa_loop()
{

	double cost = ASA_Base::asa_loop();

	
	return cost;
}


double ASA::cost_function (double *x, double *parameter_lower_bound, double *parameter_upper_bound, double *cost_tangents,
               double *cost_curvature, LONG_INT  parameter_dimension,  int *parameter_int_real,
               int *cost_flag, int *exit_code, USER_DEFINES * USER_OPTIONS)
{


  *cost_flag = TRUE;
  return (0);
}


int ASA::initialize_parameters(double *cost_parameters, double *parameter_lower_bound, double *parameter_upper_bound,
		double *cost_tangents, double *cost_curvature, long parameter_dimension, int *parameter_int_real, USER_DEFINES *USER_OPTIONS)
{
	LONG_INT index = 0;
	int multi_index;

  /* store the parameter ranges */
  for (index = 0; index < parameter_dimension; ++index)
    parameter_lower_bound[index] = -10000.0;
  for (index = 0; index < parameter_dimension; ++index)
    parameter_upper_bound[index] = 10000.0;

  /* store the initial parameter types */
  for (index = 0; index < parameter_dimension; ++index)
    parameter_int_real[index] = REAL_TYPE;

  /* store the initial parameter values */
  for (index = 0; index < parameter_dimension / 4.0; ++index) 
  {
    cost_parameters[4 * (index + 1) - 4] = 1;
    cost_parameters[4 * (index + 1) - 3] = -2;
    cost_parameters[4 * (index + 1) - 2] = 1;
    cost_parameters[4 * (index + 1) - 1] = -1;
  }

	if(user_initial_parameters_temps)
	{
		for (index = 0; index < parameter_dimension; ++index)
				USER_OPTIONS->User_Parameter_Temperature[index] = 1.0;
	}
	
	if(user_initial_cost_temp)
	{
	    USER_OPTIONS->User_Cost_Temperature = 5.936648E+09;
	}

	if(delta_parameters)
	{
        for (index = 0; index < parameter_dimension; ++index)
			USER_OPTIONS->User_Delta_Parameter[index] = 0.001;
	}

	if(quench_parameters)
	{
		for (index = 0; index < parameter_dimension; ++index)
			USER_OPTIONS->User_Quench_Param_Scale[index] = 1.0;
	}

	if(m_bquench_cost)
	{
		USER_OPTIONS->User_Quench_Cost_Scale = 0.9;
	}

	if(ratio_temperature_scales)
	{
	    for (index = 0; index < parameter_dimension; ++index)
			USER_OPTIONS->User_Temperature_Ratio[index] = 1.0;
	}
  
	if(multi_min)
	{
		for (index = 0; index < parameter_dimension; ++index) 
			USER_OPTIONS->Multi_Grid[index] = 1e-20;

		#if ASA_TEMPLATE
		  for (index = 0; index < parameter_dimension; ++index)
			USER_OPTIONS->Multi_Grid[index] =(parameter_upper_bound[index] - parameter_lower_bound[index]) / 100.0;
		  }
		  USER_OPTIONS->Multi_Specify = 0;
		#endif /* ASA_TEMPLATE */
	}
	  USER_OPTIONS->Asa_Recursive_Level = 0;

	  return (0);


}