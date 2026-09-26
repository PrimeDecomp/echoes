#ifndef _CTWEAKGUI
#define _CTWEAKGUI

#include "rstl/single_ptr.hpp"

struct SLdrTweakGui;

class CTweakGui {
public:
  explicit CTweakGui(const SLdrTweakGui& data) : mData(&data) {}

  float GetMapAlphaInterpolant() const;

private:
  const SLdrTweakGui* mData;
};
CHECK_SIZEOF(CTweakGui, 0x4)

extern rstl::single_ptr< CTweakGui > gpTweakGui;

#endif // _CTWEAKGUI
