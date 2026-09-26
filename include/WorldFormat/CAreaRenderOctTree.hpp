#ifndef _CAREARENDEROCTTREE
#define _CAREARENDEROCTTREE

#include "Kyoto/Math/CAABox.hpp"
#include "rstl/auto_ptr.hpp"

class CAreaRenderOctTree {
public:
  explicit CAreaRenderOctTree(const rstl::auto_ptr< const uchar >& buffer);

private:
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

#endif // _CAREARENDEROCTTREE
