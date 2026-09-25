#include "Kyoto/Animation/CAdditiveAnimPlayback.hpp"

#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "rstl/math.hpp"

CAdditiveAnimPlayback::CAdditiveAnimPlayback(const rstl::ncrc_ptr< CAnimTreeNode >& anim,
                                             float weight, bool loop,
                                             const CAdditiveAnimationInfo& info, bool fadeOut)
: x0_info(info)
, x8_anim(anim)
, x10_targetWeight(rstl::max_val(0.f, rstl::min_val(weight, 1.f)))
, x14_curWeight(0.f)
, x18_active(loop)
, x1c_weightTimer(0.f)
, x20_phase(kPP_FadingIn)
, x24_needsFadeOut(!loop && fadeOut) {}

void CAdditiveAnimPlayback::SetWeight(float weight) {
  x10_targetWeight = rstl::max_val(0.f, rstl::min_val(weight, 1.f));

  if (x20_phase == kPP_FadingIn) {
    x14_curWeight = x0_info.GetFadeInTime() > 0.f
                        ? x10_targetWeight * (x1c_weightTimer / x0_info.GetFadeInTime())
                        : x10_targetWeight;
    return;
  }
  if (x20_phase == kPP_FadingOut) {
    x14_curWeight = x0_info.GetFadeOutTime() > 0.f
                        ? x10_targetWeight * (x1c_weightTimer / x0_info.GetFadeOutTime())
                        : x10_targetWeight;
    return;
  }
  x14_curWeight = x10_targetWeight;
}

void CAdditiveAnimPlayback::FadeOut() {
  if (x20_phase == kPP_FadedOut || x20_phase == kPP_FadedIn) {
    x1c_weightTimer = x0_info.GetFadeOutTime();
  } else if (x20_phase == kPP_FadingIn) {
    x1c_weightTimer = (x1c_weightTimer / x0_info.GetFadeInTime()) * x0_info.GetFadeOutTime();
  }

  if (x0_info.GetFadeOutTime() > 0.f) {
    x20_phase = kPP_FadingOut;
    return;
  }

  x20_phase = kPP_FadedOut;
  x14_curWeight = 0.f;
}

void CAdditiveAnimPlayback::Update(float dt) {
  if (x20_phase == kPP_FadingIn) {
    const float time = x0_info.GetFadeInTime();
    x1c_weightTimer = rstl::min_val(time, x1c_weightTimer + dt);
    x14_curWeight = time > 0.f ? x10_targetWeight * (x1c_weightTimer / time) : x10_targetWeight;

    if (close_enough(x14_curWeight, x10_targetWeight)) {
      x20_phase = kPP_FadedIn;
    }
  } else if (x20_phase == kPP_FadingOut) {
    const float time = x0_info.GetFadeOutTime();
    x1c_weightTimer = rstl::max_val(0.f, x1c_weightTimer - dt);
    x14_curWeight = time > 0.f ? x10_targetWeight * (x1c_weightTimer / time) : 0.f;
    if (close_enough(x14_curWeight, 0.f)) {
      x20_phase = kPP_FadedOut;
    }
  }
}
