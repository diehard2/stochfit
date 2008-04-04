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

#pragma once
#include "stdafx.h"


  /* The state of the system in terms of parameters and function value */
struct STATE {
    double cost;
    double *parameter;
  };


struct USER_DEFINES {
			LONG_INT Limit_Acceptances;
			LONG_INT Limit_Generated;
			int Limit_Invalid_Generated_States;
			double Accepted_To_Generated_Ratio;
			double Cost_Precision;
			int Maximum_Cost_Repeat;
			int Number_Cost_Samples;
			double Temperature_Ratio_Scale;
			double Cost_Parameter_Scale_Ratio;
			double Temperature_Anneal_Scale;
			double User_Cost_Temperature;
			int Include_Integer_Parameters;
			int User_Initial_Parameters;
			LONG_INT Sequential_Parameters;
			double Initial_Parameter_Temperature;
			double *User_Temperature_Ratio;
			double *User_Parameter_Temperature;
			int Acceptance_Frequency_Modulus;
			int Generated_Frequency_Modulus;
			int Reanneal_Cost;
			int Reanneal_Parameters;
			double Delta_X;
			double *User_Delta_Parameter;
			int User_Tangents;
			int Curvature_0;
			double *User_Quench_Param_Scale;
			double User_Quench_Cost_Scale;
			LONG_INT N_Accepted;
			LONG_INT N_Generated;
			int Locate_Cost;
			int Immediate_Exit;
			double *Best_Cost;
			double *Best_Parameters;
			double *Last_Cost;
			double *Last_Parameters;
			LONG_INT Asa_Data_Dim_Dbl;
			double *Asa_Data_Dbl;
			LONG_INT Asa_Data_Dim_Ptr;
			void *Asa_Data_Ptr;
			
			double Asymp_Exp_Param;
			
			int User_Acceptance_Flag;
			int Cost_Acceptance_Flag;
			double Cost_Temp_Curr;
			double Cost_Temp_Init;
			double Cost_Temp_Scale;
			double Prob_Bias;
			LONG_INT Random_Seed;

			//Function pointer - break into substruct
			void* Asa;
			double (*Generating_Distrib) (LONG_INT*, LONG_INT, long int, double, double, double, double,double, double*, void*);
			int (*Reanneal_Cost_Function) (double*, double*, double*, double*, void*);
			double (*Reanneal_Params_Function) (double, double, double, void*);
			double (*Cost_Schedule) (double, void*);
			void (*Acceptance_Test) (double, double*,double*,LONG_INT, void* );
			double (*Cost_Function)(double *, double *, double *, double *, double *, LONG_INT , 
					int *, int *, int *, void *) ;
			int (*Initialize_Parameters)(double *, double *,  double *,double *, double *,  LONG_INT,
                       int *, void * );

			//Asa Sampling
			double Bias_Acceptance;
			double *Bias_Generated;
			double Average_Weights;
			double Limit_Weights;
			//End Asa Sampling

			//Local Fitting
			int Fit_Local;
			int Iter_Max;
			double Penalty;
			//End Local Fitting

			//Keep track
			int Multi_Number;
			double *Multi_Cost;
			double **Multi_Params;
			double *Multi_Grid;
			int Multi_Specify;
			
			//Keeps track of recursion
			int Asa_Recursive_Level;

		USER_DEFINES()
		{
			Limit_Acceptances = Limit_Generated = 0;
			
			Accepted_To_Generated_Ratio = Cost_Precision = Delta_X = User_Quench_Cost_Scale = 0;
			
			Limit_Invalid_Generated_States = Maximum_Cost_Repeat = Number_Cost_Samples =0;
			Temperature_Ratio_Scale =  Cost_Parameter_Scale_Ratio = Temperature_Anneal_Scale = User_Cost_Temperature = 0;
			
			Include_Integer_Parameters = User_Initial_Parameters = Sequential_Parameters = 0;
			
			Initial_Parameter_Temperature = 0;
				
			Best_Cost = User_Quench_Param_Scale = User_Delta_Parameter = User_Parameter_Temperature = User_Temperature_Ratio = NULL;
			Best_Parameters = Last_Cost = Last_Parameters = Asa_Data_Dbl = Bias_Generated = Multi_Cost = Multi_Grid = NULL;
			Asa_Data_Ptr = NULL;

			Acceptance_Frequency_Modulus =  Generated_Frequency_Modulus = Reanneal_Cost = Reanneal_Parameters = 0;
	
			Asa_Data_Dim_Ptr = User_Tangents =  Curvature_0 = N_Accepted = N_Generated = Asa_Data_Dim_Dbl = 0;
			
			Locate_Cost = Immediate_Exit = 0;

		
		    Asymp_Exp_Param = Cost_Temp_Curr = Cost_Temp_Init = Cost_Temp_Scale = Prob_Bias;
			User_Acceptance_Flag = Cost_Acceptance_Flag = Random_Seed;
		

		    Asa = NULL;
			Generating_Distrib = NULL;
			Reanneal_Cost_Function = NULL;
			Reanneal_Params_Function = NULL;
			Cost_Schedule = NULL;
			Acceptance_Test = NULL;
			Cost_Function = NULL;
			Initialize_Parameters = NULL;

			Bias_Acceptance = Average_Weights = Limit_Weights = 0;

			Fit_Local = Iter_Max = 0;
			Penalty = 0;

			Multi_Number = Multi_Specify;
			Multi_Params = NULL;
	
			Asa_Recursive_Level = 0;
		}
 };



class ASA_Base
{
	private:
		friend class LocalFitting;
	
		bool m_bASAOpen;
		string exit_msg;

		int m_inumber_asa_open;
		int m_irecursive_asa_open;
		FILE *ptr_asa_out;            /* file ptr to output file */
		wstring m_sfile_name;
		bool m_bASA_initialized;		

		int m_inumber_parameters;
		double m_dxnumber_parameters;

		bool m_bcurvature_flag;
		double m_daccepted_to_generated_ratio;
		double maximum_tangent;
		double current_cost_temperature;
		double temperature_scale_cost;
		int m_inumber_invalid_generated_states;
		LONG_INT m_irepeated_invalid_states;
		int m_inumber_generated;
		int m_inumber_accepted;
		int index_cost_repeat;
		LONG_INT number_acceptances_saved;
		LONG_INT recent_number_generated;
		LONG_INT recent_number_acceptances;
		double initial_cost_temperature;
		LONG_INT m_iseed;
		LONG_INT index_cost_acceptances;
		LONG_INT best_number_accepted_saved,best_number_generated_saved;
		
		double *m_dtangents;
		double *m_dcurvature;
		double* current_user_parameter_temp;
		double* parameter_initial_final;
		double* temperature_scale_parameters;
		double temperature_scale;
		double* initial_user_parameter_temp;
		LONG_INT* index_parameter_generations;
		LONG_INT index_exit_v;
		int* parameter_type;
		int valid_state_generated_flag;
		double final_cost;
		LONG_INT start_sequence;
		int best_flag;

		
		

		//Annealing functions
		double generate_asa_state (double temp);
		void cost_derivatives ();
		void reanneal();
		void generate_new_state();
		void accept_new_state();


		//Testing functions
		int cost_function_test (double cost, double* parameter);
		int asa_test_asa_options ();
		
		int exit_exit_status;

		//Printing and file functions

		void ASA_Output_System();
		void ASA_File_Close();
		void print_starting_parameters();
		void print_asa_options();
		void print_state();
		void print_string (string line);
		void print_string_index (string line, LONG_INT index);
		

		//Helper functions
		
		bool PARAMETER_RANGE_TOO_SMALL(int index);
		double FUNCTION_REANNEAL_PARAMS(double temperature, double tangent, double max_tangent) ;
		bool IABS(int i);
		inline int ROW_COL_INDEX(int i, int j){ return(i + m_inumber_parameters * j);}

		//User provided functions in derived class
		

		//Constants - for now
		static const int maximum_reanneal_index = 50000;
		static const int reanneal_scale = 10;

		//Multi_min variables
		  int multi_index;
		  int multi_test, multi_test_cmp, multi_test_dim;
		  int *multi_sort;
		  double *multi_cost;
		  double **multi_params;
	  
		  
	  //static  int multi_compare (const void *ii, const void *jj);

	  int asa_recursive_max;

	  //Temporary variables
	  	double tmp_var_db1, tmp_var_db2,tmp_var_db;
		int tmp_var_int;

	protected:

		//Main functions
		virtual int asa_exit();
		virtual int asa_init();
		virtual double asa_loop();
		virtual int asa_iteration();
		
		double *m_dparameter_minimum;
		double *m_dparameter_maximum;

		STATE best_generated_state;
		STATE last_saved_state;
		STATE current_generated_state;

		int exit_status;

		virtual double cost_function (double *x, double *parameter_lower_bound, double *parameter_upper_bound, double *cost_tangents,
               double *cost_curvature, LONG_INT  parameter_dimension,  int *parameter_int_real,
               int *cost_flag, int *exit_code, USER_DEFINES * USER_OPTIONS) = 0;

		virtual int initialize_parameters (double *cost_parameters, double *parameter_lower_bound,  double *parameter_upper_bound,
                       double *cost_tangents, double *cost_curvature,  LONG_INT  parameter_dimension,
                       int *parameter_int_real, USER_DEFINES * USER_OPTIONS) = 0;

		LONG_INT GetBestNumberGeneratedSaved(){return best_number_generated_saved;}

		void SetNumParameters(int paramcount){ m_inumber_parameters = paramcount;}
		int GetNumberParameters(int paramcount){return m_inumber_parameters;}
	
	public:
		ASA_Base(wstring filename, int paramcount);
		~ASA_Base();

		void asa_alloc();
		void print_cost_parameters(FILE* pointer);
		void print_multi_min_parameters(FILE* pointer);

		//Settings

		bool m_bdropped_parameters;
		bool m_bASA_print;
		bool m_bASA_print_intermed;
		bool m_basa_print_more;
		bool m_bincl_stdout;
		bool m_bASA_recursive;
		bool m_basa_sample;
		bool m_buser_reanneal_parameters;
		bool multi_min;
		bool user_initial_cost_temp;
		bool ratio_temperature_scales;
		bool delta_parameters;
		bool user_initial_parameters_temps;
		bool user_cost_schedule;
		bool optional_data_dbl;
		bool optional_data_ptr;
		bool user_acceptance_test;
		bool user_generating_function;
		bool user_reanneal_cost;
		bool quench_parameters;
		bool m_bquench_cost;
		bool m_bquench_parameters_scale;
		bool m_buser_accept_asymp_exp;
		bool m_buser_accept_threshold;
		bool m_bself_optimize;
		bool m_bquench_cost_scale;
		bool m_bno_param_temp_test;
		bool m_bno_cost_temp_test;
		bool m_blocalfit;
		bool m_buser_param_init;
		bool m_buser_cost_func;

		USER_DEFINES Options;


		double *multi_cost_qsort;
		double CalcCostFunc(double* state);
		void SetInitParamFunction();
		double* GetLastSavedStateParams(){return last_saved_state.parameter;}
		double fitloc();
};

class LocalFitting
{
	ASA_Base* m_pasa;
	private:
		double calcf (double* ansarray);
		int simplex (double* x,double tol1, double tol2, int no_progress,double alpha, double beta1, double beta2,
						  double gamma, double delta);
	public:
		double fitloc ();
		LocalFitting(ASA_Base* asaptr);
};