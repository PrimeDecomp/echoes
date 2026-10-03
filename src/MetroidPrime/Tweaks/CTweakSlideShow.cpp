#include "MetroidPrime/Tweaks/CTweakSlideShow.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakSlideShow.hpp"

rstl::string CTweakSlideShow::GetPakFile() const { return mData->pakFile; }

rstl::string CTweakSlideShow::GetFont() const { return mData->font; }

CColor CTweakSlideShow::GetFontColor() const { return mData->fontColor; }

CColor CTweakSlideShow::GetFontOutlineColor() const { return mData->fontOutlineColor; }

float CTweakSlideShow::GetScanPercentInterval() const { return mData->unknown_0xd398dac2; }

float CTweakSlideShow::GetTranslationMultiplier() const { return mData->translationMultiplier; }

float CTweakSlideShow::GetScaleMultiplier() const { return mData->scaleMultiplier; }

float CTweakSlideShow::GetSlideShowDelay() const { return mData->slideShowDelay; }

float CTweakSlideShow::GetSlideBlendTime() const { return mData->slideBlendTime; }

float CTweakSlideShow::GetSlideNumberHideDelay() const { return mData->slideNumberHideDelay; }

float CTweakSlideShow::GetSlideNumberTransitionTime() const {
  return mData->slideNumberTransitionTime;
}

float CTweakSlideShow::GetFadeInTime() const { return mData->fadeInTime; }

float CTweakSlideShow::GetFadeOutTime() const { return mData->fadeOutTime; }

rstl::string CTweakSlideShow::GetStringResName() const { return mData->stringResName; }
