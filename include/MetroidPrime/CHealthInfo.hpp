#ifndef _CHEALTHINFO
#define _CHEALTHINFO

#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"

class CHealthInfo {
public:
  CHealthInfo(float hp, float resist);

  void SetHP(float hp) { healthB = hp; }
  void SetKnockbackResistance(float resist) { knockbackResistance = resist; }

  float GetKnockBackResistance() const { return knockbackResistance; }

  float GetHP() const { return healthB; }
  float GetInitialHP() const { return healthA; } // Guessed name.
  TUniqueId GetDamageId1() const { return uidA; } // Guessed name; primary death attribution ID.
  TUniqueId GetDamageId2() const { return uidB; } // Guessed name; fallback death attribution ID.
  const CWeaponMode& GetCauseOfDeathWeapon() const { return weaponModeA; } // Guessed name.
  bool GetDamageFlag() const { return flagA; } // Guessed name; the flag's role is unresolved.

  void SetCauseOfDeathWeapon(CWeaponMode mode, TUniqueId, TUniqueId, bool, bool);
  void fn_8014206C(const CWeaponMode&, TUniqueId, TUniqueId, bool);

private:
  float healthA;
  float healthB;
  float knockbackResistance;
  CWeaponMode weaponModeA;
  TUniqueId uidA;
  TUniqueId uidB;
  CWeaponMode weaponModeB;
  TUniqueId uidC;
  TUniqueId uidD;
  bool flagA : 1;
  bool flagB : 1;
};
CHECK_SIZEOF(CHealthInfo, 0x20)

#endif // _CHEALTHINFO
