#include "MetroidPrime/Player/CGameOptions.hpp"

#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CMoviePlayer.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Streams/CMemoryStreamOut.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#include "dolphin/os.h"

extern "C" void fn_8029AF00(int, uchar);
extern "C" rstl::pair<bool, bool> fn_80227694();
extern "C" void fn_802275B8(rstl::pair<bool, bool>&, CMemoryStreamOut& out);
extern "C" rstl::pair<bool, bool> fn_80227624(CInputStream& in);

extern "C" bool lbl_804191E0;

int CGameOptions_CalculateBits(uint v) {
  int iVar1;

  iVar1 = 0;
  for (; v != 0; v >>= 1) {
    iVar1 += 1;
  }
  return iVar1;
}

inline void WritePackedBits(CMemoryStreamOut& out, uint val, uint m) {
  out.WriteBits(val, CGameOptions_CalculateBits(m));
}

void CGameOptions::InitSoundMode() {
  if (OSGetSoundMode() == 0) {
    soundMode = CAudioSys::kSM_Mono;
  } else {
    soundMode = (soundMode != CAudioSys::kSM_Mono) ? soundMode : CAudioSys::kSM_Stereo;
  }
}

bool CGameOptions::fn_80161C84() { return lbl_804191E0; }

void CGameOptions::fn_80161C7C(bool x) { lbl_804191E0 = x; }

CGameOptions::CGameOptions()

: soundMode(CAudioSys::kSM_Stereo)
, screenBrightness(4)
, screenXOffset(0)
, screenYOffset(0)
, screenStretch(0)
, sfxVol(0x69)
, musicVol(0x4f)
, hudAlpha(0xff)
, helmetAlpha(0xff)
, hudLag(true)
, invertY(false)
, rumble(true)
, swapBeamsControls(false)
, hintSystem(true)
, unk(false)
, unk2(fn_80227694())

{
  InitSoundMode();
}

CGameOptions::CGameOptions(CInputStream& in) 

: soundMode(CAudioSys::kSM_Stereo)
, screenBrightness(4)
, screenXOffset(0)
, screenYOffset(0)
, screenStretch(0)
, sfxVol(0x69)
, musicVol(0x4f)
, hudAlpha(0xff)
, helmetAlpha(0xff)
, hudLag(true)
, invertY(false)
, rumble(true)
, swapBeamsControls(false)
, hintSystem(true)
, unk(false)
, vec()
{
  in.ReadBits(32);
  soundMode = (CAudioSys::ESurroundModes) in.ReadBits(CGameOptions_CalculateBits(2));
  screenBrightness = in.ReadBits(CGameOptions_CalculateBits(8));
  screenXOffset = in.ReadBits(CGameOptions_CalculateBits(60)) - 30;
  screenYOffset = in.ReadBits(CGameOptions_CalculateBits(60)) - 30;
  screenYOffset = (screenYOffset < -19 ? -19 : (screenYOffset > 19 ? 19 : screenYOffset));
  screenStretch = in.ReadBits(CGameOptions_CalculateBits(20)) - 10;
  sfxVol = in.ReadBits(CGameOptions_CalculateBits(0x69));
  musicVol = in.ReadBits(CGameOptions_CalculateBits(0x69));
  hudAlpha = in.ReadBits(CGameOptions_CalculateBits(0xff));
  helmetAlpha = in.ReadBits(CGameOptions_CalculateBits(0xff));
  
  hudLag = in.ReadBits(1);
  hintSystem = in.ReadBits(1);
  invertY = in.ReadBits(1);
  rumble = in.ReadBits(1);
  swapBeamsControls = in.ReadBits(1);
  unk = in.ReadBits(1);
  
  for (int i = 0; i < 4; ++i) {
    unk2.push_back(fn_80227624(in));
  }

  InitSoundMode();
}

void CGameOptions::PutTo(CMemoryStreamOut& out) {
  out.WriteBits(0x4f50544e, 32);
  WritePackedBits(out, soundMode, 2);
  WritePackedBits(out, screenBrightness, 8);
  WritePackedBits(out, screenXOffset + 30, 60);
  WritePackedBits(out, screenYOffset + 30, 60);
  WritePackedBits(out, screenStretch + 10, 20);
  WritePackedBits(out, sfxVol, 0x69);
  WritePackedBits(out, musicVol, 0x69);
  WritePackedBits(out, hudAlpha, 0xff);
  WritePackedBits(out, helmetAlpha, 0xff);
  out.WriteBits(hudLag != 0, 1);
  out.WriteBits(hintSystem != 0, 1);
  out.WriteBits(invertY != 0, 1);
  out.WriteBits(rumble != 0, 1);
  out.WriteBits(swapBeamsControls != 0, 1);
  out.WriteBits(unk != 0, 1);

  int i = 0;
  rstl::pair<bool, bool>* data = unk2.data();
  for (; i < 4; ++i) {
    fn_802275B8(*data, out);
    ++data;
  }
}

void CGameOptions::ResetToDefaults() {
  screenBrightness = 4;
  screenXOffset = 0;
  screenYOffset = 0;
  screenStretch = 0;
  sfxVol = 0x69;
  musicVol = 0x4f;
  soundMode = CAudioSys::kSM_Stereo;
  hudAlpha = 0xff;
  helmetAlpha = 0xff;
  hudLag = true;
  invertY = false;
  rumble = true;
  swapBeamsControls = false;
  hintSystem = true;
  unk = false;
  InitSoundMode();
  EnsureOptions();
}

void CGameOptions::ResetExtraFlagsToDefaults() {
  invertY = false;
  rumble = true;
  swapBeamsControls = false;
  InitSoundMode();
  EnsureOptions();
}

void CGameOptions::ResetScreenToDefaults() {

  screenBrightness = 4;
  screenXOffset = 0;
  screenYOffset = 0;
  screenStretch = 0;
  InitSoundMode();
  EnsureOptions();
}

void CGameOptions::ResetSoundToDefaults() {
  sfxVol = 0x69;
  musicVol = 0x4f;
  soundMode = CAudioSys::kSM_Stereo;
  InitSoundMode();
  EnsureOptions();
}

void CGameOptions::ResetVisorToDefaults() {
  hudAlpha = 0xff;
  helmetAlpha = 0xff;
  hudLag = true;
  hintSystem = true;
  unk = false;
  InitSoundMode();
  EnsureOptions();
}

void CGameOptions::EnsureOptions() {
  SetScreenBrightness(screenBrightness, true);
  SetScreenPositionX(screenXOffset, true);
  SetScreenPositionY(screenYOffset, true);
  SetScreenStretch(screenStretch, true);
  SetSfxVolume(sfxVol, true);
  SetMusicVolume(musicVol, true);
  SetSurroundMode(soundMode, true);
  SetHudAlpha(hudAlpha);
  SetHelmetAlpha(helmetAlpha);
  SetHUDLag(hudLag);
  SetInvertYAxis(invertY);
  SetIsRumbleEnabled(rumble);
  SetIsHintSystemEnabled(hintSystem);
  ToggleControls(swapBeamsControls);
  SetFlag3(unk);
}

void CGameOptions::SetScreenBrightness(int value, bool apply) {
  screenBrightness = CMath::ClampI(0, value, 8);
  if (apply) {
    CGraphics::SetBrightness(TuneScreenBrightness());
  }
}

float CGameOptions::TuneScreenBrightness() {
  float f = screenBrightness - 4;
  return f / 4.f * 0.375f + 1.f;
}

void CGameOptions::SetScreenPositionX(int position, bool apply) {
  screenXOffset = CMath::ClampI(-30, position, 30);
  if (apply) {
    int a, b, c;
    CGraphics::GetScreenPosition(&a, &b, &c);
    CGraphics::SetScreenPosition(a, screenXOffset, c);
  }
}

void CGameOptions::SetScreenPositionY(int position, bool apply) {
  screenYOffset = CMath::ClampI(-19, position, 19);
  if (apply) {
    int a, b, c;
    CGraphics::GetScreenPosition(&a, &b, &c);
    CGraphics::SetScreenPosition(a, b, screenYOffset);
  }
}

void CGameOptions::SetScreenStretch(int value, bool apply) {
  screenStretch = CMath::ClampI(-10, value, 10);

  if (apply) {
    int a, b, c;
    CGraphics::GetScreenPosition(&a, &b, &c);
    CGraphics::SetScreenPosition(screenStretch, b, c);
  }
}
void CGameOptions::SetSfxVolume(int value, bool apply) {
  sfxVol = CMath::ClampI(0, value, 0x69);
  if (apply) {
    if (fn_80161C84()) {
      fn_8029AF00(0, sfxVol);
    } else {
      CAudioSys::SysSetSfxVolume(sfxVol, 1, true, true);
      CStreamAudioManager::SetSfxVolume(sfxVol);
      CMoviePlayer::SetSfxVolume(sfxVol);
    }
  }
}
void CGameOptions::SetMusicVolume(int value, bool apply) {
  musicVol = CMath::ClampI(0, value, 0x69);
  if (apply) {
    CStreamAudioManager::SetMusicVolume(musicVol);
  }
}

void CGameOptions::SetSurroundMode(CAudioSys::ESurroundModes mode, bool apply) {
  soundMode = CAudioSys::ESurroundModes(CMath::ClampI(0, mode, 2));
  if (apply) {
    CAudioSys::SetSurroundMode(soundMode);
  }
}

int CGameOptions::GetHudAlphaRaw() const { return hudAlpha; }

void CGameOptions::SetHudAlpha(int alpha) { hudAlpha = alpha; }

float CGameOptions::GetHudAlpha() const { return hudAlpha * 0.003921569f; }

void CGameOptions::SetHelmetAlpha(int alpha) { helmetAlpha = alpha; }

int CGameOptions::GetHelmetAlphaRaw() const { return helmetAlpha; }

float CGameOptions::GetHelmetAlpha() const { return helmetAlpha * 0.003921569f; }

void CGameOptions::SetHUDLag(bool active) { hudLag = active; }

void CGameOptions::SetIsHintSystemEnabled(bool active) { hintSystem = active; }

void CGameOptions::SetFlag3(bool active) { unk = active; }

void CGameOptions::SetInvertYAxis(bool active) { invertY = active; }

void CGameOptions::SetIsRumbleEnabled(bool active) { rumble = active; }

void CGameOptions::ToggleControls(bool flag) {
  swapBeamsControls = flag;
  if (flag) {
    SetControls(1);
  } else {
    SetControls(0);
  }
}

void CGameOptions::ResetControllerAssets(int controls) {}

void CGameOptions::SetControls(int controls) { ResetControllerAssets(controls); }
