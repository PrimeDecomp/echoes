#ifndef _WEAPONTYPES
#define _WEAPONTYPES

// TODO: Echoes stuff

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
  EWeaponType x0_weaponType;
  bool x4_24_charged : 1;
  bool x4_25_comboed : 1;
  bool x4_26_instantKill : 1;

public:
  explicit CWeaponMode(EWeaponType type = kWT_None, const bool charged = false,
                       const bool comboed = false, const bool instaKill = false)
  : x0_weaponType(type)
  , x4_24_charged(charged)
  , x4_25_comboed(comboed)
  , x4_26_instantKill(instaKill) {}

  CWeaponMode(int type, int flags) : x0_weaponType(EWeaponType(type)) {}

  EWeaponType GetType() const { return x0_weaponType; }

  bool IsCharged() const { return x4_24_charged; }
  bool IsComboed() const { return x4_25_comboed; }
  bool IsInstantKill() const { return x4_26_instantKill; }
};

#endif // _WEAPONTYPES
