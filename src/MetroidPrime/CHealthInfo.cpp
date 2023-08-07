#include "MetroidPrime/CHealthInfo.hpp"

#include "MetroidPrime/CDamageInfo.hpp"

CHealthInfo::CHealthInfo(float hp, float resist)
: healthA(hp)
, healthB(hp)
, knockbackResistance(resist)
, unkA(0)
, unkB(0)
, uidA(kInvalidUniqueId)
, uidB(kInvalidUniqueId)
, unkC(0)
, uidC(kInvalidUniqueId)
, uidD(kInvalidUniqueId)
, flagA(false)
, flagB(false) {}

void CHealthInfo::fn_80142094(const CDamageInfo& dmgInfo, TUniqueId id1, TUniqueId id2, bool a, bool b) {
  unkA = dmgInfo.weaponMode1;
  unkB = dmgInfo.weaponMode2;
  uidA = id1;
  uidB = id2;
  flagA = a;
  flagB = b;
}

void CHealthInfo::fn_8014206C(uint* a, TUniqueId id1, TUniqueId id2, bool flag) {
  unkC = *a;
  uidC = id1;
  uidD = id2;
  flagB = flag;
}
