#include "WorldFormat/CCollisionEdge.hpp"

CCollisionEdge::CCollisionEdge(ushort index1, ushort index2)
: mIndex1(index1), mIndex2(index2) {}

CCollisionEdge::CCollisionEdge(CInputStream& in)
: mIndex1(in.Get< ushort >()), mIndex2(in.Get< ushort >()) {}
