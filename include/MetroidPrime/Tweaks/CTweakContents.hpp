#ifndef _CTWEAKCONTENTS
#define _CTWEAKCONTENTS

#include "rstl/string.hpp"

#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakAutoMapper.hpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakBall.hpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakCameraBob.hpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakGame.hpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakGui.hpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakGuiColors.hpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakParticle.hpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakPlayer.hpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakPlayerControls.hpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakPlayerGun.hpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakPlayerRes.hpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakSlideShow.hpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakTargeting.hpp"

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
