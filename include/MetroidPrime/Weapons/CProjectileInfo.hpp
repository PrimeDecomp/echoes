#ifndef _CPROJECTILEINFO
#define _CPROJECTILEINFO

#include "Kyoto/TToken.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/CDamageInfo.hpp"

class CPlayer;
class CWeaponDescription;

class CProjectileInfo {
public:
  CProjectileInfo(CAssetId projectile, const CDamageInfo& damage);

  float GetProjectileSpeed() const;
  CVector3f PredictInterceptPos(const CVector3f& gunPos, const CVector3f& aimPos,
                                const CPlayer& player, bool gravity, float dt);
  static CVector3f PredictInterceptPos(const CVector3f& gunPos, const CVector3f& aimPos,
                                       const CPlayer& player, bool gravity, float speed, float dt);

  TCachedToken< CWeaponDescription >& Token() { return mWeaponDescription; }
  const CDamageInfo& GetDamage() const { return mDamageInfo; }

private:
  TCachedToken< CWeaponDescription > mWeaponDescription;
  CDamageInfo mDamageInfo;
};
CHECK_SIZEOF(CProjectileInfo, 0x28)

#endif // _CPROJECTILEINFO
