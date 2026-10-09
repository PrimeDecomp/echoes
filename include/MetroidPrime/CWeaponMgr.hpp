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
  typedef rstl::pair< TUniqueId, Vec > Entry;

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

// The map's nodes copy their entries bitwise (bitwise_copy<11> in CWeaponMgr's insert_into).
namespace rstl {
RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(CWeaponMgr::Entry)
RSTL_DECLARE_BITWISE_CONSTRUCTION(CWeaponMgr::Entry)
} // namespace rstl

#endif // _CWEAPONMGR
