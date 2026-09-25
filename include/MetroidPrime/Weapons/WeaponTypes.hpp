#ifndef _WEAPONTYPES
#define _WEAPONTYPES

#include "types.h"

enum EWeaponType {
  kWT_None = -1,
  kWT_Power = 0,
  kWT_Dark = 1,
  kWT_Light = 2,
  kWT_Annihilator = 3,
  kWT_Bomb = 4,
  kWT_PowerBomb = 5,
  kWT_Missile = 6,
  kWT_BoostBall = 7,
  kWT_CannonBall = 8,
  kWT_ScrewAttack = 9,
  kWT_Phazon = 10,
  kWT_AI = 11,
  kWT_PoisonWater1 = 12,
  kWT_PoisonWater2 = 13,
  kWT_Lava = 14,
  kWT_Heat = 15,
  kWT_Unused1 = 16,
  kWT_AreaDark = 17,
  kWT_AreaLight = 18,
  kWT_UnknownSource = 19,
  kWT_SafeZone = 20,
  kWT_Max = 21
};

class CWeaponMode {
  uint mWeaponType : 16;
  uint mCharged : 1;
  uint mComboed : 1;
  uint mInstantKill : 1;

public:
  explicit CWeaponMode(EWeaponType type = kWT_None, const bool charged = false,
                       const bool comboed = false, const bool instaKill = false)
  : mWeaponType(uint(type))
  , mCharged(charged)
  , mComboed(comboed)
  , mInstantKill(instaKill) {}

  CWeaponMode(int type, int flags) : mWeaponType(uint(type)) {}

  EWeaponType GetType() const { return EWeaponType(short(mWeaponType)); }
  ushort GetRawType() const { return ushort(mWeaponType); }

  bool IsCharged() const { return mCharged; }
  bool IsComboed() const { return mComboed; }
  bool IsInstantKill() const { return mInstantKill; }
};
CHECK_SIZEOF(CWeaponMode, 0x4)

#endif // _WEAPONTYPES
