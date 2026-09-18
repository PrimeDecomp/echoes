#ifndef _CTWEAKPLAYERGUN
#define _CTWEAKPLAYERGUN

#include "MetroidPrime/CDamageInfo.hpp"

struct SWeaponInfo {
  float m_coolDown;
  CDamageInfo m_normal;
  CDamageInfo m_charged;
};

class CTweakPlayerGun {
public:
  int GetMaxAbsorbedPhazonShots();
};

extern CTweakPlayerGun* gpTweakPlayerGun;
extern CTweakPlayerGun* gpTweakPlayerGunMulti;
extern CTweakPlayerGun* gpTweakPlayerGunSingle;

#endif // _CTWEAKPLAYERGUN
