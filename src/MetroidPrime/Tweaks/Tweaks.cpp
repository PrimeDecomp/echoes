
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "REL/REL_Setup.h"

#include "MetroidPrime/Tweaks/CTweakContents.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"

#include "dolphin/types.h"

STweaks_FuncPtrs REL_loader_Tweaks;

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
