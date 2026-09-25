#ifndef _CCHARLAYOUTINFO
#define _CCHARLAYOUTINFO

#include "types.h"

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Animation/CSegIdList.hpp"
#include "Kyoto/Animation/TSegIdMap.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/vector.hpp"

class CCharLayoutNode {
public:
  int GetNumConnectedParts() const { return mConnectedParts.size(); }
  const rstl::vector< CSegId >& GetConnectedParts() const { return mConnectedParts; }

private:
  CSegId mParent;
  CVector3f mReferenceStanceOffset;
  CQuaternion x10_;
  CQuaternion x20_;
  rstl::vector< CSegId > mConnectedParts;
};
CHECK_SIZEOF(CCharLayoutNode, 0x40)

class CCharLayoutInfo {
public:
  const CCharLayoutNode& GetSegmentData(const CSegId& seg) const { return (*mNodes)[seg]; }
  const rstl::vector< CSegId >& GetLinearParents() const { return x2c_linearParents; }

private:
  TSegIdMap< CCharLayoutNode >* mNodes;
  CSegIdList mSegIdList;
  uchar x14_[0x18]; // Prime has the bone name map here
  rstl::vector< CSegId > x2c_linearParents;
};

#endif // _CCHARLAYOUTINFO
