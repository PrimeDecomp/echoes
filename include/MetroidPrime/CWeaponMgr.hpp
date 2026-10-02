#ifndef _CWEAPONMGR
#define _CWEAPONMGR

#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"

#include "rstl/map.hpp"
#include "rstl/reserved_vector.hpp"

class CWeaponMgr {
public:
  typedef rstl::reserved_vector< int, kWT_Max > Vec;

  CWeaponMgr();
  void Remove(TUniqueId uid);
  void IncrCount(TUniqueId uid, EWeaponType type);
  void DecrCount(TUniqueId uid, EWeaponType type);
  int GetNumActive(TUniqueId uid, EWeaponType type) const;

  void Add(TUniqueId uid, EWeaponType type);
  Vec* GetIndex(TUniqueId uid) const;

private:
  rstl::map< TUniqueId, Vec > mWeapons;
};
CHECK_SIZEOF(CWeaponMgr, 0x14);

#endif // _CWEAPONMGR
