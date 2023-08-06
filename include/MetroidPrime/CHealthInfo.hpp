#ifndef _CHEALTHINFO
#define _CHEALTHINFO

#include "types.h"

class CInputStream;
class CHealthInfo {
public:
  CHealthInfo(float hp, float resist) : healthA(hp), knockbackResistance(resist) {}
  explicit CHealthInfo(CInputStream&);

  void SetHP(float hp) { healthA = hp; }
  void SetKnockbackResistance(float resist) { knockbackResistance = resist; }
  float GetHP() const { return healthA; }

private:
  float healthA;
  float healthB;
  float knockbackResistance;
  uint unkA;
  short uidA;
  short uidB;
  uint unkB;
  short uidC;
  short uidD;
  bool unkC;
};
// CHECK_SIZEOF(CHealthInfo, 0x20)

#endif // _CHEALTHINFO
