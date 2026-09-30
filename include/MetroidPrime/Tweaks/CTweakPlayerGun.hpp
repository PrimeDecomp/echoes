#ifndef _CTWEAKPLAYERGUN
#define _CTWEAKPLAYERGUN

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

struct SWeaponInfo {
  SWeaponInfo(float coolDown, const CDamageInfo& normal, const CDamageInfo& charged);

  float mCoolDown;
  CDamageInfo mNormal;
  CDamageInfo mCharged;
};
CHECK_SIZEOF(SWeaponInfo, 0x3c)

class CCameraShakerData;
struct SLdrTweakPlayerGun;

class CTweakPlayerGun {
public:
  explicit CTweakPlayerGun(const SLdrTweakPlayerGun& data) : mData(&data) { BuildCache(); }

  CCameraShakerData GetProjectileImpactCameraShakerData() const;
  CCameraShakerData GetProjectileRecoilCameraShakerData() const;
  CCameraShakerData GetRecoilCameraShakerData() const;
  CDamageInfo GetComboDamage(CPlayerState::EBeamId beam) const;
  float GetGunTransformTime() const;
  float GetHoloHoldTime() const;
  float GetBombDropDelayTime() const;
  float GetBombTriggerRadius() const; // Guessed name.
  float GetFixedVerticalAim() const;
  float GetGunNotFiringTime() const;
  float GetGunHolsterTime() const;
  CVector3f GetGrapplingArmPosition() const;
  CVector3f GetGunPosition() const;
  float GetGunExtendDistance() const; // Guessed name.
  float GetPhazonShotAbsorbRadius() const;
  int GetMaxAbsorbedPhazonShots() const;
  float GetPlayerBurnDamage() const;
  float GetAIBurnDamage() const;
  CDamageInfo GetImploderDamage() const;
  CDamageInfo GetSunBurstRaysDamage() const;
  CDamageInfo GetBlackHoleDamage() const;
  CDamageInfo GetPowerBombInfo() const;
  CDamageInfo GetBombInfo() const;
  CDamageInfo GetMissileDamage() const;
  SWeaponInfo GetPhazonBeamInfo() const;
  CDamageInfo GetDarkBeamBlobDamage() const;
  const SWeaponInfo& GetBeamInfo(CPlayerState::EBeamId beam) const;

private:
  void BuildCache();

  const SLdrTweakPlayerGun* mData;
  rstl::reserved_vector< SWeaponInfo, 4 > mBeamInfo;
};
CHECK_SIZEOF(CTweakPlayerGun, 0xf8)

extern CTweakPlayerGun* gpTweakPlayerGun;
extern rstl::single_ptr< CTweakPlayerGun > gpTweakPlayerGunMulti;
extern rstl::single_ptr< CTweakPlayerGun > gpTweakPlayerGunSingle;

#endif // _CTWEAKPLAYERGUN
