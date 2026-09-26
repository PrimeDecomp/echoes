#ifndef _CCHARLAYOUTINFO
#define _CCHARLAYOUTINFO

#include "types.h"

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Animation/CSegIdList.hpp"
#include "Kyoto/Animation/TSegIdMap.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/map.hpp"
#include "rstl/object_owner.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CCharLayoutNode {
public:
  explicit CCharLayoutNode(CInputStream& in);
  ~CCharLayoutNode() {}

  const CSegId& GetParent() const { return mParent; }

  const CVector3f& GetReferenceStanceOffset() const { return mReferenceStanceOffset; }

  const CQuaternion& GetRotation() const { return mRotation; }

  const CQuaternion& GetLocalRotation() const { return mLocalRotation; }

  int GetNumConnectedParts() const { return mConnectedParts.size(); }

  const rstl::vector< CSegId >& GetConnectedParts() const { return mConnectedParts; }

private:
  CSegId mParent;
  CVector3f mReferenceStanceOffset;
  CQuaternion mRotation;
  CQuaternion mLocalRotation;
  rstl::vector< CSegId > mConnectedParts;
};
CHECK_SIZEOF(CCharLayoutNode, 0x40)

class CCharLayoutInfo {
public:
  explicit CCharLayoutInfo(CInputStream& in);
  CSegId GetSegIdFromString(const rstl::string& bone) const;

  const CCharLayoutNode& GetSegmentData(const CSegId& seg) const { return (*mNodes)[seg]; }

  const CSegIdList& GetBodyPartSegIds() const { return mSegIdList; }

  const rstl::vector< CSegId >& GetLinearParents() const { return mLinearParents; }

  CVector3f GetFromParentUnrotated(const CSegId& id) const {
    const CCharLayoutNode& node = GetSegmentData(id);
    return !mNodes->ContainsDataFor(node.GetParent())
               ? node.GetReferenceStanceOffset()
               : node.GetReferenceStanceOffset() -
                     GetSegmentData(node.GetParent()).GetReferenceStanceOffset();
  }

private:
  void InitializeLinearData(); // Guessed name.

  rstl::object_owner< TSegIdMap< CCharLayoutNode > > mNodes;
  CSegIdList mSegIdList;
  rstl::map< rstl::string, CSegId > mNameMap;
  rstl::vector< CSegId > mLinearParents;
  rstl::vector< CVector3f > mLinearReferenceStanceOffsets;
  rstl::vector< CVector3f > mLinearParentOffsets;
  rstl::vector< CQuaternion > mLinearRotations;
  rstl::vector< CQuaternion > mLinearLocalRotations;
};
CHECK_SIZEOF(CCharLayoutInfo, 0x7c)

#endif // _CCHARLAYOUTINFO
