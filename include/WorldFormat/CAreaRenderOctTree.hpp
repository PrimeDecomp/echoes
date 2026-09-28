#ifndef _CAREARENDEROCTTREE
#define _CAREARENDEROCTTREE

#include "Kyoto/Math/CAABox.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/vector.hpp"

class CAreaRenderOctTree {
public:
  struct Node {
    // Guessed names for the serialized subdivision flags.
    enum ESubdivision {
      kS_Leaf = 0,
      kS_X = 1,
      kS_Y = 2,
      kS_XY = 3,
      kS_Z = 4,
      kS_XZ = 5,
      kS_ZX = 6,
      kS_XYZ = 7
    };

    ushort mBitmapIndex;
    uchar x2_;
    uchar mFlags;
    ushort mChildren[1]; // Variable-length serialized child indices.

    int GetChildCount() const;
    CAABox GetNodeBounds(const CAABox& bounds, int childIndex) const;
    void RecursiveBuildOverlaps(uint* bitmap, const CAreaRenderOctTree& tree, const CAABox& bounds,
                                const CAABox& testBounds) const;
  };

  explicit CAreaRenderOctTree(const rstl::auto_ptr< const uchar >& buffer);
  uint GetBitmapWordCount() const { return mBitmapWordCount; }
  void FindOverlappingModels(rstl::vector< uint >& bitmap, const CAABox& bounds) const;
  void FindOverlappingModels(uint* bitmap, const CAABox& bounds) const;
  static bool TestBit(const uint* bitmap, int bitIndex);

private:
  friend struct Node;
  const Node* GetNode(int index) const;

  rstl::auto_ptr< const uchar > mBuffer;
  uint mBitmapCount;
  uint mMeshCount;
  uint mNodeCount;
  uint mBitmapWordCount;
  CAABox mBounds;
  const uint* mBitmaps;
  const uint* mIndirectionTable;
  const uchar* mEntries;
};
CHECK_SIZEOF(CAreaRenderOctTree, 0x3c)
NESTED_CHECK_SIZEOF(CAreaRenderOctTree, Node, 0x6)

#endif // _CAREARENDEROCTTREE
