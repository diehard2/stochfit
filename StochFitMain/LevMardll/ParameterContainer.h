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
#include "stdafx.h"

class ParameterContainer
{
	private:

		int m_iparamlength;
		double m_dscore;
		double m_dparamarray[40];
		double m_dcovararray[40];
		bool m_bonesigma;
		double m_dcutoff;

	public:

		ParameterContainer()
		{
			m_iparamlength = 0;
			m_dscore = 1e6;
			m_dcutoff = -1.0;
		}

		ParameterContainer(double* params, double* covararray, int paramlength, bool onesigma, double score, double cutoff)
		{
			
			m_iparamlength = paramlength;
			m_dscore = score;
			m_bonesigma = onesigma;
			m_dcutoff = cutoff;

			if(m_iparamlength > 0)
			{
				memcpy(m_dparamarray, params, m_iparamlength*sizeof(double));
			
				//Calculate the standard deviations in the parameters
				for(int i = 0; i< paramlength;i++)
				{
					m_dcovararray[i] = sqrt(covararray[i*(paramlength+1)]);
				}
			}
		};

		~ParameterContainer()
		{
			
		};

		//Copy constructor
        ParameterContainer(const ParameterContainer &p)
        {
			m_dscore = p.m_dscore;
			m_iparamlength = p.m_iparamlength;
			m_bonesigma = p.m_bonesigma;
			m_dcutoff = p.m_dcutoff;

			if(p.m_iparamlength > 0)
			{
				memcpy(m_dparamarray, p.m_dparamarray, sizeof(double)*p.m_iparamlength);
				memcpy(m_dcovararray, p.m_dcovararray, sizeof(double)*p.m_iparamlength);
			}
        };

		void SetContainer(double* params, double* covararray, int paramlength, bool onesigma, double score, double cutoff)
		{
			m_iparamlength = paramlength;
			m_dscore = score;
			m_bonesigma = onesigma;
			m_dcutoff = cutoff;

			if(m_iparamlength > 0)
			{
				memcpy(m_dparamarray, params, paramlength*sizeof(double));
				//Calculate the standard deviations in the parameters
				for(int i = 0; i< paramlength;i++)
				{
					m_dcovararray[i] = sqrt(covararray[i*(paramlength+1)]);
				}
			}
		
		};

		int GetLength()
		{
			return m_iparamlength;
		};

		double GetScore()
		{
			return m_dscore;
		};

		double* GetParamArray()
		{
			return m_dparamarray;
		};

		double* GetCovarArray()
		{
			return m_dcovararray;
		};

		bool operator<(const ParameterContainer &param) 
		{ 
			return this->m_dscore < param.m_dscore; 
		};

		ParameterContainer& operator=(const ParameterContainer& param)
		{
			m_dscore = param.m_dscore;
			m_iparamlength = param.m_iparamlength;

			if(m_iparamlength > 0)
			{
				memcpy(m_dparamarray, param.m_dparamarray, m_iparamlength*sizeof(double));
				memcpy(m_dcovararray, param.m_dcovararray, m_iparamlength*sizeof(double));
			}
			
			return *this;
		}

		friend bool operator!=(const ParameterContainer& lhs, const ParameterContainer& rhs)
		{
			double scoreholder = lhs.m_dscore/rhs.m_dscore; 
			if(scoreholder > 1.005 || scoreholder < 0.995)
				return true;
			
			double paramholder; 
			for(int i = 0; i < lhs.m_iparamlength; i++)
			{
				paramholder = fabs(lhs.m_dparamarray[i]/rhs.m_dparamarray[i]);
				if(paramholder > 1.005 || paramholder < 0.995)
					return true;
			}

			return false;
		}

		friend bool operator==(const ParameterContainer& lhs, const ParameterContainer& rhs)
		{

			double scoreholder = lhs.m_dscore/rhs.m_dscore; 
			if(scoreholder > 1.005 || scoreholder < 0.995)
				return false;
			
			double paramholder; 
			for(int i = 0; i < lhs.m_iparamlength; i++)
			{
				paramholder = fabs(lhs.m_dparamarray[i]/rhs.m_dparamarray[i]);
				if(paramholder > 1.005 || paramholder < 0.995)
					return false;
			}

			return true;
		}

		bool IsReasonable()
		{
			if(m_dparamarray != NULL && m_dcovararray != NULL)
			{
				bool isreasonable = true;
				//Discard negative numbers and parameters with large errors
				//Negative roughnesses are allowed
				if(m_bonesigma == true)
				{
					for(int i = 0; i < (m_iparamlength-1)/2; i++)
					{
						if(m_dparamarray[2*i+1] < 0)
						{
							isreasonable = false;
							break;
						}	
						if(m_dparamarray[2*i+2] < 0)
						{
							isreasonable = false;
							break;
						}
					
					}
				#ifndef GIXOS
					for(int i = 0; i < m_iparamlength; i++)
					{
						if((m_dcovararray[i] > m_dcutoff*m_dparamarray[i]) && m_dcutoff > 0)
						{
							isreasonable = false;
							break;
						}
					}
				#endif
					return isreasonable;
				}
				else
				{
					for(int i = 0; i < (m_iparamlength-1)/3; i++)
					{
						if(m_dparamarray[3*i+1] < 0)
						{
							isreasonable = false;
							break;
						}	
						if(m_dparamarray[3*i+2] < 0)
						{
							isreasonable = false;
							break;
						}
					
					}
					#ifndef GIXOS
					for(int i = 0; i < m_iparamlength; i++)
					{
						if((m_dcovararray[i] > m_dcutoff*m_dparamarray[i]) && m_dcutoff > 0)
						{
							isreasonable = false;
							break;
						}
					}
					#endif
					return isreasonable;
				}

			}
			return false;
		}
};