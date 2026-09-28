#include "Kyoto/Animation/CHierarchyPoseBuilder.hpp"

#include "Kyoto/Animation/CCharLayoutInfo.hpp"

CHierarchyPoseBuilder::CHierarchyPoseBuilder(const CLayoutDescription& layout, bool animatedScale)
: mLayoutDesc(layout)
, mTreeMap(layout.GetNumSegments())
, mScales(animatedScale ? rstl::auto_ptr< ScaleMap >(rs_new ScaleMap(layout.GetNumSegments()))
                        : rstl::auto_ptr< ScaleMap >()) {
  TToken< CCharLayoutInfo > layoutToken = layout.ScaledLayout();
  const CCharLayoutInfo& layoutInfo = **layoutToken;
  const CSegIdList& segments = layoutInfo.GetBodyPartSegIds();
  for (CSegIdList::const_iterator it = segments.begin(), end = segments.end(); it != end; ++it) {
    CSegId seg = *it;
    BuildIntoHeirarchy(layoutInfo, seg, CSegId::Character());
    if (mScales.get()) {
      mScales->insert(seg, CVector3f::One());
    }
  }
}

void CHierarchyPoseBuilder::BuildIntoHeirarchy(const CCharLayoutInfo& layout, const CSegId& seg,
                                               const CSegId& root) {
  if (!mTreeMap.ContainsDataFor(seg)) {
    CSegId parent = layout.GetOriginalParent(seg);
    if (parent == root) {
      mRootId.build(seg);
      mTreeMap.insert(
          seg, CTreeNode(CSegId::Null(), CSegId::Null(), layout.GetFromParentUnrotated(seg)));
    } else {
      BuildIntoHeirarchy(layout, parent, root);
      mTreeMap.insert(seg, mTreeMap[parent].NodeForNextChildInserted(
                               seg, CSegId::Null(), layout.GetFromParentUnrotated(seg)));
    }
  }
}

uchar CLayoutDescription::GetNumSegments() const {
  return mLayoutToken->GetBodyPartSegIds().GetCount();
}
