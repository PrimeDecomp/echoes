#ifndef _CTWEAKPLAYERGUN
#define _CTWEAKPLAYERGUN

#include "MetroidPrime/CDamageInfo.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

struct SWeaponInfo {
  float m_coolDown;
  CDamageInfo m_normal;
  CDamageInfo m_charged;
};

class CCameraShakerData;
struct SLdrTweakPlayerGun;

class CTweakPlayerGun {
public:
  explicit CTweakPlayerGun(const SLdrTweakPlayerGun& data) : mData(&data) { InitBeamInfo(); }

  float GetGunTransformTime() const;
  float GetHoloHoldTime() const;
  float GetGunExtendDistance() const;
  int GetMaxAbsorbedPhazonShots();
  const SWeaponInfo& GetBeamInfo(int beam) const;
  CCameraShakerData GetCameraShakerData6() const; // Guessed name: sixth shaker preset.

private:
  void InitBeamInfo(); // Guessed name

  const SLdrTweakPlayerGun* mData;
  rstl::reserved_vector< SWeaponInfo, 4 > mBeamInfo;
};
CHECK_SIZEOF(CTweakPlayerGun, 0xf8)

extern CTweakPlayerGun* gpTweakPlayerGun;
extern rstl::single_ptr< CTweakPlayerGun > gpTweakPlayerGunMulti;
extern rstl::single_ptr< CTweakPlayerGun > gpTweakPlayerGunSingle;

#endif // _CTWEAKPLAYERGUN
