#ifndef _CTWEAKPLAYERGUN
#define _CTWEAKPLAYERGUN

#include "MetroidPrime/CDamageInfo.hpp"

struct SWeaponInfo {
  float m_coolDown;
  CDamageInfo m_normal;
  CDamageInfo m_charged;
};

class CCameraShakerData;

class CTweakPlayerGun {
public:
  float GetGunTransformTime() const;
  float GetHoloHoldTime() const;
  float GetGunExtendDistance() const;
  int GetMaxAbsorbedPhazonShots();
  const SWeaponInfo& GetBeamInfo(int beam) const;
  CCameraShakerData GetCameraShakerData6() const; // Guessed name: sixth shaker preset.
};

extern CTweakPlayerGun* gpTweakPlayerGun;
extern CTweakPlayerGun* gpTweakPlayerGunMulti;
extern CTweakPlayerGun* gpTweakPlayerGunSingle;

#endif // _CTWEAKPLAYERGUN
