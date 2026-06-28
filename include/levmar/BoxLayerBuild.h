#pragma once

#include "Settings.h"
#include "stochfit/LayerStack.h"
#include <complex>
#include <span>
#include <vector>

// Compact representation of a box-model layer stack.
// All rho values are pre-multiplied by 2 to match the m_DEDP convention used
// by ParrattReflectivity (rho[i] = 2 * real_SLD_i in reduced units).
// length_mult[i] = {0, -2*length[i]} — precomputed for the Parratt ak term.
// sigma_sq[i] = -2*sigma[i]^2 — precomputed for the Nevot-Croce factor.
struct BoxLayers {
    std::vector<std::complex<double>> rho;         // size = boxes + 2
    std::vector<std::complex<double>> length_mult; // size = boxes + 2
    std::vector<double>               sigma_sq;    // size = boxes + 1 (per interface)
    double normfactor = 1.0;

    // Returns a LayerStack view over this object.
    // sup_offset=0 (no flat superstrate), sub_offset=boxes (last film interface),
    // has_roughness=true, transparent = (imag part all zero).
    LayerStack View(int boxes) const noexcept;
};

// Builds BoxLayers from a flat parameter vector.
// one_sigma=true: single global roughness p[0]; stride is 2 per box (length, rho_frac).
// one_sigma=false: per-interface roughness; stride is 3 per box (length, rho_frac, sigma).
// In both cases the last parameter p.back() is the normalization factor.
void BuildBoxLayers(const BoxReflSettings& rs,
                    std::span<const double> p,
                    bool one_sigma,
                    BoxLayers& out);
