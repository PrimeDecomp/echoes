#ifndef _CTWEAKGUICOLORS
#define _CTWEAKGUICOLORS

#include "rstl/single_ptr.hpp"

struct SLdrTweakGuiColors;

class CTweakGuiColors {
public:
  explicit CTweakGuiColors(const SLdrTweakGuiColors& data) : mData(&data) {}

private:
  const SLdrTweakGuiColors* mData;
};
CHECK_SIZEOF(CTweakGuiColors, 0x4)

extern rstl::single_ptr< CTweakGuiColors > gpTweakGuiColors;

#endif // _CTWEAKGUICOLORS
