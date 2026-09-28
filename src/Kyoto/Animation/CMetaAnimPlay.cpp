#include "Kyoto/Animation/CMetaAnimPlay.hpp"

#include "Kyoto/Animation/CAllFormatsAnimSource.hpp"
#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/Animation/CAnimTreeAnimReaderContainer.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Streams/COutputStream.hpp"

rstl::ncrc_ptr< CAnimTreeNode >
CMetaAnimPlay::VGetAnimationTree(const CAnimSysContext& animSys,
                                 const CMetaAnimTreeBuildOrders& orders) const {
  if (orders.mRecursiveAdvance) {
    return GetAnimationTree(animSys,
                            CMetaAnimTreeBuildOrders::PreAdvanceForAll(*orders.mRecursiveAdvance));
  }

  IObjectStore& pool = animSys.GetSimplePool();
  const SObjectTag tag('ANIM', mPrimitive.GetAnimResId());
  TLockedToken< CAllFormatsAnimSource > anim = pool.GetObj(tag);
  const int animIdx = mPrimitive.GetAnimDatabaseIndex();
  rstl::ownership_transfer< IAnimReader > reader =
      CAllFormatsAnimSource::GetNewReader(anim, mStartTime, animSys.GetEventData(animIdx));
  return rs_new CAnimTreeAnimReaderContainer(reader, mPrimitive.GetName(), animIdx);
}

void CMetaAnimPlay::GetUniquePrimitives(rstl::set< CPrimitive >& primsOut) const {
  primsOut.insert(mPrimitive);
}

void CMetaAnimPlay::WriteAnimData(COutputStream& out) const {
  mPrimitive.PutTo(out);
  mStartTime.PutTo(out);
}
