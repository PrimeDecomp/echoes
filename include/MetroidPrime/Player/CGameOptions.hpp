#ifndef _CGAMEOPTIONS_HPP
#define _CGAMEOPTIONS_HPP

#include "types.h"

#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/TGameTypes.hpp"

class CInputStream;
class CMemoryStreamOut;

class CGameOptions {
public:
  CGameOptions();
  CGameOptions(CInputStream& in);
  ~CGameOptions();

  void PutTo(CMemoryStreamOut&);

  void InitSoundMode();
  void ResetToDefaults();
  void EnsureOptions();

  void SetScreenBrightness(int, bool);
  float TuneScreenBrightness();
  void SetScreenPositionX(int, bool);
  void SetScreenPositionY(int, bool);
  void SetScreenStretch(int, bool);
  void SetSfxVolume(int, bool);
  void SetMusicVolume(int, bool);
  void SetSurroundMode(CAudioSys::ESurroundModes, bool);

  float GetHudAlpha() const;
  float GetHelmetAlpha() const;
  void SetHelmetAlpha(int);
  void SetHUDLag(bool);
  void SetIsHintSystemEnabled(bool);
  void SetInvertYAxis(bool);
  void ToggleControls(bool);
  void ResetControllerAssets(int);
  void SetControls(int);

  void SetIsRumbleEnabled(bool rumble);
  bool GetIsRumbleEnabled() const { return rumble; }

private:
  int soundMode;
  int screenBrightness;
  int screenXOffset;
  int screenYOffset;
  int screenStretch;
  uint sfxVol;
  uint musicVol;
  int hudAlpha;
  int helmetAlpha;
  bool hudLag : 1;
  bool invertY : 1;
  bool rumble : 1;
  bool swapBeamsControls : 1;
  bool hintSystem : 1;
  char pad[28];
};
CHECK_SIZEOF(CGameOptions, 0x44)

#endif // _CGAMEOPTIONS_HPP
