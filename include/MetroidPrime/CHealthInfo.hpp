#ifndef _CHEALTHINFO
#define _CHEALTHINFO

#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"

class CDamageInfo;

class CHealthInfo {
public:
  CHealthInfo(float hp, float resist);

  void SetHP(float hp) { healthB = hp; }
  void SetKnockbackResistance(float resist) { knockbackResistance = resist; }
  float GetHP() const { return healthB; }

  void fn_80142094(const CDamageInfo& dmgInfo, TUniqueId, TUniqueId, bool, bool);
  void fn_8014206C(uint*, TUniqueId, TUniqueId, bool);

private:
  float healthA;
  float healthB;
  float knockbackResistance;
  ushort unkA;
  ushort unkB;
  TUniqueId uidA;
  TUniqueId uidB;
  uint unkC;
  TUniqueId uidC;
  TUniqueId uidD;
  bool flagA : 1;
  bool flagB : 1;
};
// CHECK_SIZEOF(CHealthInfo, 0x20)

#endif // _CHEALTHINFO
