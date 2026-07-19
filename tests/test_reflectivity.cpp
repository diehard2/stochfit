// Physics regression test for the Parratt reflectivity calculation.
// Verifies ParrattReflectivity::CalculateReflectivity() against reference
// values captured for a 2-box lipid film setup (same parameters as mirefl).
// Any change that corrupts the Parratt recursion or EDP generation will fail.

#include <gtest/gtest.h>
#include <stochfit/UnifiedReflectivity.h>
#include <stochfit/CEDP.h>
#include <stochfit/LayerStack.h>
#include <stochfit/ParamVector.h>
#include <stochfit/StochFitHarness.h>
#include <algorithm>
#include <cmath>
#include <numbers>
#include <complex>
#include <vector>

// Identical to the qrange in src/mirefl/MIRefl.cpp
static const double qrange[] = {
    0.020, 0.026, 0.032, 0.038, 0.044, 0.050, 0.056, 0.062, 0.068, 0.074,
    0.080, 0.086, 0.092, 0.098, 0.104, 0.110, 0.116, 0.122, 0.128, 0.134,
    0.140, 0.146, 0.152, 0.158, 0.164, 0.170, 0.176, 0.182, 0.188, 0.194,
    0.200, 0.206, 0.212, 0.218, 0.224, 0.230, 0.236, 0.242, 0.248, 0.254,
    0.260, 0.266, 0.272, 0.278, 0.284, 0.290, 0.296, 0.302, 0.308, 0.314,
    0.320, 0.326, 0.332, 0.338, 0.344, 0.350, 0.356, 0.362, 0.368, 0.374,
    0.380, 0.386, 0.392, 0.398, 0.404, 0.410, 0.416, 0.422, 0.428, 0.434,
    0.440, 0.446, 0.452, 0.458, 0.464, 0.470, 0.476, 0.482, 0.488, 0.494,
    0.500, 0.506, 0.512, 0.518, 0.524, 0.530, 0.536, 0.542, 0.548, 0.554,
    0.560, 0.566, 0.572, 0.578, 0.584, 0.590, 0.594, 0.596, 0.598, 0.600
};

static void FillInitStruct(ReflSettings& s)
{
    s.Wavelength       = 1.24;
    s.QErr             = 0;
    s.XRonly           = false;
    s.Impnorm          = false;
    s.Q                = std::vector<double>(std::begin(qrange), std::end(qrange));
    s.CritEdgeOffset   = 0;
    s.HighQOffset      = 0;
    s.UseSurfAbs       = false;
    s.SupAbs           = 0;
    s.SubAbs           = 0;
    s.FilmAbs          = 0;
    s.Boxes            = 2;
    s.FilmLength       = 26;
    s.Resolution       = 10;
    s.FilmSLD          = 9.38;
    s.SupSLD           = 0.0;
    s.SubSLD           = 9.38;
    s.Forcesig         = 0;
    s.Objectivefunction = 0;
    // Refl, ReflError, QError left empty — only CalculateReflectivity is called
}

// Relative-tolerance check. At a thin-film interference minimum R can be
// many orders of magnitude smaller than neighbouring points, so pure
// absolute tolerance is unsuitable here.
static void ExpectRelative(double actual, double expected,
                           double reltol = 1e-3,
                           const char* label = "")
{
    double tol = std::fabs(expected) * reltol + 1e-15;
    EXPECT_NEAR(actual, expected, tol) << label;
}

// Reference values captured from the Parratt recursion for this setup.
// Q < Qc uses the complex path; Q > Qc uses the real-kk fast path.
TEST(Reflectivity, TwoBoxLipidFilmParratt)
{
    const double FilmSLD  = 9.38;
    const double SLD[]    = {0.0, 8.911, 13.5072, 9.38};  // sup, box1, box2, sub

    ReflSettings init = {};
    FillInitStruct(init);

    CEDP edp;
    edp.Init(init);

    ParamVector params(init);
    for (int i = 0; i < init.Boxes; i++)
        params.SetMutatableParameter(i, SLD[i + 1] / FilmSLD);
    params.SetRoughness(3.15);

    edp.GenerateEDP(params);

    ParrattReflectivity parratt(init);
    auto reflOut = parratt.CalculateReflectivity(edp);

    ASSERT_EQ(reflOut.size(), 100u);

    // ── Q < Qc region (complex Parratt path) ──────────────────────────────
    // Q=0.020  total external reflection (below critical edge)
    ExpectRelative(reflOut[0],  1.0,          1e-6, "Q=0.020");
    // Q=0.026  just above critical Q, sharp drop
    ExpectRelative(reflOut[1],  9.57934e-02,  1e-3, "Q=0.026");

    // ── Q > Qc region (real Parratt path) ─────────────────────────────────
    // Q=0.032  continuing fall
    ExpectRelative(reflOut[2],  2.96789e-02,  1e-3, "Q=0.032");
    // Q=0.038
    ExpectRelative(reflOut[3],  1.36002e-02,  1e-3, "Q=0.038");
    // Q=0.050  Kiessig fringe region
    ExpectRelative(reflOut[5],  4.56059e-03,  1e-3, "Q=0.050");
    // Q=0.080
    ExpectRelative(reflOut[10], 7.70483e-04,  1e-3, "Q=0.080");
    // Q=0.140  rapid fall-off
    ExpectRelative(reflOut[20], 3.53317e-05,  1e-3, "Q=0.140");
    // Q=0.188  near thin-film interference minimum — use looser tolerance
    ExpectRelative(reflOut[28], 6.14088e-08,  2e-2, "Q=0.188");
    // Q=0.200  recovering after minimum
    ExpectRelative(reflOut[30], 2.13742e-07,  1e-3, "Q=0.200");
    // Q=0.260  second fringe maximum
    ExpectRelative(reflOut[40], 2.61797e-06,  1e-3, "Q=0.260");
    // Q=0.320
    ExpectRelative(reflOut[50], 9.57848e-07,  1e-3, "Q=0.320");
    // Q=0.440
    ExpectRelative(reflOut[70], 1.04542e-07,  1e-3, "Q=0.440");
    // Q=0.560  high-Q tail
    ExpectRelative(reflOut[90], 4.49987e-10,  2e-2, "Q=0.560");
    // Q=0.600
    ExpectRelative(reflOut[99], 1.19181e-09,  2e-2, "Q=0.600");
}

// ── Box model Parratt tests ───────────────────────────────────────────────────
//
// These tests drive ParrattReflectivity through a manually constructed LayerStack
// (the same path used by LevMar / BoxLayerBuild) and check known analytical values.
//
// Density convention: density[i] = SLD_i * 2 * rhofactor
//   where rhofactor = 1e-6 * lambda^2 / (2*pi)
// k_i = k0 * sqrt(sin^2(theta) - density[i] + sup_sld)
// For air superstrate (SLD_sup=0): sup_sld=0, k_air = Q/2 (exact).
//
// Reference values are derived analytically from the Fresnel/Parratt formulae.

static void MakeBoxReflSettings(ReflSettings &s, double wavelength,
                                 const std::vector<double> &Q)
{
    s = {};
    s.Wavelength = wavelength;
    s.SupSLD     = 0.0;  // air (Parratt units: 2*raw*rhofactor = 0 for air)
    s.Q          = Q;
    s.QErr       = 0;
}

// Single-box film where SLD_film = SLD_sub (degenerate: film has same density as
// substrate, so the only interface is air/substrate). Expected reflectivity equals
// the bare Fresnel: R = ((k_air - k_sub)/(k_air + k_sub))^2.
//
// Analytical:  k_air = Q/2 = 0.07,  k_sub = sqrt((Q/2)^2 - 4*pi*SubSLD*1e-6)
//              SubSLD = 9.38e-6, Q = 0.14 -> k_sub = 0.069153
//              r = 0.000847/0.139153 = 6.087e-3  ->  R = 3.703e-5
TEST(Reflectivity, BoxModelNoFilm)
{
    const double SubSLD    = 9.38;
    const double lambda    = 1.24;
    const double rhofactor = 1e-6 * lambda * lambda / (2.0 * std::numbers::pi);
    const double rho2      = 2.0 * rhofactor;

    ReflSettings s;
    MakeBoxReflSettings(s, lambda, {0.14});

    std::vector<std::complex<double>> rho = {
        {0.0,             0.0},   // superstrate (air)
        {SubSLD * rho2,   0.0},   // film = Si (no contrast → r12 = 0)
        {SubSLD * rho2,   0.0},   // substrate (Si)
    };
    std::vector<std::complex<double>> lm = {
        {0.0, 0.0},               // superstrate (zero length)
        {0.0, -2.0 * 200.0},      // film 200 Å (irrelevant since r12=0)
        {0.0, 0.0},               // substrate
    };

    LayerStack ls{};
    ls.rho          = rho;
    ls.length_mult  = lm;
    ls.sup_offset   = 0;
    ls.sub_offset   = 1;
    ls.has_roughness = false;
    ls.transparent  = true;

    ParrattReflectivity parratt(s);
    auto refl = parratt.CalculateReflectivity(ls);
    ASSERT_EQ(refl.size(), 1u);
    ExpectRelative(refl[0], 3.703e-5, 5e-3, "BoxModel-no-film Q=0.14");
}

// Single-box film with SLD_film = SubSLD/2 = 4.69, thickness 200 Å, no roughness.
// At Q=0.14 the propagation phase is ≈ 27.83 rad (close to a reflectivity minimum):
// r01 ≈ 0.003023, r12 ≈ 0.003063; the phase modulation gives R ≈ 1.77e-6.
// Also checks Q=0.016 (below Qc_Si ≈ 0.0217) where R must be ≈ 1.
TEST(Reflectivity, BoxModelSingleFilm)
{
    const double SubSLD  = 9.38;
    const double filmSLD = SubSLD / 2.0;   // 4.69
    const double d       = 200.0;
    const double lambda  = 1.24;
    const double rhofactor = 1e-6 * lambda * lambda / (2.0 * std::numbers::pi);
    const double rho2    = 2.0 * rhofactor;

    // -- Q = 0.016 (below Qc_Si ≈ 0.0217): total reflection ----------------------
    {
        ReflSettings s;
        MakeBoxReflSettings(s, lambda, {0.016});

        std::vector<std::complex<double>> rho = {
            {0.0, 0.0}, {filmSLD*rho2, 0.0}, {SubSLD*rho2, 0.0}
        };
        std::vector<std::complex<double>> lm = {
            {0.0, 0.0}, {0.0, -2.0*d}, {0.0, 0.0}
        };
        LayerStack ls{};
        ls.rho = rho; ls.length_mult = lm;
        ls.sup_offset = 0; ls.sub_offset = 1;
        ls.has_roughness = false; ls.transparent = false;

        ParrattReflectivity parratt(s);
        auto refl = parratt.CalculateReflectivity(ls);
        ASSERT_EQ(refl.size(), 1u);
        // k_sub is imaginary (Q < Qc) → total reflection
        EXPECT_NEAR(refl[0], 1.0, 1e-6) << "BoxModel-total-reflection Q=0.016";
    }

    // -- Q = 0.14 (above all critical edges): interference -----------------------
    {
        ReflSettings s;
        MakeBoxReflSettings(s, lambda, {0.14});

        std::vector<std::complex<double>> rho = {
            {0.0, 0.0}, {filmSLD*rho2, 0.0}, {SubSLD*rho2, 0.0}
        };
        std::vector<std::complex<double>> lm = {
            {0.0, 0.0}, {0.0, -2.0*d}, {0.0, 0.0}
        };
        LayerStack ls{};
        ls.rho = rho; ls.length_mult = lm;
        ls.sup_offset = 0; ls.sub_offset = 1;
        ls.has_roughness = false; ls.transparent = true;

        ParrattReflectivity parratt(s);
        auto refl = parratt.CalculateReflectivity(ls);
        ASSERT_EQ(refl.size(), 1u);
        // Near a reflectivity minimum; analytical value ≈ 1.77e-6 (loose tol)
        ExpectRelative(refl[0], 1.77e-6, 5e-2, "BoxModel-single-film Q=0.14");
        // Must be well below bare-Fresnel (3.7e-5) due to destructive interference
        EXPECT_LT(refl[0], 1e-5) << "BoxModel-single-film Q=0.14 should be below bare Fresnel";
    }
}

// ── Harness fit-window test ───────────────────────────────────────────────────
//
// With nonzero CritEdgeOffset/HighQOffset the harness must compute the model on
// the same sliced Q window as the measured data. Regression test for a bug where
// the Parratt grid was built from the full Q array while the SA buffer was sized
// to the window: the full-grid result overflowed the buffer and the model was
// misaligned with the data by CritEdgeOffset points.
TEST(Harness, QOffsetsSliceFitWindow)
{
    constexpr int kLowOff  = 5;
    constexpr int kHighOff = 5;
    constexpr int kFull    = 100;
    constexpr int kWindow  = kFull - kLowOff - kHighOff;

    ReflSettings init = {};
    FillInitStruct(init);

    // Reference model on the sliced Q grid at the harness's initial parameter
    // values (ParamVector's constructor defaults: box SLDs = FilmSLD,
    // roughness = 2.0). This mirrors what InitEnergy computes inside StochFit.
    ReflSettings sliced = init;
    sliced.Q.assign(init.Q.begin() + kLowOff, init.Q.end() - kHighOff);

    CEDP edp;
    edp.Init(sliced);
    ParamVector params(sliced);
    params.UpdateBoundaries();
    edp.GenerateEDP(params);
    ParrattReflectivity parratt(sliced, edp.GetLayerCount());
    auto model = parratt.CalculateReflectivity(edp);
    ASSERT_EQ(model.size(), static_cast<size_t>(kWindow));

    // "Measured" data: the exact model inside the window, garbage outside it.
    // If the harness reads any point outside the window, the energy is nonzero.
    init.Refl.assign(kFull, 12345.0);
    init.ReflError.assign(kFull, 1.0);
    std::copy(model.begin(), model.end(), init.Refl.begin() + kLowOff);
    init.CritEdgeOffset = kLowOff;
    init.HighQOffset    = kHighOff;

    StochFit fit(init);
    ASSERT_TRUE(fit.GetInitError().has_value()) << fit.GetInitError().error();

    // Model at initial params reproduces the measured window bit-for-bit, so
    // the initial (lowest) energy must be exactly zero. A misaligned or
    // full-grid model makes this far from zero.
    EXPECT_NEAR(fit.GetLowestEnergy(), 0.0, 1e-12);

    const DataSnapshot snap = fit.GetData();
    ASSERT_EQ(snap.Q.size(), static_cast<size_t>(kWindow));
    EXPECT_DOUBLE_EQ(snap.Q.front(), qrange[kLowOff]);
    EXPECT_DOUBLE_EQ(snap.Q.back(),  qrange[kFull - kHighOff - 1]);
    EXPECT_EQ(snap.refl.size(), static_cast<size_t>(kWindow));
}
