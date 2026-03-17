// Physics regression test for the Parratt reflectivity calculation.
// Verifies CReflCalc::CalculateReflectivity() against reference values
// captured from the current build for the same 2-box lipid film setup
// used by mirefl.  Any change that corrupts the Parratt recursion or the
// EDP generation will cause a failure here.

#include <gtest/gtest.h>
#include <stochfit/stochfitdll/ReflCalc.h> 
#include <cmath>

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

static void FillInitStruct(ReflSettings* s)
{
    s->Wavelength       = 1.24;
    s->Forcenorm        = false;
    s->QErr             = 0;
    s->XRonly           = false;
    s->Impnorm          = false;
    s->Q                = const_cast<double*>(qrange);
    s->CritEdgeOffset   = 0;
    s->HighQOffset      = 0;
    s->UseSurfAbs       = false;
    s->SupAbs           = 0;
    s->SubAbs           = 0;
    s->FilmAbs          = 0;
    s->Boxes            = 2;
    s->FilmLength       = 26;
    s->Resolution       = 10;
    s->FilmSLD          = 9.38;
    s->SupSLD           = 0.0;
    s->SubSLD           = 9.38;
    s->QPoints          = static_cast<int>(sizeof qrange / sizeof qrange[0]);
    s->Forcesig         = 0;
    s->Objectivefunction = 0;
    s->QError           = nullptr;
    s->Refl             = nullptr;
    s->ReflError        = nullptr;
}

// Relative-tolerance check. At a thin-film interference minimum R can be
// many orders of magnitude smaller than neighbouring points, so pure
// absolute tolerance is unsuitable here.
static void ExpectRelative(double actual, double expected,
                           double reltol = 1e-3,
                           const char* label = "")
{
    // absolute tolerance floor prevents division by zero for values near 0
    double tol = std::fabs(expected) * reltol + 1e-15;
    EXPECT_NEAR(actual, expected, tol) << label;
}

// Reference values captured from refl.txt (ParamsRF at the exact data
// Q-points) from the build that passes the complex-arithmetic unit tests.
// Both ParamsRF and CalculateReflectivity call MyTransparentRF with the
// same EDP; at a shared Q-point the arithmetic path (complex / real) is
// identical, so the values agree to floating-point precision.
TEST(Reflectivity, TwoBoxLipidFilmParratt)
{
    CEDP        edp;
    CReflCalc   refl;
    ReflSettings init = {};

    const double FilmSLD  = 9.38;
    const double SLD[]    = {0.0, 8.911, 13.5072, 9.38};  // sup, box1, box2, sub

    FillInitStruct(&init);

    ParamVector params(&init);
    for (int i = 0; i < init.Boxes; i++)
        params.SetMutatableParameter(i, SLD[i + 1] / FilmSLD);
    params.setroughness(3.15);

    refl.Init(&init);
    edp.Init(&init);
    edp.GenerateEDP(&params);
    refl.CalculateReflectivity(&edp);

    ASSERT_EQ(refl.m_idatapoints, 100);

    // ── Q < Qc region (complex Parratt path) ──────────────────────────────
    // Q=0.020  total external reflection (below critical edge)
    ExpectRelative(refl.reflpt[0],  1.0,          1e-6, "Q=0.020");
    // Q=0.026  just above critical Q, sharp drop
    ExpectRelative(refl.reflpt[1],  9.57934e-02,  1e-3, "Q=0.026");

    // ── Q > Qc region (real Parratt path) ─────────────────────────────────
    // Q=0.032  continuing fall
    ExpectRelative(refl.reflpt[2],  2.96789e-02,  1e-3, "Q=0.032");
    // Q=0.038
    ExpectRelative(refl.reflpt[3],  1.36002e-02,  1e-3, "Q=0.038");
    // Q=0.050  Kiessig fringe region
    ExpectRelative(refl.reflpt[5],  4.56059e-03,  1e-3, "Q=0.050");
    // Q=0.080
    ExpectRelative(refl.reflpt[10], 7.70483e-04,  1e-3, "Q=0.080");
    // Q=0.140  rapid fall-off
    ExpectRelative(refl.reflpt[20], 3.53317e-05,  1e-3, "Q=0.140");
    // Q=0.188  near thin-film interference minimum — use looser tolerance
    ExpectRelative(refl.reflpt[28], 6.14088e-08,  2e-2, "Q=0.188");
    // Q=0.200  recovering after minimum
    ExpectRelative(refl.reflpt[30], 2.13742e-07,  1e-3, "Q=0.200");
    // Q=0.260  second fringe maximum
    ExpectRelative(refl.reflpt[40], 2.61797e-06,  1e-3, "Q=0.260");
    // Q=0.320
    ExpectRelative(refl.reflpt[50], 9.57848e-07,  1e-3, "Q=0.320");
    // Q=0.440
    ExpectRelative(refl.reflpt[70], 1.04542e-07,  1e-3, "Q=0.440");
    // Q=0.560  high-Q tail
    ExpectRelative(refl.reflpt[90], 4.49987e-10,  2e-2, "Q=0.560");
    // Q=0.600
    ExpectRelative(refl.reflpt[99], 1.19181e-09,  2e-2, "Q=0.600");
}
