#pragma once
#include "asa_base.h"

class ASA:public ASA_Base
{
	private:
		int asa_exit();

		double cost_function (double *x, double *parameter_lower_bound, double *parameter_upper_bound, double *cost_tangents,
               double *cost_curvature, LONG_INT  parameter_dimension,  int *parameter_int_real,
               int *cost_flag, int *exit_code, USER_DEFINES * USER_OPTIONS);

		int initialize_parameters (double *cost_parameters, double *parameter_lower_bound,  double *parameter_upper_bound,
                       double *cost_tangents, double *cost_curvature,  LONG_INT  parameter_dimension,
                       int *parameter_int_real, USER_DEFINES * USER_OPTIONS);

	public:
		ASA(string filename, int paramcount);

		//int asa_iteration();
		int funcevals;
		bool m_basa_test_point;
		double asa_loop();
		void Initialize();
		int GetExitCode(){return exit_status;}

};