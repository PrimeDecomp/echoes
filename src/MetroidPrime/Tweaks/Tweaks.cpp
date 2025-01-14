
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "REL/REL_Setup.h"

#include "MetroidPrime/Tweaks/CTweakContents.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"

#include "dolphin/types.h"

#include "MetroidPrime/ScriptLoader/SLdrTDamageInfo.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTWeaponDamage.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTBeamInfo.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTIcon_Configurations.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTGunResources.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTBallTransitionResources.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_DarkWorld.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakBall_Misc.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakBall_Movement.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGui_ScannableObjectDownloadTimes.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerControls_UnknownStruct2.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerControls_UnknownStruct1.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun_Holstering.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun_Misc.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun_Position.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun_RicochetDamage_Factor.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerRes_AutoMapperIcons.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerRes_MapScreenIcons.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_AimStuff.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_Collision.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_Frozen.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_GrappleBeam.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_Misc.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_Motion.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_Orbit.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_ScanVisor.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_Shield.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_SuitDamageReduction.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakAutoMapper_Base.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakAutoMapper_DoorColors.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakBall_BoostBall.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakBall_Camera.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakBall_DeathBall.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakBall_CannonBall.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakBall_ScrewAttack.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGuiColors_HUDColorsTypedef.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGuiColors_Misc.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGuiColors_Multiplayer.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGui_Completion.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGuiColors_TurretHudTypedef.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGui_Credits.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGui_DarkVisor.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGui_EchoVisor.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGui_VisorColorSchemeTypedef.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGui_HudColorTypedef.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGame_FragLimitChoices.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGame_CoinLimitChoices.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGame_TimeLimitChoices.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGui_LogBook.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGui_MovieVolumes.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGui_Misc.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGui_ScanVisor.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun_Arm_Position.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun_UnknownStruct1.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun_Beam_Misc.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_FirstPersonCamera.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_Grapple.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakTargeting_Charge_Gauge.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakTargeting_LockDagger.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakTargeting_LockFire.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakTargeting_OuterBeamIcon.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakTargeting_Scan.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakTargeting_VulnerabilityIndicator.cpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun_Weapons.cpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakGui.cpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakTargeting.cpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakPlayerRes.cpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakParticle.cpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakGuiColors.cpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakGame.cpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakSlideShow.cpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakBall.cpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakAutoMapper.cpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakPlayerControls.cpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakPlayerGun.cpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakCameraBob.cpp"
#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakPlayer.cpp"

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

void REL_LoadTweaks(CInputStream& input) {
  if ((uint)input.ReadInt32() == 0x4e54574b && input.ReadInt8() == 1) {
    gpTweakContents = new CTweakContents();

    for (int instanceCount = input.ReadInt32(); instanceCount != 0; --instanceCount) {
      uint instanceType = (uint)input.ReadInt32();
      u16 instanceSize = input.ReadUint16();
      input.ReadInt32(); // skip instance id
      instanceSize -= 6;

      for (int connectionCount = input.ReadInt32(); connectionCount != 0; connectionCount--) {
        instanceSize -= 0xc;
        input.ReadInt32();
        input.ReadInt32();
        input.ReadInt32();
      }

      // Record current position of input
      DecodeAnyTweak(instanceType, input);
      // Read instanceSize - (current position - old position)
    }
  }
}

void REL_CreateTweakGlobals() {}

void REL_FreeTweaks() {
  delete gpTweakGame;
  gpTweakGame = nullptr;
}

void TweaksInit() {
  REL_loader_Tweaks.Loader = REL_LoadTweaks;
  REL_loader_Tweaks.CreateGlobals = REL_CreateTweakGlobals;
  REL_loader_Tweaks.FreeTweaks = REL_FreeTweaks;
  SetTweaks_FuncPtrs(&REL_loader_Tweaks);
}

extern "C" void RELMain() { TweaksInit(); }

extern "C" void RELExit() { SetTweaks_FuncPtrs(nullptr); }
