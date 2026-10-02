#include "MetroidPrime/CWeaponMgr.hpp"

CWeaponMgr::CWeaponMgr() {}

void CWeaponMgr::Add(TUniqueId uid, EWeaponType type) {
  rstl::pair< TUniqueId, Vec > newIndex(uid, Vec(0));
  newIndex.second[type] += 1;
  mWeapons.insert(newIndex);
}

void CWeaponMgr::Remove(TUniqueId uid) {
  rstl::map< TUniqueId, Vec >::iterator iter = mWeapons.find(uid);
  if (iter != mWeapons.end()) {
    mWeapons.erase(iter);
  }
}

void CWeaponMgr::IncrCount(TUniqueId uid, EWeaponType type) {
  Vec* vec = GetIndex(uid);
  if (vec == nullptr) {
    Add(uid, type);
  } else {
    (*vec)[type]++;
  }
}

void CWeaponMgr::DecrCount(TUniqueId uid, EWeaponType type) {
  Vec* vecP = GetIndex(uid);
  if (!vecP) {
    return;
  }

  Vec& vec = *vecP;
  vec[type]--;

  bool empty = true;
  Vec::iterator it = vec.begin(), end = vec.end();
  for (; it != end; ++it) {
    if (*it > 0) {
      empty = false;
      break;
    }
  }
  if (empty) {
    Remove(uid);
  }
}

int CWeaponMgr::GetNumActive(TUniqueId uid, EWeaponType type) const {
  Vec* vec = GetIndex(uid);
  if (vec) {
    return (*vec)[type];
  } else {
    return 0;
  }
}

CWeaponMgr::Vec* CWeaponMgr::GetIndex(TUniqueId uid) const {
  rstl::map< TUniqueId, Vec >::const_iterator iter = mWeapons.find(uid);
  if (iter != mWeapons.end()) {
    return const_cast< Vec* >(&iter->second);
  }
  return nullptr;
}
