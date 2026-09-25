#include "Kyoto/Animation/CCharLayoutInfo.hpp"

#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/StringExtras.hpp"

CFactoryFnReturn FCharLayoutInfo(const SObjectTag& tag, CInputStream& in, const CVParamTransfer&) {
  return rs_new CCharLayoutInfo(in);
}

CCharLayoutNode::CCharLayoutNode(CInputStream& in)
: mParent(in), mReferenceStanceOffset(in), mRotation(in), mLocalRotation(in), mConnectedParts(in) {}

CCharLayoutInfo::CCharLayoutInfo(CInputStream& in)
: mNodes(rstl::ownership_transfer< TSegIdMap< CCharLayoutNode > >(
      rs_new TSegIdMap< CCharLayoutNode >(in)))
, mSegIdList(in)
, mNameMap(in) {
  InitializeLinearData();
}

void CCharLayoutInfo::InitializeLinearData() {
  const int count = mSegIdList.GetCount();
  mLinearParents.reserve(count);
  mLinearReferenceStanceOffsets.reserve(count);
  mLinearParentOffsets.reserve(count);
  mLinearRotations.reserve(count);
  mLinearLocalRotations.reserve(count);

  for (int i = 0; i < count; ++i) {
    CSegId id(i);
    const CCharLayoutNode& node = GetSegmentData(id);
    mLinearParents.push_back_unsafe(node.GetParent());
    mLinearReferenceStanceOffsets.push_back_unsafe(node.GetReferenceStanceOffset());
    mLinearParentOffsets.push_back_unsafe(GetFromParentUnrotated(id));
    mLinearRotations.push_back_unsafe(node.GetRotation());
    mLinearLocalRotations.push_back_unsafe(node.GetLocalRotation());
  }
}

CSegId CCharLayoutInfo::GetSegIdFromString(const rstl::string& bone) const {
  for (rstl::map< rstl::string, CSegId >::const_iterator it = mNameMap.begin();
       it != mNameMap.end(); ++it) {
    if (CStringExtras::CompareCaseInsensitive(it->first, bone) == 0) {
      return it->second;
    }
  }
  return CSegId::Invalid();
}
