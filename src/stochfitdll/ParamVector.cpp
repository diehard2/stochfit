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

#include "platform.h"
#include "ParamVector.h"

ParamVector::ParamVector(ReflSettings* InitStruct):m_bfixroughness(false),
m_busesurfabs(false), m_bfiximpnorm(false), m_binitialized(false), m_isurfabs_index(-1), m_iimpnorm_index(-1),
m_iroughness_index(-1)
{
	m_binitialized = true;
	m_busesurfabs = InitStruct->UseSurfAbs;
	m_bfiximpnorm = InitStruct->Impnorm;
	
	length = m_dparameter_size = InitStruct->Boxes;
	
	if(m_busesurfabs == true)
	{
		m_busesurfabs = true;
		m_isurfabs_index = m_dparameter_size;
		m_dparameter_size++;
	}

	if(InitStruct->Impnorm)
	{
		m_bfiximpnorm = true;
		m_iimpnorm_index = m_dparameter_size;
		m_dparameter_size++;
	}

	if(InitStruct->Forcesig > 0.0)
	{
		m_bfixroughness = true;
		roughness = InitStruct->Forcesig;
	}
	else
	{
		m_iroughness_index = m_dparameter_size;
		m_dparameter_size++;
	}

	data_params_low_val.resize(m_dparameter_size);
	data_params_high_val.resize(m_dparameter_size);
	data_params.resize(m_dparameter_size);

	//Give default values to our parameters bounds. These will be updated later
	for(int i = 0; i < m_dparameter_size; i++)
	{
		data_params_high_val.at(i) = 1000;
		data_params_low_val.at(i) = -1000;
	}

    gnome.resize(length+2); //Add in 2 layers for superphase and subphase
	
	setImpNorm(1.0);
	setSurfAbs(1.0);

	SetSupphase(InitStruct->SupSLD/InitStruct->FilmSLD);
	SetSubphase(InitStruct->SubSLD/InitStruct->FilmSLD);
	

	setroughness(2.0);

	for(int i = 0 ; i < InitStruct->Boxes; i++)
		SetMutatableParameter(i,1.0);
}

ParamVector::ParamVector():m_bfixroughness(false),m_busesurfabs(false), m_bfiximpnorm(false), m_binitialized(false)
{
	m_binitialized = false;
}

void ParamVector::SetBounds(double lowrough, double highrough, double highimp, double highabs)
{
	if(m_binitialized)
	{
		//For annealing, we need to provide more reasonable restriction on boundary heights
		for(int i = 0; i < length ; i++)
		{
			data_params_high_val[i] = 5.0;
			data_params_low_val[i] = -5.0;
		}

		if(m_bfixroughness == false)
		{
			data_params_high_val.at(m_iroughness_index) = highrough;
			data_params_low_val.at(m_iroughness_index) = lowrough;
		}

		if(m_bfiximpnorm)
		{
			data_params_high_val.at(m_iimpnorm_index) = highimp;
			data_params_low_val.at(m_iimpnorm_index) = 0.0;
		}

		if(m_busesurfabs)
		{
			data_params_high_val.at(m_isurfabs_index) = highabs;
			data_params_low_val.at(m_isurfabs_index) = 0.0;
		}
	}
}

void ParamVector::UpdateBoundaries(double* high, double* low)
{
	SetBounds(0.1,8.0,10000,10000);

	if(high != NULL && low != NULL)
	{
		for(int i = 0; i < m_dparameter_size; i++)
		{
			high[i] = GetUpperBounds(i);
			low[i] = GetLowerBounds(i);
		}
	}
	else
	{
	}
	
}
int ParamVector::RealparamsSize()
{
	if(m_binitialized)
		return(length+2);
	else
		return 0;
}

int ParamVector::ParamCount()
{
	return m_dparameter_size;
}
 
int ParamVector::GetInitializationLength()
{
	return length;
}

double ParamVector::GetRealparams(int i)
{
	if(m_binitialized)
	{
		if(i < length + 2)
			return gnome.at(i);
		else
			return -1;
	}
	else
		return -1;

}

void ParamVector::SetSubphase(double subval)
{
	if(m_binitialized)
		gnome.at(length+1) = subval;
}

void ParamVector::SetSupphase(double supval)
{
	if(m_binitialized)
		gnome.at(0) = supval;
}

double ParamVector::GetMutatableParameter(int i)
{
	if(m_binitialized)
		return data_params.at(i);
	else
		return -1;
}


int ParamVector::SetMutatableParameter(int i, double val)
{
	if(m_binitialized)
	{
		if(i < ParamCount() && val < GetUpperBounds(i) && val > GetLowerBounds(i))
		{
			data_params.at(i) = val;
			
			if(i < length)
				gnome.at(i+1) = val;
			
			return 0;
		}
		else if( i < ParamCount() && val > GetUpperBounds(i))
		{
			data_params.at(i) = GetUpperBounds(i);
			
			if(i < length)
				gnome.at(i+1) = GetUpperBounds(i);
			
			return 0;
			
		}
		else if( i < ParamCount() && val < GetLowerBounds(i))
		{
			data_params.at(i) = GetLowerBounds(i);
			
			if(i < length)
				gnome.at(i+1) = GetLowerBounds(i);
			
			return 0;
			
		}
		else 
			return -1;
	}
	else
		return -1;
}

double ParamVector::GetUpperBounds(int index)
{
	if(m_binitialized && index < m_dparameter_size)
		return data_params_high_val.at(index);
	else
		return -1;
}

double ParamVector::GetLowerBounds(int index)
{
	if(m_binitialized && index < m_dparameter_size)
		return data_params_low_val.at(index);
	else
		return -1;
}

double ParamVector::getroughness()
{
	if(m_binitialized)
	{
		if(m_bfixroughness)
			return roughness;
		else
			return data_params.at(m_iroughness_index);
	}
	else
		return -1;
}

int ParamVector::setroughness(double rough)
{
	if(m_binitialized)
	{
		if(m_bfixroughness == true)
			return -1;
		
		if(rough > data_params_low_val.at(m_iroughness_index) && rough < data_params_high_val.at(m_iroughness_index))
			data_params.at(m_iroughness_index) = rough;
		else 
			return -1;

		return 0;
	}
	else
		return -1;
}

double ParamVector::getImpNorm()
{
	if(m_binitialized)
	{
		if(m_bfiximpnorm)
			return data_params.at(m_iimpnorm_index);
		else
			return 1.0;
	}
	else
		return -1;
}

int ParamVector::setImpNorm(double norm)
{
	if(m_binitialized)
	{
		if(m_bfiximpnorm == true)
		{
		
			if(norm > data_params_low_val.at(m_iimpnorm_index) && norm < data_params_high_val.at(m_iimpnorm_index))
				data_params.at(m_iimpnorm_index) = norm;
			else 
				return -1;
		}
		else
			return -1;

		return 0;
	}
	else
		return -1;
}

double ParamVector::getSurfAbs()
{
	if(m_binitialized)
	{
		if(m_busesurfabs)
			return data_params.at(m_isurfabs_index);
		else
			return 0;
	}
	else
		return -1;
}

int ParamVector::setSurfAbs(double surfabs)
{
	if(m_binitialized)
	{
		if(m_busesurfabs == true)
		{
		
			if(surfabs > data_params_low_val.at(m_isurfabs_index) && surfabs < data_params_high_val.at(m_isurfabs_index))
				data_params.at(m_isurfabs_index) = surfabs;
			else 
				return -1;
		}
		else
			return -1;

		return 0;
	}
	else 
		return -1;
}


bool ParamVector::CopyArraytoGene(double* myarray)
{
	bool succeeded = true;

	if(m_binitialized)
	{
		for(int i = 0; i < ParamCount(); i++)
		{
			if(SetMutatableParameter(i, myarray[i]) == -1)
				succeeded = false;
		}

		if(succeeded)
			return true;
		else
			return false;
	}
	return false;

}