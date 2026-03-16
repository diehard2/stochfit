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

// FFI entry points exported from stochfit_shared.dll / libstochfit.so.
// Called from the Electron GUI via koffi. All functions use C linkage.
// Normal lifecycle:    Init() → Start() → [GetData() polling] → Stop() → GetRunState() → Destroy()
// Cancel (no save):    Init() → Start() → [GetData() polling] → Cancel()
// GpuAvailable() is called at startup to determine if UseGpu can be enabled.

#include <stochfit/common/platform.h>
#include <stochfit/stochfitdll/SettingsStruct.h>

extern "C" EXPORT void Init(ReflSettings* initstruct, StochRunState* state);
extern "C" EXPORT const char* GetInitError();
// ******** MAYBEDEAD ******** GenPriority — not called from GUI (thread priority not portable)
extern "C" EXPORT void GenPriority(int priority);
extern "C" EXPORT void Start(int iterations);
// Stop: request_stop + join — blocks until worker exits, then safe to call GetRunState.
extern "C" EXPORT void Stop();
// Destroy: delete the stochfit object. Call after GetRunState.
extern "C" EXPORT void Destroy();
// Cancel: Stop + Destroy combined (for cases where state save is not needed).
extern "C" EXPORT void Cancel();
extern "C" EXPORT int GetData(double ZRange[],double Rho[],double QRange[], double Refl[] ,double* roughness, double* chisquare, double* goodnessoffit, BOOL* isfinished);
// GetRunState: fills flat output buffers. Only call after Stop().
// saScalars[9]: roughness, filmAbsInput, surfAbs, temperature, impNorm, avgfSTUN, bestSolution, chiSquare, goodnessOfFit
extern "C" EXPORT void GetRunState(double* saScalars, double* edValues, int* edCount);
extern "C" EXPORT void ArraySizes(int* RhoSize, int* Reflsize);
extern "C" EXPORT bool WarmedUp();
extern "C" EXPORT void SAparams(double* lowestenergy, double* temp, int* mode);
extern "C" EXPORT bool GpuAvailable();