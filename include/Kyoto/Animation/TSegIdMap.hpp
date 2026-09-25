#ifndef _TSEGIDMAP
#define _TSEGIDMAP

#include "types.h"

#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"
#include "Kyoto/Animation/CSegId.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"

template < typename T >
class TSegIdMap {
public:
  TSegIdMap(CInputStream& in)
  : mBoneCount(0)
  , mCapacity(in.Get< uint >())
  , mIndirectionMap(100, rstl::pair< CSegId, CSegId >(CSegId::Null(), CSegId::Null()))
  , mNodes(reinterpret_cast< T* >(rs_new char[mCapacity * sizeof(T)]))
  , mCurPrevBone(CSegId::Null()) {
    for (int i = 0; i < mCapacity; ++i) {
      CSegId seg(in);
      T node(in);
      insert(seg, node);
    }
    CCharAnimMemoryMetrics::AddToTotalSize(mCapacity, CCharAnimMemoryMetrics::kASS_Two);
  }

  ~TSegIdMap();

  T& operator[](const CSegId& id) { return mNodes[mIndirectionMap[id.val()].second.val()]; }

  const T& operator[](const CSegId& id) const {
    return mNodes[mIndirectionMap[id.val()].second.val()];
  }

  const T& AccessElement(int index) const { return mNodes[index]; }

  bool ContainsDataFor(CSegId id) const {
    return mIndirectionMap[id.val()] !=
           rstl::pair< CSegId, CSegId >(CSegId::Null(), CSegId::Null());
  }

  void insert(const CSegId& id, const T& value);

private:
  char mBoneCount;
  char mCapacity;
  rstl::reserved_vector< rstl::pair< CSegId, CSegId >, 100 > mIndirectionMap;
  T* mNodes;
  CSegId mCurPrevBone;
};
typedef TSegIdMap< uchar > TSegIdMapSizeCheck;
CHECK_SIZEOF(TSegIdMapSizeCheck, 0xd8)

template < typename T >
TSegIdMap< T >::~TSegIdMap() {
  CSegId id(mCurPrevBone);
  while (id != CSegId::Null()) {
    (*this)[id].~T();
    id = mIndirectionMap[id.val()].first;
  }

  CCharAnimMemoryMetrics::SubtractFromTotalSize(mCapacity, CCharAnimMemoryMetrics::kASS_Two);
  delete[] reinterpret_cast< char* >(mNodes);
}

template < typename T >
void TSegIdMap< T >::insert(const CSegId& id, const T& value) {
  T* node = &mNodes[mBoneCount];
  uchar rawId = id.val();
  new (node) T(value);
  mIndirectionMap[rawId] = rstl::pair< CSegId, CSegId >(mCurPrevBone, CSegId(mBoneCount));
  mCurPrevBone = id;
  ++mBoneCount;
}

#endif // _TSEGIDMAP
