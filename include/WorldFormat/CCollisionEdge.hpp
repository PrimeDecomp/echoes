#ifndef _CCOLLISIONEDGE
#define _CCOLLISIONEDGE

#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/construct.hpp"

class CCollisionEdge {
public:
  CCollisionEdge(ushort index1, ushort index2);
  explicit CCollisionEdge(CInputStream& in);

  ushort GetVertIndex1() const { return mIndex1; }
  ushort GetVertIndex2() const { return mIndex2; }

private:
  ushort mIndex1;
  ushort mIndex2;
};
CHECK_SIZEOF(CCollisionEdge, 4)

namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CCollisionEdge)
} // namespace rstl

#endif // _CCOLLISIONEDGE
