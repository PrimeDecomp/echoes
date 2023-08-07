#ifndef _CDAMAGEINFO
#define _CDAMAGEINFO

#include "types.h"

struct CDamageInfo {
  ushort weaponMode1;
  ushort weaponMode2;
  float damage;
  float radiusDamageAmount;
  float damageRadius;
  float knockbackPower;
  short x14;
  short x16;
  short x18;
  bool noImmunity;
};

#endif // _CDAMAGEINFO
