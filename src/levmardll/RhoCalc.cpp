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
#include "RhoCalc.h"
#include "Settings.h"

void RhoCalc::init(const BoxReflSettings& InitStruct)
{
	onesigma  = InitStruct.OneSigma;
	boxnumber = InitStruct.Boxes;
	ZIncrement= InitStruct.ZIncrement;
	Zlength   = InitStruct.ZLength;
	MIRho     = InitStruct.MIEDP;
	SubSLD    = InitStruct.SubSLD;
	m_dSupSLD = InitStruct.SupSLD;

	nk.resize(Zlength);
	nkb.resize(Zlength);

	distarray.resize(boxnumber+1);
	rhoarray.resize(boxnumber+1);
	rougharray.resize(boxnumber+1);

	m_LengthArray.resize(boxnumber);
	m_RhoArray.resize(boxnumber);
	m_SigmaArray.resize(boxnumber);
}

RhoCalc::~RhoCalc() = default;

void RhoCalc::objective(double* par, double* x, int m, int n, void* data)
{
    RhoCalc* rhoinst = (RhoCalc*)data;
    rhoinst->mkdensity(std::span<const double>(par, m));

    for (int i = 0; i < rhoinst->Zlength; ++i)
        x[i] = rhoinst->MIRho[i] - rhoinst->nk[i];
}

void RhoCalc::Rhocalculate(double SubRough, double Zoffset)
{
	const double SuperphaseSLD = m_dSupSLD;
	const double sqrt2 = sqrt(2.0);
	double dist = 0;

	for (int i = 0; i <= boxnumber; i++)
    {
        double deltarho, thick, roughness;
        if (i == 0)
        {
            deltarho  = m_RhoArray[0] * SubSLD - SuperphaseSLD;
            thick     = 0;
            roughness = m_SigmaArray[0];
        }
        else if (i == boxnumber)
        {
            deltarho  = SubSLD - m_RhoArray[i-1] * SubSLD;
            roughness = SubRough;
            thick     = m_LengthArray[i-1];
        }
        else
        {
            deltarho  = (m_RhoArray[i] - m_RhoArray[i-1]) * SubSLD;
            thick     = m_LengthArray[i-1];
            roughness = m_SigmaArray[i];
        }

		dist += thick;
		distarray[i]  = dist;
		rhoarray[i]   = deltarho / 2.0;
		rougharray[i] = roughness * sqrt2;
	}

	#pragma omp parallel for
	for (int j = 0; j < Zlength; j++)
	{
		double summ = SuperphaseSLD;
		for (int i = 0; i <= boxnumber; i++)
			summ += rhoarray[i] * (1.0 + erf((ZIncrement[j] - distarray[i] - Zoffset) / rougharray[i]));

		if (SubRough != 1e-16)
			nk[j] = summ / SubSLD;
		else
			nkb[j] = summ / SubSLD;
	}
}

void RhoCalc::mkdensityboxmodel(std::span<const double> p)
{
	constexpr double SubRough = 1e-16;
	const double ZOffset = p[1];

	if (onesigma)
	{
		for (int i = 0; i < boxnumber; i++)
		{
			m_LengthArray[i] = p[2*i+2];
			m_RhoArray[i]    = p[2*i+3];
			m_SigmaArray[i]  = 1e-16;
		}
	}
	else
	{
		for (int i = 0; i < boxnumber; i++)
		{
			m_LengthArray[i] = p[3*i+2];
			m_RhoArray[i]    = p[3*i+3];
			m_SigmaArray[i]  = 1e-16;
		}
	}
	Rhocalculate(SubRough, ZOffset);
}

void RhoCalc::mkdensity(std::span<const double> p)
{
	const double SubRough = p[0];
	const double ZOffset  = p[1];

	if (onesigma)
	{
		for (int i = 0; i < boxnumber; i++)
		{
			m_LengthArray[i] = p[2*i+2];
			m_RhoArray[i]    = p[2*i+3];
			m_SigmaArray[i]  = p[0];
		}
	}
	else
	{
		for (int i = 0; i < boxnumber; i++)
		{
			m_LengthArray[i] = p[3*i+2];
			m_RhoArray[i]    = p[3*i+3];
			m_SigmaArray[i]  = p[3*i+4];
		}
	}
	Rhocalculate(SubRough, ZOffset);
}
