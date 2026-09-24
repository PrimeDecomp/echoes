#ifndef _CADDITIVEANIMPLAYBACK
#define _CADDITIVEANIMPLAYBACK

// TODO: check for Echoes

#include "types.h"

#include "rstl/rc_ptr.hpp"

class CAnimTreeNode;

class CAdditiveAnimationInfo {
private:
  float x0_fadeInDur;
  float x4_fadeOutDur;
};

class CAdditiveAnimPlayback {
public:
  enum EPlaybackPhase {
    kPP_None,
    kPP_FadingIn,
    kPP_FadingOut,
    kPP_FadedIn,
    kPP_FadedOut,
  };

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
