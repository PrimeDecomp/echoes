
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "REL/REL_Setup.h"

#include "MetroidPrime/CMappableObject.hpp"
#include "MetroidPrime/Player/CPlayerCameraBob.hpp"
#include "MetroidPrime/Tweaks/CTweakAutoMapper.hpp"
#include "MetroidPrime/Tweaks/CTweakBall.hpp"
#include "MetroidPrime/Tweaks/CTweakContents.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"
#include "MetroidPrime/Tweaks/CTweakParticle.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerControls.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"
#include "MetroidPrime/Tweaks/CTweakSlideShow.hpp"
#include "MetroidPrime/Tweaks/CTweakTargeting.hpp"

#include "dolphin/types.h"

STweaks_FuncPtrs REL_loader_Tweaks;

CTweakContents::CTweakContents() {}

CTweakContents::~CTweakContents() {}

void DecodeAnyTweak(uint instanceId, CInputStream& input) {
  switch (instanceId) {

  case 0x5457414d:
    LoadTypedefSLdrTweakAutoMapper(gpTweakContents->TweakAutoMapper, input);
    break;
  case 0x5457424c:
    LoadTypedefSLdrTweakBall(gpTweakContents->TweakBall, input);
    break;
  case 0x54574342:
    LoadTypedefSLdrTweakCameraBob(gpTweakContents->TweakCameraBob, input);
    break;
  case 0x5457474d:
    LoadTypedefSLdrTweakGame(gpTweakContents->TweakGame, input);
    break;
  case 0x54574755:
    LoadTypedefSLdrTweakGui(gpTweakContents->TweakGui, input);
    break;
  case 0x54574743:
    LoadTypedefSLdrTweakGuiColors(gpTweakContents->TweakGuiColors, input);
    break;
  case 0x54575041:
    LoadTypedefSLdrTweakParticle(gpTweakContents->TweakParticle, input);
    break;
  case 0x5457504c:
    LoadTypedefSLdrTweakPlayer(gpTweakContents->TweakPlayer, input);
    break;
  case 0x54575032:
    LoadTypedefSLdrTweakPlayer(gpTweakContents->TweakPlayer2, input);
    break;
  case 0x54575043:
    LoadTypedefSLdrTweakPlayerControls(gpTweakContents->TweakPlayerControls, input);
    break;
  case 0x54574332:
    LoadTypedefSLdrTweakPlayerControls(gpTweakContents->TweakPlayerControls2, input);
    break;
  case 0x54575047:
    LoadTypedefSLdrTweakPlayerGun(gpTweakContents->TweakPlayerGun, input);
    break;
  case 0x5457504d:
    LoadTypedefSLdrTweakPlayerGun(gpTweakContents->TweakPlayerGunMuli, input);
    break;
  case 0x54575052:
    LoadTypedefSLdrTweakPlayerRes(gpTweakContents->TweakPlayerRes, input);
    break;
  case 0x54575353:
    LoadTypedefSLdrTweakSlideShow(gpTweakContents->TweakSlideShow, input);
    break;
  case 0x54575447:
    LoadTypedefSLdrTweakTargeting(gpTweakContents->TweakTargeting, input);
    break;
  default:
    break;
  }
}

#include "../ScriptLoader/TweaksArchive.inc"

void REL_CreateTweakGlobals() {
  gpTweakAutoMapper = rs_new CTweakAutoMapper(gpTweakContents->TweakAutoMapper);
  gpTweakBall = rs_new CTweakBall(gpTweakContents->TweakBall);
  gpTweakGame = rs_new CTweakGame(gpTweakContents->TweakGame);
  gpTweakGui = rs_new CTweakGui(gpTweakContents->TweakGui);
  gpTweakGuiColors = rs_new CTweakGuiColors(gpTweakContents->TweakGuiColors);
  gpTweakParticle = rs_new CTweakParticle(gpTweakContents->TweakParticle);
  gpTweakPlayerB = rs_new CTweakPlayer(gpTweakContents->TweakPlayer2);
  gpTweakPlayerA = rs_new CTweakPlayer(gpTweakContents->TweakPlayer);
  gpTweakPlayerControlsB = rs_new CTweakPlayerControls(gpTweakContents->TweakPlayerControls2);
  gpTweakPlayerControlsA = rs_new CTweakPlayerControls(gpTweakContents->TweakPlayerControls);
  gpTweakPlayerGunMulti = rs_new CTweakPlayerGun(gpTweakContents->TweakPlayerGunMuli);
  gpTweakPlayerGunSingle = rs_new CTweakPlayerGun(gpTweakContents->TweakPlayerGun);
  gpTweakPlayerRes = rs_new CTweakPlayerRes(gpTweakContents->TweakPlayerRes);
  gpTweakSlideShow = rs_new CTweakSlideShow(gpTweakContents->TweakSlideShow);
  gpTweakTargeting = rs_new CTweakTargeting(gpTweakContents->TweakTargeting);

  gpTweakPlayerGun = gpTweakPlayerGunSingle.get();
  CPlayerCameraBob::ReadTweaks(gpTweakContents->TweakCameraBob);
  CMappableObject::ReadAutomapperTweaks();
}

void REL_FreeTweaks() {
  delete gpTweakContents;
  gpTweakContents = nullptr;
  gpTweakAutoMapper = nullptr;
  gpTweakBall = nullptr;
  gpTweakGame = nullptr;
  gpTweakGui = nullptr;
  gpTweakGuiColors = nullptr;
  gpTweakParticle = nullptr;
  gpTweakPlayerB = nullptr;
  gpTweakPlayerA = nullptr;
  gpTweakPlayerControlsB = nullptr;
  gpTweakPlayerControlsA = nullptr;
  gpTweakPlayerGunMulti = nullptr;
  gpTweakPlayerGunSingle = nullptr;
  gpTweakPlayerRes = nullptr;
  gpTweakSlideShow = nullptr;
  gpTweakTargeting = nullptr;
}

void TweaksInit() {
  REL_loader_Tweaks.Loader = REL_LoadTweaks;
  REL_loader_Tweaks.CreateGlobals = REL_CreateTweakGlobals;
  REL_loader_Tweaks.FreeTweaks = REL_FreeTweaks;
  SetTweaks_FuncPtrs(&REL_loader_Tweaks);
}

extern "C" void RELMain() { TweaksInit(); }

extern "C" void RELExit() { SetTweaks_FuncPtrs(nullptr); }
