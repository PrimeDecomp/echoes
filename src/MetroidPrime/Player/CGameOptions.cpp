#include "MetroidPrime/Player/CGameOptions.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CMoviePlayer.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Streams/CBitStreamReader.hpp"
#include "Kyoto/Streams/CBitStreamWriter.hpp"
#include "rstl/algorithm.hpp"

#include "dolphin/os.h"

extern "C" bool lbl_804191E0;

// Guessed names; the original stored these as individual small-data words.
static CAssetId skControlTXTR0A = 0x2A13C23E;
static CAssetId skControlTXTR0B = 0xF13452F8;
static CAssetId skControlTXTR1A = 0xA91A7703;
static CAssetId skControlTXTR1B = 0xC042EC91;
static CAssetId skControlTXTR2A = 0x12A12131;
static CAssetId skControlTXTR2B = 0x5F556002;
static CAssetId skControlTXTR3A = 0xA9798329;
static CAssetId skControlTXTR3B = 0xB306E26F;
static CAssetId skControlTXTR4A = 0xCD7B1ACA;
static CAssetId skControlTXTR4B = 0x8ADA8184;
static CAssetId skControlTXTR5A = 0x1A29C0E6;
static CAssetId skControlTXTR5B = 0xF13452F8;
static CAssetId skControlTXTR6A = 0x5D9F9796;
static CAssetId skControlTXTR6B = 0xC042EC91;
static CAssetId skControlTXTR7A = 0x951546A8;
static CAssetId skControlTXTR7B = 0x5F556002;
static CAssetId skControlTXTR8A = 0x7946C4C5;
static CAssetId skControlTXTR8B = 0xB306E26F;
static CAssetId skControlTXTR9A = 0x409AA72E;
static CAssetId skControlTXTR9B = 0x8ADA8184;

int CGameOptions_CalculateBits(uint v) {
  int iVar1;

  iVar1 = 0;
  for (; v != 0; v >>= 1) {
    iVar1 += 1;
  }
  return iVar1;
}

inline void WritePackedBits(CBitStreamWriter& out, uint val, uint m) {
  out.WriteBits(val, CGameOptions_CalculateBits(m));
}

void CGameOptions::InitSoundMode() {
  if (OSGetSoundMode() == 0) {
    soundMode = CAudioSys::kSM_Mono;
  } else {
    CAudioSys::ESurroundModes mode = soundMode;
    soundMode = (mode != CAudioSys::kSM_Mono) ? mode : CAudioSys::kSM_Stereo;
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
, hudEnglish(false)
, mPlayerOptions(CPlayerOptions())

{
  InitSoundMode();
}

CGameOptions::CGameOptions(CBitStreamReader& in)

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
, hudEnglish(false)
, mControlTXTRMap() {
  in.ReadBits(32);
  soundMode = (CAudioSys::ESurroundModes)in.ReadBits(CGameOptions_CalculateBits(2));
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
  hudEnglish = in.ReadBits(1);

  for (int i = 0; i < 4; ++i) {
    mPlayerOptions.push_back(CPlayerOptions(in));
  }

  InitSoundMode();
}

void CGameOptions::PutTo(CBitStreamWriter& out) {
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
  out.WriteBits(hudEnglish != 0, 1);

  int i = 0;
  CPlayerOptions* data = mPlayerOptions.data();
  for (; i < 4; ++i) {
    data->PutTo(out);
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
  hudEnglish = false;
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
  hudEnglish = false;
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
  SetIsHudEnglish(hudEnglish);
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
      CSfxManager::SetAreaVolume(0, sfxVol);
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

void CGameOptions::SetIsHudEnglish(bool active) { hudEnglish = active; }

void CGameOptions::SetInvertYAxis(bool active) { invertY = active; }

void CGameOptions::SetIsRumbleEnabled(bool active) { rumble = active; }

void CGameOptions::ToggleControls(const bool flag) {
  swapBeamsControls = flag;
  if (flag) {
    SetControls(1);
  } else {
    SetControls(0);
  }
}

void CGameOptions::ResetControllerAssets(int controls) {
  switch (controls) {
  case 0:
    mControlTXTRMap = rstl::vector< rstl::pair< CAssetId, CAssetId > >();
    break;
  case 1:
    if (mControlTXTRMap.empty()) {
      const rstl::pair< CAssetId, CAssetId > stickRemap[5] = {
          rstl::pair< CAssetId, CAssetId >(skControlTXTR0A, skControlTXTR0B),
          rstl::pair< CAssetId, CAssetId >(skControlTXTR1A, skControlTXTR1B),
          rstl::pair< CAssetId, CAssetId >(skControlTXTR2A, skControlTXTR2B),
          rstl::pair< CAssetId, CAssetId >(skControlTXTR3A, skControlTXTR3B),
          rstl::pair< CAssetId, CAssetId >(skControlTXTR4A, skControlTXTR4B),
      };
      const rstl::pair< CAssetId, CAssetId > outlineRemap[5] = {
          rstl::pair< CAssetId, CAssetId >(skControlTXTR5A, skControlTXTR5B),
          rstl::pair< CAssetId, CAssetId >(skControlTXTR6A, skControlTXTR6B),
          rstl::pair< CAssetId, CAssetId >(skControlTXTR7A, skControlTXTR7B),
          rstl::pair< CAssetId, CAssetId >(skControlTXTR8A, skControlTXTR8B),
          rstl::pair< CAssetId, CAssetId >(skControlTXTR9A, skControlTXTR9B),
      };
      mControlTXTRMap.reserve(15);
      for (int i = 0; i < 5; ++i) {
        const rstl::pair< CAssetId, CAssetId > entry = stickRemap[i];
        mControlTXTRMap.push_back_unsafe(entry);
        mControlTXTRMap.push_back_unsafe(
            rstl::pair< CAssetId, CAssetId >(entry.second, entry.first));
      }
      for (int i = 0; i < 5; ++i) {
        mControlTXTRMap.push_back_unsafe(outlineRemap[i]);
      }
      rstl::sort(
          mControlTXTRMap.begin(), mControlTXTRMap.end(),
          rstl::pair_sorter_finder< rstl::pair< CAssetId, CAssetId >, rstl::less< CAssetId > >(
              rstl::less< CAssetId >()));
    }
    break;
  }
}

void CGameOptions::SetControls(int controls) { ResetControllerAssets(controls); }
