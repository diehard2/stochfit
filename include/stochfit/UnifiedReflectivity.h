#pragma once

#include "CEDP.h"
#include "LayerStack.h"
#include "QSmear.h"

struct ReflConstants {
  double k0;
  double sup_sld;
  double indexsup;
  double indexsupsquared;
  bool qsmear_enabled = false;
  std::vector<double> sinthetai, sinsquaredthetai;

  explicit ReflConstants(const ReflSettings &s);
};

template <typename T> struct WaveScratch {
  std::vector<T> kk, ak, rj, Rj;
  int edp_points = 0;

  void resize(int new_edp_points, int num_threads) {
    if (new_edp_points == edp_points)
      return;
    const int n = new_edp_points * num_threads;
    kk.resize(n);
    ak.resize(n);
    rj.resize(n);
    Rj.resize(n);
    edp_points = new_edp_points;
  }
};

class ParrattReflectivity {
public:
  explicit ParrattReflectivity(const ReflSettings &settings);

  // Primary entry point — takes any LayerStack (CEDP-sourced or box-model).
  // When qsmear is enabled the 13·N spread grid is evaluated then averaged.
  auto CalculateReflectivity(const LayerStack &ls) -> std::span<double>;

  // Convenience overload: builds a LayerStack from EDP and delegates.
  auto CalculateReflectivity(const CEDP &EDP) -> std::span<double>;

private:
  template <bool HasRoughness>
  void ReflectivityCalcCoreImpl(const LayerStack &ls,
                                std::span<const double> sinthetai,
                                std::span<const double> sin2thetai,
                                std::span<double> out,
                                int q_end = std::numeric_limits<int>::max());

  void ReflectivityCalc(const LayerStack &ls,
                        int q_end = std::numeric_limits<int>::max());
  void TransparentReflectivityCalc(const LayerStack &ls);

  const ReflConstants m_consts;
  WaveScratch<std::complex<double>> m_complex;
  WaveScratch<double> m_real;

  bool m_qsmear_enabled = false;
  std::vector<double> m_refl_out;      // Parratt output (sinthetai.size(): N or 13·N)
  std::vector<double> m_refl_smeared;  // Smeared result (N); only used when qsmear on

  int m_num_threads = 1;
};
