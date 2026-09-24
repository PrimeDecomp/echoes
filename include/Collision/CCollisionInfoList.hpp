#ifndef _CCOLLISIONINFOLIST
#define _CCOLLISIONINFOLIST

#include "Collision/CCollisionInfo.hpp"
#include "rstl/reserved_vector.hpp"

class CCollisionInfoList {
public:
  int GetCount() const { return x0_list.size(); }
  void Clear() { x0_list.clear(); }
  const CCollisionInfo& operator[](int index) const { return x0_list[index]; }
  CCollisionInfo& operator[](int index) { return x0_list[index]; }

private:
  rstl::reserved_vector< CCollisionInfo, 32 > x0_list;
};
CHECK_SIZEOF(CCollisionInfoList, 0xc04)

#endif // _CCOLLISIONINFOLIST
