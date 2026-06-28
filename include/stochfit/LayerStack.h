#pragma once

#include <complex>
#include <span>

// A read-only view of an interface stack passed to ParrattReflectivity.
//
// CEDP produces stacks with uniform dz, no roughness (has_roughness=false) and
// sigma_sq empty. The box-model (LevMar) path produces compact N=boxes+2 stacks
// with per-interface sigma values and has_roughness=true.
//
// length_mult[i] = {0, -2·length[i]} — precomputed by the builder so the inner
// Parratt loop pays only one std::exp() call per interface without extra ops.
// sigma_sq[i] = -2·sigma[i]² — precomputed for the same reason; empty span when
// has_roughness is false (Nevot-Croce term is skipped at compile time).
struct LayerStack {
    std::span<const std::complex<double>> rho;          // 2·EDP density per interface
    std::span<const std::complex<double>> length_mult;  // {0, -2·length[i]}
    std::span<const double>               sigma_sq;     // -2·σ[i]²; empty when !has_roughness
    int  sup_offset   = 0;
    int  sub_offset   = 0;
    bool transparent  = true;   // imag(rho) all zero → real-only fast path eligible
    bool has_roughness = false; // any σ != 0 → Nevot-Croce multiplied into Fresnel rj
};
