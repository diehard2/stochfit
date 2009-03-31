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
#include "ParamVector.h"

ParamVector::ParamVector():m_bfixroughness(false),
m_busesurfabs(false), m_bfiximpnorm(false), m_bXROnly(false), m_binitialized(false), m_isurfabs_index(-1), m_iimpnorm_index(-1),
m_iroughness_index(-1)
{}

void ParamVector::Initialize(const ReflSettings* InitStruct)
{
	m_binitialized = true;
	m_busesurfabs = InitStruct->UseSurfAbs;
	m_bfiximpnorm = InitStruct->Impnorm;
	
	length = m_dparameter_size = InitStruct->Boxes;
	
	if(m_busesurfabs)
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


	data_params.resize(m_dparameter_size);

	gnome.resize(length+2); //Add in 2 layers for superphase and subphase
	
	setImpNorm(1.0);
	setSurfAbs(1.0);

	SetSupphase(InitStruct->SupSLD/InitStruct->FilmSLD);
	SetSubphase(InitStruct->SubSLD/InitStruct->FilmSLD);
	

	SetRoughness(2.0);

	for(int i = 0 ; i < InitStruct->Boxes; i++)
		SetMutatableParameter(i,1.0);

}

int ParamVector::RealparamsSize() const 
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

double ParamVector::GetRealparams(int i) const
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


int ParamVector::SetMutatableParameter(int i,double val)
{
	if(m_binitialized)
	{
		if(i < ParamCount())
		{
			data_params.at(i) = val;
			
			if(i < length)
				gnome.at(i+1) = val;
			
			return 0;
		}
		else 
			return -1;
	}
	else
		return -1;
}

double ParamVector::GetRoughness() const
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

int ParamVector::SetRoughness(double rough)
{
	if(m_binitialized)
	{
		if(m_bfixroughness)
			return -1;
		
		data_params.at(m_iroughness_index) = rough;
		

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
		if(m_bfiximpnorm)
		{
			data_params.at(m_iimpnorm_index) = norm;
		}
		else
			return -1;

		return 0;
	}
	else
		return -1;
}

double ParamVector::GetSurfAbs() const
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
		if(m_busesurfabs)
		{
			data_params.at(m_isurfabs_index) = surfabs;
		}
		else
			return -1;

		return 0;
	}
	else 
		return -1;
}


