#ifndef _CCOLLISIONINFOLIST
#define _CCOLLISIONINFOLIST

#include "Collision/CCollisionInfo.hpp"
#include "rstl/reserved_vector.hpp"

class CCollisionInfoList {
public:
  int GetCount() const { return mList.size(); }
  void Clear() { mList.clear(); }
  const CCollisionInfo& operator[](int index) const { return mList[index]; }
  CCollisionInfo& operator[](int index) { return mList[index]; }

private:
  rstl::reserved_vector< CCollisionInfo, 32 > mList;
};
CHECK_SIZEOF(CCollisionInfoList, 0xc04)

#endif // _CCOLLISIONINFOLIST
