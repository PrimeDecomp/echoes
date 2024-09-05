#ifndef _CGAMEOPTIONS
#define _CGAMEOPTIONS

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
  void ResetExtraFlagsToDefaults();
  void ResetScreenToDefaults();
  void ResetSoundToDefaults();
  void ResetVisorToDefaults();
  void EnsureOptions();

  void SetScreenBrightness(int, bool);
  float TuneScreenBrightness();
  void SetScreenPositionX(int, bool);
  void SetScreenPositionY(int, bool);
  void SetScreenStretch(int, bool);
  void SetSfxVolume(int, bool);
  void SetMusicVolume(int, bool);
  void SetSurroundMode(CAudioSys::ESurroundModes, bool);

  int GetHudAlphaRaw() const;
  float GetHudAlpha() const;
  void SetHudAlpha(int);
  int GetHelmetAlphaRaw() const;
  float GetHelmetAlpha() const;
  void SetHelmetAlpha(int);

  void SetHUDLag(bool);
  void SetIsHintSystemEnabled(bool);
  void SetFlag3(bool);
  void SetInvertYAxis(bool);
  void SetIsRumbleEnabled(bool rumble);
  void ToggleControls(bool);

  void ResetControllerAssets(int);
  void SetControls(int);

  static bool fn_80161C84();
  static void fn_80161C7C(bool);

private:
  CAudioSys::ESurroundModes soundMode;
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
  bool unk : 1;
  rstl::vector<SObjectTag> vec;
  rstl::reserved_vector<rstl::pair<bool, bool>, 4> unk2;
};
CHECK_SIZEOF(CGameOptions, 0x44)

#endif // _CGAMEOPTIONS
