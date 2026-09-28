#include "Kyoto/Animation/CMetaTransPhaseTrans.hpp"
#include "Kyoto/Animation/CAnimTreeTimeScale.hpp"
#include "Kyoto/Animation/CAnimTreeTransition.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"

CMetaTransPhaseTrans::CMetaTransPhaseTrans(CInputStream& in)
: mTransDur(CCharAnimTime(in))
, mCharacterSpaceBlend(in.ReadBool())
, mRunA(in.ReadBool())
, mFlags(in.ReadInt32()) {}

rstl::ncrc_ptr< CAnimTreeNode >
CMetaTransPhaseTrans::VGetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                         const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                         const CAnimSysContext& animSys) const {
  CSteadyStateAnimInfo infoA = a->GetContributionOfHighestInfluence().GetSteadyStateAnimInfo();
  CCharAnimTime durationA = infoA.GetDuration();
  CSteadyStateAnimInfo infoB = b->GetContributionOfHighestInfluence().GetSteadyStateAnimInfo();
  CCharAnimTime durationB = infoB.GetDuration();
  float scaleA = durationA / durationB;
  float scaleB = durationB / durationA;
  CAnimTreeEffectiveContribution contribution = a->GetContributionOfHighestInfluence();
  rstl::optional_object< float > phase = contribution.GetPhase();
  b->VSetPhase(*phase);

  rstl::ownership_transfer< IVaryingAnimationTimeScale > timeScaleA =
      rs_new CLinearAnimationTimeScale(CCharAnimTime::ZeroFlat(), 1.f, mTransDur, scaleA);
  rstl::ownership_transfer< IVaryingAnimationTimeScale > timeScaleB =
      rs_new CLinearAnimationTimeScale(CCharAnimTime::ZeroFlat(), scaleB, mTransDur, 1.f);
  rstl::ncrc_ptr< CAnimTreeNode > treeA = rs_new CAnimTreeTimeScale(
      a, timeScaleA, mTransDur, CAnimTreeTimeScale::CreatePrimitiveName(a, 1.f, mTransDur, scaleA));
  rstl::ncrc_ptr< CAnimTreeNode > treeB = rs_new CAnimTreeTimeScale(
      b, timeScaleB, mTransDur, CAnimTreeTimeScale::CreatePrimitiveName(b, scaleB, mTransDur, 1.f));
  return rs_new CAnimTreeTransition(
      mCharacterSpaceBlend, treeA, treeB, mTransDur, mRunA, mFlags,
      CAnimTreeTransition::CreatePrimitiveName(treeA, treeB, mTransDur.GetSeconds()));
}

void CMetaTransPhaseTrans::WriteTransData(COutputStream& out) const {
  mTransDur.PutTo(out);
  out.WriteChar(bool(mCharacterSpaceBlend));
  out.WriteChar(bool(mRunA));
  out.WriteLong(mFlags);
}
