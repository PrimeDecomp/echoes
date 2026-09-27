#ifndef _CAREAOCTTREE
#define _CAREAOCTTREE

#include "Kyoto/Math/CAABox.hpp"
#include "WorldFormat/CCollisionPrimitiveData.hpp"
#include "WorldFormat/CCollisionSurface.hpp"
#include "rstl/optional_object.hpp"

class CLine;
class CMaterialFilter;

class CAreaOctTree : public CCollisionPrimitiveData {
public:
  struct SRayResult {
    CPlane mPlane;
    rstl::optional_object< CCollisionSurface > mSurface;
    float mT;

    SRayResult() : mPlane(0.f, CVector3f::Right()), mT(0.f) {}
  };

  class TriListReference {
  public:
    explicit TriListReference(const void* data) : mData(static_cast< const ushort* >(data)) {}
    ushort GetSize() const { return mData[kTriangleCountOffset]; }
    ushort GetAt(int index) const { return mData[kTriangleDataOffset + index]; }

  private:
    enum {
      kTriangleCountOffset = sizeof(CAABox) / sizeof(ushort),
      kTriangleDataOffset = kTriangleCountOffset + 1
    };
    const ushort* mData;
  };

  class Node {
  public:
    enum ETreeType { kTT_Invalid, kTT_Branch, kTT_Leaf };

    Node(const void* ptr, const CAABox& aabb, const CAreaOctTree& owner, ETreeType type)
    : mAabb(aabb), mPtr(static_cast< const uchar* >(ptr)), mOwner(owner), mNodeType(type) {}

    bool LineTest(const CLine& line, const CMaterialFilter& filter, float length) const;
    void LineTestEx(const CLine& line, const CMaterialFilter& filter, SRayResult& result,
                    float length) const;
    Node GetChild(int index) const;
    TriListReference GetTriangleArray() const;

    const CAABox& GetBoundingBox() const { return mAabb; }
    const CAreaOctTree& GetOwner() const { return mOwner; }
    ETreeType GetTreeType() const { return mNodeType; }
    ushort GetChildFlags() const { return *reinterpret_cast< const ushort* >(mPtr); }
    ETreeType GetChildType(int index) const {
      return static_cast< ETreeType >((GetChildFlags() >> (2 * index)) & 3);
    }

  private:
    CAABox mAabb;
    const uchar* mPtr;
    const CAreaOctTree& mOwner;
    ETreeType mNodeType;

    bool LineTestInternal(const CLine& line, const CMaterialFilter& filter, float lowT, float highT,
                          float maxT, const CVector3f& directionReciprocal) const;
    void LineTestExInternal(const CLine& line, const CMaterialFilter& filter, SRayResult& result,
                            float lowT, float highT, float maxT,
                            const CVector3f& directionReciprocal) const;
  };

  CAreaOctTree(const CAABox& bounds, Node::ETreeType treeType, const uchar* buffer,
               const void* treeBuffer, int materialCount, int vertexCount, int edgeCount,
               int triangleCount, const u64* materials, const uchar* vertexMaterials,
               const uchar* edgeMaterials, const uchar* surfaceMaterials,
               const CCollisionEdge* edges, const ushort* surfaceIndices,
               const ushort* extraIndices, const CVector3f* vertices);
  static void MakeFromMemory(void* buffer, uint bufferLength, CAreaOctTree** treeOut, bool* valid);

  Node GetRootNode() const { return Node(mTreeBuf, mAabb, *this, mTreeType); }
  const CAABox& GetBoundingBox() const { return mAabb; }
  Node::ETreeType GetTreeType() const { return mTreeType; }
  const void* GetTreeMemory() const { return mTreeBuf; }

private:
  CAABox mAabb;
  Node::ETreeType mTreeType;
  const uchar* mBuf;
  const void* mTreeBuf;
};
CHECK_SIZEOF(CAreaOctTree, 0x58)
NESTED_CHECK_SIZEOF(CAreaOctTree, Node, 0x24)
NESTED_CHECK_SIZEOF(CAreaOctTree, TriListReference, 0x4)
NESTED_CHECK_SIZEOF(CAreaOctTree, SRayResult, 0x48)

#endif // _CAREAOCTTREE
