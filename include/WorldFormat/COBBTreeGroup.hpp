#ifndef _COBBTREEGROUP
#define _COBBTREEGROUP

#include "Kyoto/Math/CAABox.hpp"
#include "WorldFormat/COBBTree.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/vector.hpp"

class CInputStream;

class COBBTreeGroup {
public:
  explicit COBBTreeGroup(CInputStream& in);
  COBBTreeGroup(const CVector3f& extent, const CVector3f& center);
  explicit COBBTreeGroup(rstl::auto_ptr< COBBTree >& tree);

  int NumTrees() const { return mTrees.size(); }
  COBBTree* GetTree(int idx) const { return mTrees[idx].get(); } // Guessed name.

private:
  friend class CCollidableOBBTreeGroup;
  rstl::vector< rstl::auto_ptr< COBBTree > > mTrees;
  rstl::vector< CAABox > mAabbs;
  CAABox mAabox;
};
CHECK_SIZEOF(COBBTreeGroup, 0x38)

#endif // _COBBTREEGROUP
