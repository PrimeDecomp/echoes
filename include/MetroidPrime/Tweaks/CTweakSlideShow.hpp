#ifndef _CTWEAKSLIDESHOW
#define _CTWEAKSLIDESHOW

#include "Kyoto/Graphics/CColor.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/string.hpp"

struct SLdrTweakSlideShow;

class CTweakSlideShow {
public:
  explicit CTweakSlideShow(const SLdrTweakSlideShow& data) : mData(&data) {}

  rstl::string GetPakFile() const;            // Guessed name.
  rstl::string GetFont() const;               // Guessed name.
  CColor GetFontColor() const;                // Guessed name.
  CColor GetFontOutlineColor() const;         // Guessed name.
  float GetScanPercentInterval() const;       // Guessed name.
  float GetTranslationMultiplier() const;     // Guessed name.
  float GetScaleMultiplier() const;           // Guessed name.
  float GetSlideShowDelay() const;            // Guessed name.
  float GetSlideBlendTime() const;            // Guessed name.
  float GetSlideNumberHideDelay() const;      // Guessed name.
  float GetSlideNumberTransitionTime() const; // Guessed name.
  float GetFadeInTime() const;                // Guessed name.
  float GetFadeOutTime() const;               // Guessed name.
  rstl::string GetStringResName() const;      // Guessed name.

private:
  const SLdrTweakSlideShow* mData;
};
CHECK_SIZEOF(CTweakSlideShow, 0x4)

extern rstl::single_ptr< CTweakSlideShow > gpTweakSlideShow;

#endif // _CTWEAKSLIDESHOW
