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

// FFI entry points for levmardll.
// All data crosses the boundary as FlatBuffers byte buffers (schema: schema/stochfit.fbs).
// Input: LevMarRequest (BoxReflSettings + parameters)
// Output: result table — returns bytes written into outBuf.

#pragma once
#include "platform.h"
#include <cstdint>

extern "C" EXPORT int FastReflfit     (const uint8_t* inBuf, int inLen, uint8_t* outBuf, int maxLen);
extern "C" EXPORT int FastReflGenerate(const uint8_t* inBuf, int inLen, uint8_t* outBuf, int maxLen);
extern "C" EXPORT int Rhofit          (const uint8_t* inBuf, int inLen, uint8_t* outBuf, int maxLen);
extern "C" EXPORT int RhoGenerate     (const uint8_t* inBuf, int inLen, uint8_t* outBuf, int maxLen);
extern "C" EXPORT int StochFitBoxModel(const uint8_t* inBuf, int inLen, uint8_t* outBuf, int maxLen);
