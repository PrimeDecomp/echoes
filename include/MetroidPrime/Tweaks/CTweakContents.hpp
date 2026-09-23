#ifndef _CTWEAKCONTENTS
#define _CTWEAKCONTENTS

#include "rstl/string.hpp"

#include "MetroidPrime/ScriptLoader/SLdrTweakAutoMapper.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakBall.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakCameraBob.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGame.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGui.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGuiColors.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakParticle.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerControls.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerRes.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakSlideShow.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakTargeting.hpp"

class CTweakContents {
public:
  CTweakContents();
  ~CTweakContents();

  SLdrTweakAutoMapper TweakAutoMapper;
  SLdrTweakBall TweakBall;
  SLdrTweakCameraBob TweakCameraBob;
  SLdrTweakGame TweakGame;
  SLdrTweakGui TweakGui;
  SLdrTweakGuiColors TweakGuiColors;
  SLdrTweakParticle TweakParticle;
  SLdrTweakPlayer TweakPlayer;
  SLdrTweakPlayer TweakPlayer2;
  SLdrTweakPlayerControls TweakPlayerControls;
  SLdrTweakPlayerControls TweakPlayerControls2;
  SLdrTweakPlayerGun TweakPlayerGun;
  SLdrTweakPlayerGun TweakPlayerGunMuli;
  SLdrTweakPlayerRes TweakPlayerRes;
  SLdrTweakSlideShow TweakSlideShow;
  SLdrTweakTargeting TweakTargeting;
};

extern CTweakContents* gpTweakContents;

#endif
