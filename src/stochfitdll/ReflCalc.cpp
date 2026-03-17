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

#include "ReflCalc.h"
#include "ParamVector.h"
#include "platform.h"
#include "QSmear.h"


// Multilayer reflection and transmission
// The calculation scheme used can be found in
// L. G. Parratt, Phys. Rev. 95(2), 359(1954)

CReflCalc::CReflCalc() : m_bReflInitialized(false) {}

tl::expected<void, std::string> CReflCalc::Init(ReflSettings *InitStruct) {
  lambda = InitStruct->Wavelength;
  k0 = 2.0 * std::numbers::pi / lambda;

  objectivefunction = InitStruct->Objectivefunction;
  m_bforcenorm = InitStruct->Forcenorm;
  m_dnormfactor = 1.0;
  m_dQSpread = InitStruct->QErr / 100;
  m_bXRonly = InitStruct->XRonly;
  m_bImpNorm = InitStruct->Impnorm;

  // Setup OpenMP - currently a maximum of 8 processors is allowed. After a
  // certain number of data points, there will not be much of a benefit to
  // allowing additional processors to work on the calculation. If more than
  // 2-300 data points are being analyzed, it would make sense to increase this
  // number
  if (MAX_OMP_THREADS > omp_get_num_procs())
    m_iuseableprocessors = omp_get_num_procs();
  else
    m_iuseableprocessors = MAX_OMP_THREADS;

  // For Debugging Purposes
#ifdef SINGLEPROCDEBUG
  m_iuseableprocessors = 1;
#endif

  omp_set_num_threads(m_iuseableprocessors);
  return SetupRef(InitStruct);
}

tl::expected<void, std::string> CReflCalc::SetupRef(ReflSettings *InitStruct) {
  if (InitStruct->Refl == nullptr)
    return tl::unexpected(
        std::string("Refl data pointer is null — check data file format"));
  if (InitStruct->ReflError == nullptr)
    return tl::unexpected(
        std::string("ReflError data pointer is null — check data file format"));

  // Now create our xi,yi,dyi, and thetai
  m_idatapoints = InitStruct->QPoints - InitStruct->HighQOffset -
                  InitStruct->CritEdgeOffset;
  xi.resize(m_idatapoints);
  yi.resize(m_idatapoints);

  if (InitStruct->QError != NULL)
    exi = vector<double>(m_idatapoints);

  eyi.resize(m_idatapoints);
  sinthetai.resize(m_idatapoints);
  sinsquaredthetai.resize(m_idatapoints);
  qspreadsinthetai.resize(m_idatapoints * 13);
  qspreadsinsquaredthetai.resize(m_idatapoints * 13);
  reflpt.resize(m_idatapoints);
  qspreadreflpt.resize(m_idatapoints * 13);

  // and fill them up
  for (int i = 0; i < m_idatapoints; i++) {
    xi[i] = InitStruct->Q[i + InitStruct->CritEdgeOffset];
    yi[i] = InitStruct->Refl[i + InitStruct->CritEdgeOffset];
    eyi[i] = InitStruct->ReflError[i + InitStruct->CritEdgeOffset];

    if (exi.has_value())
      (*exi)[i] = InitStruct->QError[i + InitStruct->CritEdgeOffset];

    sinthetai[i] = InitStruct->Q[i + InitStruct->CritEdgeOffset] * lambda /
                   (4 * std::numbers::pi);
  }

  // Calculate the qspread sinthetai's for resolution smearing
  QSmear::BuildArrays(m_idatapoints, lambda, m_dQSpread,
                      sinthetai.data(),
                      exi.has_value() ? exi->data() : nullptr,
                      qspreadsinthetai.data(), qspreadsinsquaredthetai.data());

  // Now, create our q's for plotting, but mix-in the actual data points
  double x0 = InitStruct->Q[0];
  double x1 = InitStruct->Q[InitStruct->QPoints - 1];
  double dx = (x1 - x0) / 150.0;
  x1 = 1.1 * x1 - 0.1 * x0;

  tsinthetai.resize(3000);
  dataout.resize(3000);
  qarray.resize(3000);
  tarraysize = 0;

  int j = 0;
  int inc = 0;
  double x;

  for (x = x0; x <= x1; x += dx) {
    if (inc < m_idatapoints) {
      if (x == xi[inc]) {
        qarray[j] = x;
        tsinthetai[j] = x * lambda / (4 * std::numbers::pi);
        inc++;
      } else if (x < xi[inc]) {
        qarray[j] = x;
        tsinthetai[j] = x * lambda / (4 * std::numbers::pi);
      } else {
        qarray[j] = xi[inc];
        tsinthetai[j] = xi[inc] * lambda / (4 * std::numbers::pi);
        inc++;
        x -= dx;
      }
    } else {
      qarray[j] = x;
      tsinthetai[j] = x * lambda / (4 * std::numbers::pi);
    }
    tarraysize++;
    j++;
  }

  // Calculate the theta's we'll use to make our reflectivities
  for (int l = 0; l < m_idatapoints; l++) {
    sinsquaredthetai[l] = sinthetai[l] * sinthetai[l];
  }

  // Calculate the theta's we'll use to make our plotting reflectivity
  tsinsquaredthetai.resize(tarraysize);

  for (int l = 0; l < tarraysize; l++) {
    tsinsquaredthetai[l] = tsinthetai[l] * tsinthetai[l];
  }

  return {};
}

// Write output files
void CReflCalc::ParamsRF(CEDP *EDP, string reflfile) {
  ofstream reflout(reflfile.c_str());

  if (m_dQSpread == 0.0 || !exi.has_value()) {
    if (EDP->Get_UseABS() == false)
      MyTransparentRF(tsinthetai.data(), tsinsquaredthetai.data(), tarraysize,
                      dataout.data(), EDP);
    else
      MyRF(tsinthetai.data(), tsinsquaredthetai.data(), tarraysize,
           dataout.data(), EDP);
  } else {
    if (EDP->Get_UseABS())
      MyTransparentRF(qspreadsinthetai.data(), qspreadsinsquaredthetai.data(),
                      13 * m_idatapoints, qspreadreflpt.data(), EDP);
    else
      MyRF(qspreadsinthetai.data(), qspreadsinsquaredthetai.data(),
           13 * m_idatapoints, qspreadreflpt.data(), EDP);

    QsmearRf(qspreadreflpt.data(), reflpt.data(), m_idatapoints);
  }

  if (m_bforcenorm) {
    impnorm(dataout.data(), tarraysize, false);
  }

  if (m_bImpNorm) {
    impnorm(dataout.data(), tarraysize, true);
  }

#ifndef CHECKREFLCALC
  for (int i = 0; i < tarraysize; i++) {
    reflout << qarray[i] << " " << dataout[i] << endl;
  }
#else
  for (int i = 0; i < m_idatapoints; i++) {
    reflout << xi[i] << " " << reflpt[i] << endl;
  }
#endif

  reflout.close();
}

// Check to see if there is any negative electron density for the XR case, false
// if there is neg ED
bool CReflCalc::CheckDensity(CEDP *EDP) {
  int EDPoints = EDP->Get_EDPPointCount();
  std::complex<double> *tEDP = EDP->m_EDP.data();

  for (int i = 0; i < EDPoints; i++) {
    if (tEDP[i].real() < 0)
      return false;
  }

  return true;
}

double CReflCalc::Objective(CEDP *EDP) {
  int counter = m_idatapoints;

  if (m_bXRonly == true) {
    if (CheckDensity(EDP) == false)
      return -1;
  }

  if (m_dQSpread == 0.0 || !exi.has_value()) {
    if (!EDP->Get_UseABS())
      MyTransparentRF(sinthetai.data(), sinsquaredthetai.data(), m_idatapoints,
                      reflpt.data(), EDP);
    else
      MyRF(sinthetai.data(), sinsquaredthetai.data(), m_idatapoints,
           reflpt.data(), EDP);
  } else {
    if (EDP->Get_UseABS())
      MyTransparentRF(qspreadsinthetai.data(), qspreadsinsquaredthetai.data(),
                      13 * m_idatapoints, qspreadreflpt.data(), EDP);
    else
      MyRF(qspreadsinthetai.data(), qspreadsinsquaredthetai.data(),
           13 * m_idatapoints, qspreadreflpt.data(), EDP);

    QsmearRf(qspreadreflpt.data(), reflpt.data(), m_idatapoints);
  }

  // Normalize if we let the absorption vary
  if (m_bforcenorm) {
    impnorm(reflpt.data(), m_idatapoints, false);
  }

  // Fix imperfect normalization
  if (m_bImpNorm) {
    impnorm(reflpt.data(), m_idatapoints, true);
  }

  // Calculate the fitness score
  double calcholder1 = 0;

  m_dChiSquare = 0;
  m_dgoodnessoffit = 0;

  // Log difference
  if (objectivefunction == 0) {
    for (int i = 0; i < counter; i++) {
      calcholder1 = log(yi[i]) - log(reflpt[i]);
      m_dgoodnessoffit += calcholder1 * (calcholder1); /// fabs(log(eyi[i]));
    }

    m_dgoodnessoffit /= counter + 1;
  } else if (objectivefunction == 1) {
    // Inverse difference
    for (int i = 0; i < counter; i++) {
      calcholder1 = yi[i] / reflpt[i];

      if (calcholder1 < 1.0)
        calcholder1 = 1.0 / calcholder1;

      m_dgoodnessoffit += (1.0 - calcholder1) * (1.0 - calcholder1);
    }

    m_dgoodnessoffit /= counter + 1;
  } else if (objectivefunction == 2) { // Log difference with errors
    for (int i = 0; i < counter; i++) {
      calcholder1 = log(yi[i]) - log(reflpt[i]);
      m_dgoodnessoffit += calcholder1 * calcholder1 / fabs(log(eyi[i]));
    }

    m_dgoodnessoffit /= counter + 1;
  } else if (objectivefunction == 3) {
    // Inverse difference with errors
    for (int i = 0; i < counter; i++) {
      calcholder1 = yi[i] / reflpt[i];

      if (calcholder1 < 1.0)
        calcholder1 = 1.0 / calcholder1;

      double errormap = (yi[i] / eyi[i]) * (yi[i] / eyi[i]);
      m_dgoodnessoffit += (1.0 - calcholder1) * (1.0 - calcholder1) * errormap;
    }

    m_dgoodnessoffit /= counter + 1;
  }

  // Calculate the Chi Square (reduced with no parameters)
  for (int i = 0; i < counter; i++) {
    calcholder1 = (yi[i] - reflpt[i]);
    m_dChiSquare += (calcholder1 * calcholder1) / (eyi[i] * eyi[i]);
  }

  m_dChiSquare /= counter;

  return (m_dgoodnessoffit);
}

// Perform a rudimentary normalization on the modeled reflectivity (for
// absorbing films) This is for the output reflectivity. If the normalization is
// imperfect for neutrons, this should use isimprefl = true
void CReflCalc::impnorm(double *refl, int datapoints, bool isimprefl) {
  double normfactor;

  if (isimprefl == true)
    normfactor = m_dnormfactor;
  else
    normfactor = 1.0 / refl[0];

  for (int i = 0; i < datapoints; i++) {
    refl[i] *= normfactor;
  }
}

void CReflCalc::MyRF(double *sintheta, double *sinsquaredtheta, int datapoints,
                     double *refl, CEDP *EDP) {
  std::complex<double> *DEDP = EDP->m_DEDP.data();
  int EDPoints = EDP->Get_EDPPointCount();

  if (!m_bReflInitialized) {
    InitializeScratchArrays(EDP->Get_EDPPointCount());
  }

  // Calculate some complex constants to keep them out of the loop
  std::complex<double> lengthmultiplier =
      -2.0 * std::complex<double>(0.0, 1.0) * EDP->Get_Dz();
  std::complex<double> indexsup = 1.0 - DEDP[0] / 2.0;
  std::complex<double> indexsupsquared = indexsup * indexsup;
  std::complex<double> zero;

  int HighOffSet = 0;
  int LowOffset = 0;

  GetOffSets(HighOffSet, LowOffset, DEDP, EDPoints);

#pragma omp parallel
  {
    // Figure out the number of the thread we're currently in
    int threadnum = omp_get_thread_num();
    int arrayoffset = threadnum * EDPoints;

    double *dkk = m_dkk.data() + arrayoffset;
    std::complex<double> *kk = m_ckk.data() + arrayoffset;
    std::complex<double> *ak = m_cak.data() + arrayoffset;
    double *drj = m_drj.data() + arrayoffset;
    std::complex<double> *rj = m_crj.data() + arrayoffset;
    std::complex<double> *Rj = m_cRj.data() + arrayoffset;
    std::complex<double> cholder, tempk1, tempk2;
    double holder;

    /********Boundary conditions********/
    // No reflection in the last layer
    Rj[EDPoints - 1] = 0.0;
    // The first layer and the last layer are assumed to be infinite
    // e^-(0+i*(layerlength)) = 1+e^(-inf*i*beta) = 1 + 0*i*beta
    ak[EDPoints - 1] = 1.0;
    ak[0] = 1.0;

    // In order to vectorize loops, you cannot use global variables
    int nlminone = EDPoints - 1;
    int numlay = EDPoints;

#pragma omp for schedule(runtime)
    for (int l = 0; l < datapoints; l++) {
      // The refractive index for air is 1, so there is no refractive index term
      // for kk[0]
      kk[0] = k0 * indexsup * sintheta[l];

      tempk1 = k0 * std::sqrt(indexsupsquared * sinsquaredtheta[l] - DEDP[1] +
                              DEDP[0]);
      tempk2 = k0 * std::sqrt(indexsupsquared * sinsquaredtheta[l] -
                              DEDP[numlay - 1] + DEDP[0]);
      // Workout the wavevector k -> kk[i] = k0
      // *std::sqrt(sinsquaredthetai[l]-2.0*nk[i]);
      for (int i = 1; i <= LowOffset; i++) {
        kk[i] = tempk1;
      }

      for (int i = LowOffset + 1; i < HighOffSet; i++) {
        kk[i] = k0 * std::sqrt(indexsupsquared * sinsquaredtheta[l] - DEDP[i] +
                               DEDP[0]);
      }

      for (int i = HighOffSet; i < numlay; i++) {
        kk[i] = tempk2;
      }

      // Make the phase factors ak -> ak[i] =
      // std::exp(-1.0*imaginary*kk[i]*dz0/2.0);

      tempk1 = std::exp(lengthmultiplier * kk[1]);
      tempk2 = std::exp(lengthmultiplier * kk[numlay - 1]);

      for (int i = 1; i <= LowOffset; i++) {
        ak[i] = tempk1;
      }

      for (int i = LowOffset + 1; i < HighOffSet; i++) {
        ak[i] = std::exp(lengthmultiplier * kk[i]);
      }

      for (int i = HighOffSet; i < numlay - 1; i++) {
        ak[i] = tempk2;
      }

      // Make the Fresnel coefficients -> rj[i]
      // =(kk[i]-kk[i+1])/(kk[i]+kk[i+1]);

      for (int i = 0; i <= LowOffset; i++) {
        rj[i] = zero;
      }

      for (int i = LowOffset + 1; i < HighOffSet; i++) {
        rj[i] = (kk[i] - kk[i + 1]) / (kk[i] + kk[i + 1]);
      }

      for (int i = HighOffSet; i < numlay - 1; i++) {
        rj[i] = zero;
      }

      // Parratt recursion of the amplitude reflectivity

      for (int i = EDPoints - 2; i >= 0; i--) {
        Rj[i] = ak[i] * (Rj[i + 1] + rj[i]) / (Rj[i + 1] * rj[i] + 1.0);
      }

      // The magnitude of the reflection at layer 0 is the measured reflectivity
      // of the film
      holder = std::abs(Rj[0]);
      refl[l] = holder * holder;
    }
  }
}

void CReflCalc::MyTransparentRF(double *sintheta, double *sinsquaredtheta,
                                int datapoints, double *refl, CEDP *EDP) {
  std::complex<double> *DEDP = EDP->m_DEDP.data();
  int EDPoints = EDP->Get_EDPPointCount();

  if (!m_bReflInitialized) {
    InitializeScratchArrays(EDP->Get_EDPPointCount());
  }

  ////Calculate some complex constants to keep them out of the loop
  std::complex<double> lengthmultiplier =
      -2.0 * std::complex<double>(0.0, 1.0) * EDP->Get_Dz();
  std::complex<double> indexsup = 1.0 - DEDP[0] / 2.0;
  std::complex<double> indexsupsquared = indexsup * indexsup;
  int HighOffSet = EDPoints;
  int LowOffset = 0;
  int offset = datapoints;

  GetOffSets(HighOffSet, LowOffset, DEDP, EDPoints);

  // Find the point at which we no longer need to use complex numbers
  // exclusively
  for (int i = 0; i < datapoints; i++) {
    int neg = 0;
    for (int k = 0; k < EDPoints; k++) {
      if ((indexsupsquared.real() * sinsquaredtheta[i] - DEDP[k].real() +
           DEDP[0].real()) < 0.0) {
        neg -= 1;
        break;
      }
    }
    if (neg == 0) {
      offset = i;
      if (m_dQSpread == 0)
        break;
    }
  }

  // In order to vectorize loops, you cannot use global variables
  int EDPointsMinOne = EDPoints - 1;

#pragma omp parallel
  {
    // Figure out the number of the thread we're currently in
    int threadnum = omp_get_thread_num();
    int arrayoffset = threadnum * EDPoints;

    double *dkk = m_dkk.data() + arrayoffset;
    std::complex<double> *kk = m_ckk.data() + arrayoffset;
    std::complex<double> *ak = m_cak.data() + arrayoffset;
    double *drj = m_drj.data() + arrayoffset;
    std::complex<double> *rj = m_crj.data() + arrayoffset;
    std::complex<double> *Rj = m_cRj.data() + arrayoffset;
    std::complex<double> cholder, tempk1, tempk2, zero;
    double holder, dtempk1, dtempk2;

    /********Boundary conditions********/
    // No reflection in the last layer
    Rj[EDPointsMinOne] = 0.0;
    // The first layer and the last layer are assumed to be infinite
    ak[EDPointsMinOne] = 1.0;
    ak[0] = 1.0;

#pragma omp for nowait schedule(guided)
    for (int l = 0; l < offset; l++) {

      // The refractive index for air is 1, so there is no refractive index term
      // for kk[0]
      kk[0] = k0 * indexsup * sintheta[l];

      tempk1 = k0 * std::sqrt(indexsupsquared * sinsquaredtheta[l] - DEDP[1] +
                              DEDP[0]);
      tempk2 = k0 * std::sqrt(indexsupsquared * sinsquaredtheta[l] -
                              DEDP[EDPointsMinOne] + DEDP[0]);
      // Workout the wavevector k -> kk[i] = k0
      // *std::sqrt(sinsquaredthetai[l]-2.0*nk[i]);
      for (int i = 1; i <= LowOffset; i++) {
        kk[i] = tempk1;
      }

      for (int i = LowOffset + 1; i < HighOffSet; i++) {
        kk[i] = k0 * std::sqrt(indexsupsquared * sinsquaredtheta[l] - DEDP[i] +
                               DEDP[0]);
      }

      for (int i = HighOffSet; i < EDPoints; i++) {
        kk[i] = tempk2;
      }

      // Make the phase factors ak -> ak[i] =
      // std::exp(-1.0*imaginary*kk[i]*dz0/2.0);

      tempk1 = std::exp(lengthmultiplier * kk[1]);
      tempk2 = std::exp(lengthmultiplier * kk[EDPointsMinOne]);

      for (int i = 1; i <= LowOffset; i++) {
        ak[i] = tempk1;
      }

      for (int i = LowOffset + 1; i < HighOffSet; i++) {
        ak[i] = std::exp(lengthmultiplier * kk[i]);
      }

      for (int i = HighOffSet; i < EDPointsMinOne; i++) {
        ak[i] = tempk2;
      }

      // Make the Fresnel coefficients -> rj[i]
      // =(kk[i]-kk[i+1])/(kk[i]+kk[i+1]);

      for (int i = 0; i < LowOffset; i++) {
        rj[i] = zero;
      }

      for (int i = LowOffset + 1; i < HighOffSet; i++) {
        rj[i] = (kk[i] - kk[i + 1]) / (kk[i] + kk[i + 1]);
      }

      for (int i = HighOffSet; i < EDPointsMinOne; i++) {
        rj[i] = zero;
      }

      // Parratt recursion of the amplitude reflectivity

      for (int i = EDPoints - 2; i >= 0; i--) {
        Rj[i] = ak[i] * (Rj[i + 1] + rj[i]) / (Rj[i + 1] * rj[i] + 1.0);
      }

      // The magnitude of the reflection at layer 0 is the measured reflectivity
      // of the film
      holder = std::abs(Rj[0]);
      refl[l] = holder * holder;
    }

// Now calculate the rest using doubles
#pragma omp for schedule(guided)
    for (int l = offset; l < datapoints; l++) {
      // The refractive index for air is 1, so there is no refractive index term
      // for kk[0]
      dkk[0] = k0 * indexsup.real() * sintheta[l];

      dtempk1 = k0 * sqrt(indexsupsquared.real() * sinsquaredtheta[l] -
                          DEDP[1].real() + DEDP[0].real());
      dtempk2 = k0 * sqrt(indexsupsquared.real() * sinsquaredtheta[l] -
                          DEDP[EDPointsMinOne].real() + DEDP[0].real());
      // Workout the wavevector k -> kk[i] = k0
      // *std::sqrt(sinsquaredthetai[l]-2.0*nk[i]);
      for (int i = 1; i <= LowOffset; i++) {
        dkk[i] = dtempk1;
      }

      for (int i = LowOffset + 1; i < HighOffSet; i++) {
        dkk[i] = k0 * sqrt(indexsupsquared.real() * sinsquaredtheta[l] -
                           DEDP[i].real() + DEDP[0].real());
      }

      for (int i = HighOffSet; i < EDPoints; i++) {
        dkk[i] = dtempk2;
      }

      // Make the phase factors ak -> ak[i] =
      // std::exp(-1.0*imaginary*kk[i]*dz0/2.0);

      tempk1 = std::exp(lengthmultiplier * dkk[1]);
      tempk2 = std::exp(lengthmultiplier * dkk[EDPointsMinOne]);

      for (int i = 1; i <= LowOffset; i++) {
        ak[i] = tempk1;
      }

      for (int i = LowOffset + 1; i < HighOffSet; i++) {
        ak[i] = std::exp(lengthmultiplier * dkk[i]);
      }

      for (int i = HighOffSet; i < EDPointsMinOne; i++) {
        ak[i] = tempk2;
      }

      // Make the Fresnel coefficients -> rj[i]
      // =(kk[i]-kk[i+1])/(kk[i]+kk[i+1]);

      for (int i = 0; i <= LowOffset; i++) {
        drj[i] = 0.0;
      }

      for (int i = LowOffset + 1; i < HighOffSet; i++) {
        drj[i] = (dkk[i] - dkk[i + 1]) / (dkk[i] + dkk[i + 1]);
      }

      for (int i = HighOffSet; i < EDPointsMinOne; i++) {
        drj[i] = 0.0;
      }

      // Parratt recursion of the amplitude reflectivity

      for (int i = EDPoints - 2; i >= 0; i--) {
        Rj[i] = ak[i] * (Rj[i + 1] + drj[i]) / (Rj[i + 1] * drj[i] + 1.0);
      }

      // The magnitude of the reflection at layer 0 is the measured reflectivity
      // of the film
      holder = std::abs(Rj[0]);
      refl[l] = holder * holder;
    }
  }
}

void CReflCalc::InitializeScratchArrays(int EDPoints) {
  // Create the scratch arrays for the reflectivity calculation
  m_ckk.resize(EDPoints * m_iuseableprocessors);
  m_dkk.resize(EDPoints * m_iuseableprocessors);
  m_cak.resize(EDPoints * m_iuseableprocessors);
  m_crj.resize(EDPoints * m_iuseableprocessors);
  m_drj.resize(EDPoints * m_iuseableprocessors);
  m_cRj.resize(EDPoints * m_iuseableprocessors);

  m_bReflInitialized = true;
}

void CReflCalc::QsmearRf(double *qspreadreflpt, double *refl, int datapoints) {
  QSmear::Apply(qspreadreflpt, refl, datapoints);
}

int CReflCalc::GetDataCount() {
#ifndef CHECKREFLCALC
  if (m_dQSpread == 0.0 || !exi.has_value())
    return tarraysize;
  else
    return m_idatapoints;
#else
  return m_idatapoints;
#endif
}

void CReflCalc::GetOffSets(int &HighOffset, int &LowOffset,
                           std::complex<double> *EDP, int EDPoints) {
  // Find duplicate pts so we don't do the same calculation over and over again
  for (int i = 0; i < EDPoints; i++) {
    if (EDP[i].real() == EDP[0].real())
      LowOffset++;
    else
      break;
  }

  // Cap LowOffset: loops use `i <= LowOffset` starting at i=1, so max valid
  // index is EDPoints-1
  if (LowOffset >= EDPoints)
    LowOffset = EDPoints - 1;

  for (int i = EDPoints - 1; i != 0; i--) {
    if (EDP[i].real() == EDP[EDPoints - 1].real())
      HighOffset = i;
    else
      break;
  }
}

// Forward calculation without fitting data (used by mirefl and similar tools).
// Results are stored in reflpt[].
void CReflCalc::CalculateReflectivity(CEDP *EDP) {
  if (m_bXRonly == true) {
    if (CheckDensity(EDP) == false)
      return;
  }

  if (m_dQSpread < 0.005 || !exi.has_value()) {
    if (EDP->Get_UseABS() == false)
      MyTransparentRF(sinthetai.data(), sinsquaredthetai.data(), m_idatapoints,
                      reflpt.data(), EDP);
    else
      MyRF(sinthetai.data(), sinsquaredthetai.data(), m_idatapoints,
           reflpt.data(), EDP);
  } else {
    if (EDP->Get_UseABS())
      MyTransparentRF(qspreadsinthetai.data(), qspreadsinsquaredthetai.data(),
                      13 * m_idatapoints, qspreadreflpt.data(), EDP);
    else
      MyRF(qspreadsinthetai.data(), qspreadsinsquaredthetai.data(),
           13 * m_idatapoints, qspreadreflpt.data(), EDP);

    QsmearRf(qspreadreflpt.data(), reflpt.data(), m_idatapoints);
  }

  if (m_bforcenorm)
    impnorm(reflpt.data(), m_idatapoints, false);

  if (m_bImpNorm)
    impnorm(reflpt.data(), m_idatapoints, true);
}