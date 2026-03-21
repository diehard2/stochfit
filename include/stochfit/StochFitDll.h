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
//
// All data crosses the boundary as FlatBuffers byte buffers (schema: schema/stochfit.fbs).
// Input functions:  (const uint8_t* buf, int len)
// Output functions: (uint8_t* outBuf, int maxLen) → int bytes written
//
// Normal lifecycle:  Init() → Start() → [GetData() polling] → Stop() → GetRunState() → Destroy()
// Cancel (no save):  Init() → Start() → Cancel()
// GpuAvailable() is called at startup to determine if UseGpu can be enabled.

#pragma once
#include "platform.h"
#include <cstdint>

// SA lifecycle
extern "C" EXPORT void        Init(const uint8_t* buf, int len);  // StochFitProto::InitRequest
extern "C" EXPORT const char* GetInitError();
extern "C" EXPORT void        Start(int iterations);
extern "C" EXPORT void        Stop();
extern "C" EXPORT void        Destroy();
extern "C" EXPORT void        Cancel();

// SA polling — returns bytes written into outBuf; outBuf must be pre-allocated by caller
extern "C" EXPORT int GetData    (uint8_t* outBuf, int maxLen);  // → GetDataResult
extern "C" EXPORT int GetRunState(uint8_t* outBuf, int maxLen);  // → GetRunStateResult (call after Stop())
extern "C" EXPORT int ArraySizes (uint8_t* outBuf, int maxLen);  // → ArraySizesResult
extern "C" EXPORT int SAParams   (uint8_t* outBuf, int maxLen);  // → SAParamsResult

extern "C" EXPORT bool WarmedUp();
extern "C" EXPORT bool GpuAvailable();
