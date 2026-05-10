#include "BoxLayerBuild.h"
#include <numbers>
#include <cmath>

LayerStack BoxLayers::View(int boxes) const noexcept {
    LayerStack ls;
    ls.rho          = rho;
    ls.length_mult  = length_mult;
    ls.sigma_sq     = sigma_sq;
    ls.sup_offset   = 0;
    ls.sub_offset   = boxes;   // last film-layer interface (substrate boundary is at boxes+1)
    ls.has_roughness = true;   // box model always applies Nevot-Croce
    // transparent = true when all imag parts are zero (no absorption)
    ls.transparent  = std::ranges::all_of(rho, [](const auto& c){ return c.imag() == 0.0; });
    return ls;
}

void BuildBoxLayers(const BoxReflSettings& rs,
                    std::span<const double> p,
                    bool one_sigma,
                    BoxLayers& out) {
    const int nl = rs.Boxes + 2;
    const double rhofactor = 1e-6 * rs.Wavelength * rs.Wavelength /
                             (2.0 * std::numbers::pi);
    // Factor 2 applied here so rho[i] matches the m_DEDP convention used by
    // ParrattReflectivity (density_profile[i] = 2 * n_i in reduced units).
    const double rho2 = 2.0 * rhofactor;

    out.rho.resize(nl);
    out.length_mult.resize(nl);
    out.sigma_sq.resize(nl - 1);  // one per interface: 0..boxes

    // Superstrate
    out.rho[0]         = rs.SupSLD * rho2;
    out.length_mult[0] = {0.0, 0.0};

    // Film boxes
    for (int i = 1; i <= rs.Boxes; ++i) {
        double sigma;
        if (one_sigma) {
            out.length_mult[i] = {0.0, -2.0 * p[2*(i-1)+1]};
            out.rho[i]         = p[2*(i-1)+2] * rs.SubSLD * rho2;
            sigma              = p[0];
        } else {
            out.length_mult[i] = {0.0, -2.0 * p[3*(i-1)+1]};
            out.rho[i]         = p[3*(i-1)+2] * rs.SubSLD * rho2;
            sigma              = std::fabs(p[3*(i-1)+3]);
            if (sigma < 1e-8) sigma = 1e-8;
        }
        out.sigma_sq[i-1] = -2.0 * sigma * sigma;
    }

    // Substrate
    out.rho[nl-1]         = rs.SubSLD * rho2;
    out.length_mult[nl-1] = {0.0, 0.0};
    // Interface between last film box and substrate uses global roughness p[0]
    out.sigma_sq[rs.Boxes] = -2.0 * p[0] * p[0];

    out.normfactor = p.back();
}
