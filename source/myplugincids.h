//------------------------------------------------------------------------
// Copyright(c) 2023 Sommet.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace SommetApp {
//------------------------------------------------------------------------
static const Steinberg::FUID kControl_MasterProcessorUID (0x3767C222, 0x33F951C2, 0x890F309E, 0xCC3A5006);
static const Steinberg::FUID kControl_MasterControllerUID (0x3F446E37, 0x18E25805, 0xA4155C58, 0x109F5C43);

#define Control_MasterVST3Category "Fx|Network"

//------------------------------------------------------------------------
} // namespace SommetApp
