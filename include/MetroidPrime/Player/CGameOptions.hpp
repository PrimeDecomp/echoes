#ifndef _CGAMEOPTIONS
#define _CGAMEOPTIONS

#include "types.h"

#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/Player/CPlayerOptions.hpp"
#include "MetroidPrime/TGameTypes.hpp"

class CBitStreamReader;
class CBitStreamWriter;

class CGameOptions {
public:
  CGameOptions();
  CGameOptions(CBitStreamReader& in);
  ~CGameOptions();

  void PutTo(CBitStreamWriter&);

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
  uint GetSfxVolume() const { return sfxVol; }
  void SetMusicVolume(int, bool);
  uint GetMusicVolume() const { return musicVol; }
  void SetSurroundMode(CAudioSys::ESurroundModes, bool);

  int GetHudAlphaRaw() const;
  float GetHudAlpha() const;
  void SetHudAlpha(int);
  int GetHelmetAlphaRaw() const;
  float GetHelmetAlpha() const;
  void SetHelmetAlpha(int);

  void SetHUDLag(bool);
  bool GetHUDLag() const { return hudLag; }
  void SetIsHintSystemEnabled(bool);
  bool GetIsHintSystemEnabled() const { return hintSystem; }
  void SetIsHudEnglish(bool);
  bool GetIsHudEnglish() const { return hudEnglish; } // Guessed name; selects STRG_HudEngOnly.
  void SetInvertYAxis(bool);
  bool GetInvertYAxis() const { return invertY; }
  void SetIsRumbleEnabled(bool rumble);
  bool GetIsRumbleEnabled() const { return rumble; }
  // Guessed name
  bool GetIsPlayerRumbleEnabled(int player) const {
    return mPlayerOptions[player].GetRumbleEnabled();
  }
  CPlayerOptions& PlayerOptions(int player) { return mPlayerOptions[player]; } // Guessed name
  void ToggleControls(bool);
  bool GetSwapBeamControls() const { return swapBeamsControls; }
  const rstl::vector< rstl::pair< CAssetId, CAssetId > >& GetControlTXTRMap() const {
    return mControlTXTRMap;
  }

  void ResetControllerAssets(int);
  void SetControls(int);

  static bool fn_80161C84();
  static void fn_80161C7C(bool);

private:
  friend class CScanTreeMenu;
  friend class CScanTreeSlider;

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
  bool hudEnglish : 1;
  rstl::vector< rstl::pair< CAssetId, CAssetId > > mControlTXTRMap;
  rstl::reserved_vector< CPlayerOptions, 4 > mPlayerOptions;
};
CHECK_SIZEOF(CGameOptions, 0x44)

#endif // _CGAMEOPTIONS
