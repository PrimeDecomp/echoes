#include "WorldFormat/CAreaOctTree.hpp"

static CAABox BoxFromIndex(int index, const CVector3f& min, const CVector3f& center,
                           const CVector3f& max) {
  switch (index) {
  case 0:
    return CAABox(min, center);
  case 1:
    return CAABox(CVector3f(center.GetX(), min.GetY(), min.GetZ()),
                  CVector3f(max.GetX(), center.GetY(), center.GetZ()));
  case 2:
    return CAABox(CVector3f(min.GetX(), center.GetY(), min.GetZ()),
                  CVector3f(center.GetX(), max.GetY(), center.GetZ()));
  case 3:
    return CAABox(CVector3f(center.GetX(), center.GetY(), min.GetZ()),
                  CVector3f(max.GetX(), max.GetY(), center.GetZ()));
  case 4:
    return CAABox(CVector3f(min.GetX(), min.GetY(), center.GetZ()),
                  CVector3f(center.GetX(), center.GetY(), max.GetZ()));
  case 5:
    return CAABox(CVector3f(center.GetX(), min.GetY(), center.GetZ()),
                  CVector3f(max.GetX(), center.GetY(), max.GetZ()));
  case 6:
    return CAABox(CVector3f(min.GetX(), center.GetY(), center.GetZ()),
                  CVector3f(center.GetX(), max.GetY(), max.GetZ()));
  case 7:
    return CAABox(center, max);
  default:
    return CAABox(min, max);
  }
}

CAreaOctTree::Node CAreaOctTree::Node::GetChild(int index) const {
  const ETreeType type = GetChildType(index);
  const uint* offsets = reinterpret_cast< const uint* >(mPtr + sizeof(uint));
  const void* child = mPtr + 9 * sizeof(uint) + offsets[index];
  if (type == kTT_Leaf) {
    return Node(child, *static_cast< const CAABox* >(child), mOwner, type);
  }

  const CVector3f center = 0.5f * (mAabb.GetMinPoint() + mAabb.GetMaxPoint());
  return Node(child, BoxFromIndex(index, mAabb.GetMinPoint(), center, mAabb.GetMaxPoint()), mOwner,
              type);
}

CAreaOctTree::TriListReference CAreaOctTree::Node::GetTriangleArray() const {
  // Include the leaf bounds prefix so the empty reference's count is valid too.
  static const ushort skDeadArray[sizeof(CAABox) / sizeof(ushort) + 1] = {0};
  return TriListReference(mNodeType == kTT_Leaf ? mPtr : static_cast< const void* >(skDeadArray));
}

CAreaOctTree::CAreaOctTree(const CAABox& bounds, Node::ETreeType treeType, const uchar* buffer,
                           const void* treeBuffer, int materialCount, int vertexCount,
                           int edgeCount, int triangleCount, const u64* materials,
                           const uchar* vertexMaterials, const uchar* edgeMaterials,
                           const uchar* surfaceMaterials, const CCollisionEdge* edges,
                           const ushort* surfaceIndices, const ushort* extraIndices,
                           const CVector3f* vertices)
: CCollisionPrimitiveData(materialCount, vertexCount, edgeCount, triangleCount, materials,
                          vertexMaterials, edgeMaterials, surfaceMaterials, edges, surfaceIndices,
                          extraIndices, vertices, false)
, mAabb(bounds)
, mTreeType(treeType)
, mBuf(buffer)
, mTreeBuf(treeBuffer) {}

void CAreaOctTree::MakeFromMemory(void* buffer, uint bufferLength, CAreaOctTree** treeOut,
                                  bool* valid) {
  // TODO: Decode the tree and nonowning collision-array views, including 64-bit
  // materials and Echoes's additional serialized triangle-index array.
  *treeOut = nullptr;
  *valid = false;
}
