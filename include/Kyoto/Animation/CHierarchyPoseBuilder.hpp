#ifndef _CHIERARCHYPOSEBUILDER
#define _CHIERARCHYPOSEBUILDER

#include "Kyoto/Animation/CLayoutDescription.hpp"
#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Animation/TSegIdMap.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/construction_deferred.hpp"

class CCharLayoutInfo;

class CHierarchyPoseBuilder {
public:
  typedef TSegIdMap< CVector3f > ScaleMap;

  CHierarchyPoseBuilder(const CLayoutDescription& layout, bool animatedScale);

  class CTreeNode {
  public:
    CTreeNode(const CSegId& sibling, const CSegId& child, const CVector3f& offset)
    : mChild(child), mSibling(sibling), mRotation(CQuaternion::NoRotation()), mOffset(offset) {}

    CTreeNode NodeForNextChildInserted(const CSegId& child, const CSegId& nullId,
                                       const CVector3f& offset) {
      CSegId sibling = mChild;
      mChild = child;
      return CTreeNode(sibling, nullId, offset);
    }

  private:
    CSegId mChild;
    CSegId mSibling;
    CQuaternion mRotation;
    CVector3f mOffset;
  };

private:
  void BuildIntoHeirarchy(const CCharLayoutInfo& layout, const CSegId& seg, const CSegId& root);

  CLayoutDescription mLayoutDesc;
  rstl::construction_deferred< CSegId > mRootId;
  TSegIdMap< CTreeNode > mTreeMap;
  rstl::auto_ptr< ScaleMap > mScales;
};

CHECK_SIZEOF(CHierarchyPoseBuilder, 0x118)
NESTED_CHECK_SIZEOF(CHierarchyPoseBuilder, CTreeNode, 0x20)

#endif // _CHIERARCHYPOSEBUILDER
