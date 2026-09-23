#include "MetroidPrime/CHealthInfo.hpp"

CHealthInfo::CHealthInfo(float hp, float resist)
: healthA(hp)
, healthB(hp)
, knockbackResistance(resist)
, weaponModeA(CWeaponMode(kWT_None))
, uidA(kInvalidUniqueId)
, uidB(kInvalidUniqueId)
, weaponModeB(CWeaponMode(kWT_None))
, uidC(kInvalidUniqueId)
, uidD(kInvalidUniqueId)
, flagA(false)
, flagB(false) {}

void CHealthInfo::SetCauseOfDeathWeapon(CWeaponMode mode, TUniqueId id1, TUniqueId id2, bool a,
                                       bool b) {
  weaponModeA = mode;
  uidA = id1;
  uidB = id2;
  flagA = a;
  flagB = b;
}

void CHealthInfo::fn_8014206C(const CWeaponMode& mode, TUniqueId id1, TUniqueId id2, bool flag) {
  weaponModeB = mode;
  uidC = id1;
  uidD = id2;
  flagB = flag;
}
