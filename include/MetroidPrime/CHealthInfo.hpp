#ifndef _CHEALTHINFO
#define _CHEALTHINFO

#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"

class CHealthInfo {
public:
  CHealthInfo(float hp, float resist);

  void SetHP(float hp) { mHealth = hp; }

  void SetKnockbackResistance(float resist) { mKnockbackResistance = resist; }

  float GetKnockBackResistance() const { return mKnockbackResistance; }

  float GetHP() const { return mHealth; }

  float GetInitialHP() const { return mInitialHealth; } // Guessed name.

  TUniqueId GetDamageId1() const { return mDeathDamageOwner; } // Guessed name.

  TUniqueId GetDamageId2() const { return mDeathDamageSource; } // Guessed name.

  TUniqueId GetLastDamageSource() const { return mLastDamageSource; } // Guessed name.

  const CWeaponMode& GetCauseOfDeathWeapon() const { return mDeathWeaponMode; } // Guessed name.

  bool GetDamageFlag() const { return mFrozenAtDeath; } // Guessed name; victim was frozen.

  // SetCauseOfDeathWeapon is an original Wii export; parameter names are guessed.
  void SetCauseOfDeathWeapon(CWeaponMode mode, TUniqueId owner, TUniqueId source, const bool frozen,
                             bool radiusDamage);
  // Guessed name; native damage attribution distinguishes weapon owner and source.
  void SetLastDamageWeapon(CWeaponMode mode, TUniqueId owner, TUniqueId source, bool radiusDamage);

private:
  // Guessed names, corroborated by construction and native damage/death consumers.
  float mInitialHealth;
  float mHealth;
  float mKnockbackResistance;
  CWeaponMode mDeathWeaponMode;
  TUniqueId mDeathDamageOwner;
  TUniqueId mDeathDamageSource;
  CWeaponMode mLastDamageWeaponMode;
  TUniqueId mLastDamageOwner;
  TUniqueId mLastDamageSource;
  bool mFrozenAtDeath : 1;
  bool mLastDamageWasRadius : 1;
};
CHECK_SIZEOF(CHealthInfo, 0x20)

#endif // _CHEALTHINFO
