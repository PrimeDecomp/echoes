#include "WorldFormat/CAreaBspTree.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CAreaBspTree::CAreaBspTree(CInputStream& in, const CTransform4f& transform) {
  in.ReadInt32();
  in.ReadInt16();
  in.ReadInt16();
}
