#ifndef _CCOLLISIONEDGE
#define _CCOLLISIONEDGE

#include "Kyoto/Streams/CInputStream.hpp"

class CCollisionEdge {
public:
  CCollisionEdge(ushort index1, ushort index2) : mIndex1(index1), mIndex2(index2) {}
  explicit CCollisionEdge(CInputStream& in) {
    mIndex1 = in.Get< ushort >();
    mIndex2 = in.Get< ushort >();
  }

  ushort GetVertIndex1() const { return mIndex1; }
  ushort GetVertIndex2() const { return mIndex2; }

private:
  ushort mIndex1;
  ushort mIndex2;
};
CHECK_SIZEOF(CCollisionEdge, 4)

#endif // _CCOLLISIONEDGE
