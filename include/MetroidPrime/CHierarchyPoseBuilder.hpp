#ifndef _CHIERARCHYPOSEBUILDER
#define _CHIERARCHYPOSEBUILDER

#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Animation/TSegIdMap.hpp"

#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/construction_deferred.hpp"
#include "rstl/optional_object.hpp"

class CCharLayoutInfo;

class CLayoutDescription {
public:
  explicit CLayoutDescription(const TLockedToken< CCharLayoutInfo >& layout)
  : mLayoutToken(layout) {}

  class CScaledLayoutDescription {
  private:
    TLockedToken< CCharLayoutInfo > mLayoutToken;
    float mScale;
    rstl::optional_object< CVector3f > mScaleVec;
  };

private:
  TLockedToken< CCharLayoutInfo > mLayoutToken;
  rstl::optional_object< CScaledLayoutDescription > mScaled;
};
CHECK_SIZEOF(CLayoutDescription, 0x30)

class CHierarchyPoseBuilder {
public:
  CHierarchyPoseBuilder(const CLayoutDescription& layout, bool animatedScale);

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
  rstl::auto_ptr< TSegIdMap< CVector3f > > mScales;
};
CHECK_SIZEOF(CHierarchyPoseBuilder, 0x118)

#endif // _CHIERARCHYPOSEBUILDER
