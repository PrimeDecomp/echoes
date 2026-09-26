#ifndef _CTWEAKGUI
#define _CTWEAKGUI

#include "rstl/single_ptr.hpp"

struct SLdrTweakGui;
class CColor;

class CTweakGui {
public:
  explicit CTweakGui(const SLdrTweakGui& data) : mData(&data) {}

  float GetMapAlphaInterpolant() const;

  // Guessed names
  const CColor& GetMapBackgroundColor() const;
  float GetMapBackgroundPulseWidth() const;
  float GetMapBackgroundCycleTime() const;

private:
  const SLdrTweakGui* mData;
};
CHECK_SIZEOF(CTweakGui, 0x4)

extern rstl::single_ptr< CTweakGui > gpTweakGui;

#endif // _CTWEAKGUI
