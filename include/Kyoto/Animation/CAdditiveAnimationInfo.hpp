#ifndef _CADDITIVEANIMATIONINFO
#define _CADDITIVEANIMATIONINFO

#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/construct.hpp"

class CAdditiveAnimationInfo {
public:
  CAdditiveAnimationInfo(const float fadeInDur, const float fadeOutDur)
  : mFadeInDur(fadeInDur), mFadeOutDur(fadeOutDur) {}
  CAdditiveAnimationInfo(CInputStream& in)
  : mFadeInDur(in.Get< float >()), mFadeOutDur(in.Get< float >()) {}

  float GetFadeInTime() const { return mFadeInDur; }
  float GetFadeOutTime() const { return mFadeOutDur; }

private:
  float mFadeInDur;
  float mFadeOutDur;
};

namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CAdditiveAnimationInfo)
} // namespace rstl

#endif
