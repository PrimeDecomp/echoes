#ifndef _CTWEAKGUI
#define _CTWEAKGUI

#include "rstl/single_ptr.hpp"
#include "rstl/string.hpp"

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

  // Guessed names, recovered from the credits settings and their consumers.
  rstl::string GetCreditsTable() const;
  rstl::string GetCreditsFont() const;
  CColor GetCreditsFontColor() const;
  CColor GetCreditsOutlineColor() const;
  float GetCreditsTotalTime() const;
  float GetCreditsTextFadeTime() const;
  float GetCreditsMovieFadeTime() const;
  int GetCreditsVolume() const;

private:
  const SLdrTweakGui* mData;
};
CHECK_SIZEOF(CTweakGui, 0x4)

extern rstl::single_ptr< CTweakGui > gpTweakGui;

#endif // _CTWEAKGUI
