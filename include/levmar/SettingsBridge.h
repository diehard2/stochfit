#pragma once

#include "Settings.h"
#include "stochfit/SettingsStruct.h"

// Converts a BoxReflSettings (LevMar FlatBuffer input) into a ReflSettings
// suitable for constructing a ParrattReflectivity instance.
// Only the fields actually read by ParrattReflectivity::ReflConstants are
// filled; CEDP-only fields (FilmSLD, Boxes, Resolution, FilmLength, etc.)
// are left at zero/default.
ReflSettings ToReflSettings(const BoxReflSettings& rs);
