#include "SettingsBridge.h"
#include <numbers>

ReflSettings ToReflSettings(const BoxReflSettings& rs) {
    ReflSettings out;
    out.Q          = rs.Q;
    out.QError     = rs.QError;
    out.Wavelength = rs.Wavelength;
    // ReflConstants uses: sup_sld = SupSLD; indexsup = 1 - SupSLD/2.
    // The Parratt formula requires SupSLD = 2 * raw_SLD * rhofactor where
    // rhofactor = 1e-6 * lambda^2 / (2*pi). BoxReflSettings stores the raw
    // SLD in 10^-6 Å^-2, so convert here.
    const double rhofactor = 1e-6 * rs.Wavelength * rs.Wavelength /
                             (2.0 * std::numbers::pi);
    out.SupSLD = 2.0 * rs.SupSLD * rhofactor;
    out.QErr   = rs.QSpread;  // both are in percentage units
    return out;
}
