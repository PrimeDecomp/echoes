#ifndef _CADDITIVEANIMPLAYBACK
#define _CADDITIVEANIMPLAYBACK

#include "types.h"

#include "Kyoto/Animation/CAdditiveAnimationInfo.hpp"

#include "rstl/rc_ptr.hpp"

class CAnimTreeNode;

class CAdditiveAnimPlayback {
public:
  enum EPlaybackPhase {
    kPP_None,
    kPP_FadingIn,
    kPP_FadingOut,
    kPP_FadedIn,
    kPP_FadedOut,
  };

  CAdditiveAnimPlayback(const rstl::ncrc_ptr< CAnimTreeNode >& anim, float weight, bool loop,
                        const CAdditiveAnimationInfo& info, bool fadeOut);

  const rstl::rc_ptr< CAnimTreeNode >& GetAnimationTree() const { return x8_anim; }
  rstl::ncrc_ptr< CAnimTreeNode >& AnimationTree() { return x8_anim; }

  float GetWeight() const { return x10_targetWeight; }
  void SetWeight(float weight);

  EPlaybackPhase GetFadingMode() const { return x20_phase; }
  bool IsLoop() const { return x18_active; }
  void SetLoop(bool loop) { x18_active = loop; }

  bool IsFadeOutWhenAnimOver() const { return x24_needsFadeOut; }
  void SetFadeOutWhenAnimOver(bool fadeOut) { x24_needsFadeOut = fadeOut; }

  void FadeOut();
  void Update(float dt);

private:
  CAdditiveAnimationInfo x0_info;
  rstl::ncrc_ptr< CAnimTreeNode > x8_anim;
  float x10_targetWeight;
  float x14_curWeight;
  bool x18_active;
  float x1c_weightTimer;
  EPlaybackPhase x20_phase;
  bool x24_needsFadeOut;
};
CHECK_SIZEOF(CAdditiveAnimPlayback, 0x28)

#endif // _CADDITIVEANIMPLAYBACK
