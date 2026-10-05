#ifndef _CCOLLISIONINFOLIST
#define _CCOLLISIONINFOLIST

#include "Collision/CCollisionInfo.hpp"
#include "rstl/reserved_vector.hpp"

class CCollisionInfoList {
public:
  void Add(const CCollisionInfo& info) {
    if (mList.size() == 32) {
      return;
    }
    mList.push_back(info);
  }
  void Swap(int start) {
    for (int i = start; i < GetCount(); ++i) {
      mList[i].Swap();
    }
  }

  int GetCount() const { return mList.size(); }
  void Clear() { mList.clear(); }
  const CCollisionInfo& operator[](int index) const { return mList[index]; }
  CCollisionInfo& operator[](int index) { return mList[index]; }

  const CCollisionInfo* Begin() const { return mList.begin(); }
  const CCollisionInfo* End() const { return mList.end(); }
  CVector3f GetCombinedNormalLeft() const;

private:
  rstl::reserved_vector< CCollisionInfo, 32 > mList;
};
CHECK_SIZEOF(CCollisionInfoList, 0xc04)

#endif // _CCOLLISIONINFOLIST
