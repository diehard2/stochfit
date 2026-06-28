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

#pragma once

// SA parameter bounds and mutation management.
// Maps a box-model electron density profile to a mutable search space of
// double-precision parameters: per-box SLD values, roughness, surface
// absorption, and imperfect-normalization factor. Enforces clamped bounds
// on all mutations. All public API uses double.

#include <span>
#include "platform.h"
#include "SettingsStruct.h"

class ParamVector
{
public:
    explicit ParamVector(const ReflSettings&);

    // EDP grid (supphase, box1..boxN, subphase) — used by CEDP.
    int                     RealParamsSize()    const { return m_boxes + 2; }
    double                  GetRealParams(int i) const;
    std::span<const double> RealParams()         const;

    // Mutable SA search space (per-box SLDs + optional roughness/surfabs/impnorm).
    int    ParamCount()             const { return m_paramCount; }
    int    BoxCount()               const { return m_boxes; }
    double GetMutatableParameter(int i) const;
    void   SetMutatableParameter(int i, double val);  // clamps to bounds

    // Named parameter accessors.
    double GetRoughness() const;
    void   SetRoughness(double rough);     // no-op if roughness is fixed
    double GetImpNorm()   const;
    void   SetImpNorm(double norm);        // no-op if impnorm disabled
    double GetSurfAbs()   const;
    void   SetSurfAbs(double surfabs);     // no-op if surfabs disabled

    // Phase-endpoint values (not part of the SA mutable space).
    void SetSubphase(double subval) { m_edpValues[m_boxes + 1] = subval; }
    void SetSupphase(double supval) { m_edpValues[0]           = supval; }

    // Bounds.
    double GetUpperBounds(int index) const;
    double GetLowerBounds(int index) const;
    void   UpdateBoundaries();

    // Feature flags.
    bool IsRoughnessFixed() const { return m_fixRoughness; }
    bool IsImpNormFixed()   const { return m_fixImpNorm; }
    bool UsesSurfAbs()      const { return m_useSurfAbs; }

private:
    std::vector<double> m_edpValues;     // [supphase, box1..boxN, subphase]
    std::vector<double> m_mutableParams;
    std::vector<double> m_high;
    std::vector<double> m_low;

    int    m_boxes        = 0;
    int    m_paramCount   = 0;
    int    m_roughnessIdx = -1;
    int    m_surfAbsIdx   = -1;
    int    m_impNormIdx   = -1;
    double m_roughness    = 0.0;   // fixed roughness value (when m_fixRoughness)
    double m_roughnessMax = 8.0;
    bool   m_fixRoughness = false;
    bool   m_useSurfAbs   = false;
    bool   m_fixImpNorm   = false;
    bool   m_xrOnly       = false;

    void SetBounds(double lowrough, double highrough, double highimp, double highabs);
};
