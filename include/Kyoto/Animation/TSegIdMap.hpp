#ifndef _TSEGIDMAP
#define _TSEGIDMAP

#include "types.h"

#include "Kyoto/Animation/CSegId.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"

template < typename T >
class TSegIdMap {
public:
  const T& operator[](const CSegId& id) const {
    return mNodes[mIndirectionMap[id.val()].second.val()];
  }
  const T& AccessElement(int index) const { return mNodes[index]; }

private:
  char mBoneCount;
  char mCapacity;
  rstl::reserved_vector< rstl::pair< CSegId, CSegId >, 100 > mIndirectionMap;
  T* mNodes;
  char mCurPrevBone;
};

#endif // _TSEGIDMAP
