#ifndef _CPROJECTILEINFO
#define _CPROJECTILEINFO

#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CDamageInfo.hpp"

class CWeaponDescription;

class CProjectileInfo {
public:
  CProjectileInfo(CAssetId projectile, const CDamageInfo& damage);

  TCachedToken< CWeaponDescription >& Token() { return mWeaponDescription; }
  const CDamageInfo& GetDamage() const { return mDamageInfo; }

private:
  TCachedToken< CWeaponDescription > mWeaponDescription;
  CDamageInfo mDamageInfo;
};
CHECK_SIZEOF(CProjectileInfo, 0x28)

#endif // _CPROJECTILEINFO
