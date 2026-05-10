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

ParamVector::ParamVector(const ReflSettings& s)
    : m_boxes(s.Boxes),
      m_paramCount(s.Boxes),
      m_roughnessMax(s.RoughnessMax),
      m_useSurfAbs(s.UseSurfAbs),
      m_fixImpNorm(s.Impnorm)
{
    if (m_useSurfAbs) {
        m_surfAbsIdx = m_paramCount++;
    }
    if (m_fixImpNorm) {
        m_impNormIdx = m_paramCount++;
    }
    if (s.Forcesig > 0.0) {
        m_fixRoughness = true;
        m_roughness    = s.Forcesig;
    } else {
        m_roughnessIdx = m_paramCount++;
    }

    m_high.assign(m_paramCount, 1000.0);
    m_low.assign(m_paramCount, -1000.0);
    m_mutableParams.resize(m_paramCount);

    m_edpValues.resize(m_boxes + 2);

    SetImpNorm(1.0);
    SetSurfAbs(1.0);
    SetSupphase(s.SupSLD / s.FilmSLD);
    SetSubphase(s.SubSLD / s.FilmSLD);
    SetRoughness(2.0);

    for (int i = 0; i < m_boxes; i++)
        SetMutatableParameter(i, 1.0);
}

void ParamVector::SetBounds(double lowrough, double highrough, double highimp, double highabs)
{
    std::ranges::fill(m_high, 5.0);
    std::ranges::fill(m_low, -5.0);

    if (!m_fixRoughness) {
        m_high[m_roughnessIdx] = highrough;
        m_low[m_roughnessIdx]  = lowrough;
    }
    if (m_fixImpNorm) {
        m_high[m_impNormIdx] = highimp;
        m_low[m_impNormIdx]  = 0.0;
    }
    if (m_useSurfAbs) {
        m_high[m_surfAbsIdx] = highabs;
        m_low[m_surfAbsIdx]  = 0.0;
    }
}

void ParamVector::UpdateBoundaries()
{
    SetBounds(0.1, m_roughnessMax, 10000.0, 10000.0);
}

double ParamVector::GetRealParams(int i) const
{
    return (i < m_boxes + 2) ? m_edpValues[i] : -1.0;
}

std::span<const double> ParamVector::RealParams() const
{
    return m_edpValues;
}

double ParamVector::GetMutatableParameter(int i) const
{
    return m_mutableParams[i];
}

void ParamVector::SetMutatableParameter(int i, double val)
{
    if (i >= m_paramCount)
        return;

    const double clamped = std::clamp(val, m_low[i], m_high[i]);
    m_mutableParams[i] = clamped;

    if (i < m_boxes)
        m_edpValues[i + 1] = clamped;
}

double ParamVector::GetUpperBounds(int index) const
{
    return (index < m_paramCount) ? m_high[index] : -1.0;
}

double ParamVector::GetLowerBounds(int index) const
{
    return (index < m_paramCount) ? m_low[index] : -1.0;
}

double ParamVector::GetRoughness() const
{
    return m_fixRoughness ? m_roughness : m_mutableParams[m_roughnessIdx];
}

void ParamVector::SetRoughness(double rough)
{
    if (m_fixRoughness)
        return;
    m_mutableParams[m_roughnessIdx] = std::clamp(rough, m_low[m_roughnessIdx], m_high[m_roughnessIdx]);
}

double ParamVector::GetImpNorm() const
{
    return m_fixImpNorm ? m_mutableParams[m_impNormIdx] : 1.0;
}

void ParamVector::SetImpNorm(double norm)
{
    if (!m_fixImpNorm)
        return;
    m_mutableParams[m_impNormIdx] = std::clamp(norm, m_low[m_impNormIdx], m_high[m_impNormIdx]);
}

double ParamVector::GetSurfAbs() const
{
    return m_useSurfAbs ? m_mutableParams[m_surfAbsIdx] : 0.0;
}

void ParamVector::SetSurfAbs(double surfabs)
{
    if (!m_useSurfAbs)
        return;
    m_mutableParams[m_surfAbsIdx] = std::clamp(surfabs, m_low[m_surfAbsIdx], m_high[m_surfAbsIdx]);
}
