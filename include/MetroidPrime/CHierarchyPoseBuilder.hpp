#ifndef _CHIERARCHYPOSEBUILDER
#define _CHIERARCHYPOSEBUILDER

// TODO: check for Echoes

#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Animation/CSegId.hpp"

#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/construction_deferred.hpp"
#include "rstl/optional_object.hpp"

class CCharLayoutInfo;

template < typename T >
class TSegIdMap {
private:
  CSegId mBoneCount;
  CSegId mCapacity;
  uint mMaxCapacity;
  CSegId mIndirectionMap[200];
  T* mNodes;
  CSegId mCurPrevBone;
};
typedef TSegIdMap< void > unk_TSegIdMap;
CHECK_SIZEOF(unk_TSegIdMap, 0xd8)

class CLayoutDescription {
public:
  class CScaledLayoutDescription {
  private:
    TCachedToken< CCharLayoutInfo > mLayoutToken;
    float mScale;
    rstl::optional_object< CVector3f > mScaleVec;
  };

private:
  TCachedToken< CCharLayoutInfo > mLayoutToken;
  rstl::optional_object< CScaledLayoutDescription > mScaled;
};
CHECK_SIZEOF(CLayoutDescription, 0x30)

class CHierarchyPoseBuilder {
public:
  class CTreeNode {
  private:
    CSegId mChild;
    CSegId mSibling;
    CQuaternion mRotation;
    CVector3f mOffset;
  };

private:
  CLayoutDescription mLayoutDesc;
  rstl::construction_deferred< CSegId > mRootId;
  TSegIdMap< CTreeNode > mTreeMap;
};
CHECK_SIZEOF(CHierarchyPoseBuilder, 0x110)

#endif // _CHIERARCHYPOSEBUILDER
