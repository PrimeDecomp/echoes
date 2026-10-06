#include "MetroidPrime/CHealthInfo.hpp"

CHealthInfo::CHealthInfo(float hp, float resist)
: mInitialHealth(hp)
, mHealth(hp)
, mKnockbackResistance(resist)
, mDeathWeaponMode(CWeaponMode(kWT_None))
, mDeathDamageOwner(kInvalidUniqueId)
, mDeathDamageSource(kInvalidUniqueId)
, mLastDamageWeaponMode(CWeaponMode(kWT_None))
, mLastDamageOwner(kInvalidUniqueId)
, mLastDamageSource(kInvalidUniqueId)
, mFrozenAtDeath(false)
, mLastDamageWasRadius(false) {}

void CHealthInfo::SetCauseOfDeathWeapon(const CWeaponMode mode, const TUniqueId owner,
                                        const TUniqueId source, const bool frozen,
                                        const bool radiusDamage) {
  mDeathWeaponMode = mode;
  mDeathDamageOwner = owner;
  mDeathDamageSource = source;
  mFrozenAtDeath = frozen;
  mLastDamageWasRadius = radiusDamage;
}

void CHealthInfo::SetLastDamageWeapon(const CWeaponMode mode, const TUniqueId owner,
                                      const TUniqueId source, const bool radiusDamage) {
  mLastDamageWeaponMode = mode;
  mLastDamageOwner = owner;
  mLastDamageSource = source;
  mLastDamageWasRadius = radiusDamage;
}
