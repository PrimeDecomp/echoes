#include "Kyoto/Animation/CAnimTreeTransition.hpp"

CAnimTreeTransition::CAnimTreeTransition(bool characterSpaceBlend,
                                         const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                         const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                         const CCharAnimTime& duration, bool runA, int flags,
                                         const rstl::string& name)
: CAnimTreeTweenBase(characterSpaceBlend, a, b, flags, name)
, mTransDur(duration)
, mTimeInTrans(0.f)
, mRunA(runA)
, mLoopA(a->VGetBoolPOIState(GetLoopPOIHash()))
, mInitialized(false) {}

CAnimTreeTransition::CAnimTreeTransition(bool characterSpaceBlend,
                                         const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                         const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                         const CCharAnimTime& duration,
                                         const CCharAnimTime& timeInTrans, bool runA, bool loopA,
                                         int flags, const rstl::string& name, bool initialized)
: CAnimTreeTweenBase(characterSpaceBlend, a, b, flags, name)
, mTransDur(duration)
, mTimeInTrans(timeInTrans)
, mRunA(runA)
, mLoopA(loopA)
, mInitialized(initialized) {}

CAnimTreeTransition::~CAnimTreeTransition() {}

rstl::optional_object< rstl::ownership_transfer< IAnimReader > >
CAnimTreeTransition::VSimplified() {
  // TODO: Select the destination reader when the transition finishes; otherwise simplify both.
  return rstl::optional_object_null();
}

rstl::optional_object< rstl::ownership_transfer< IAnimReader > >
CAnimTreeTransition::VReverseSimplified() {
  // TODO: Select the source reader at zero transition weight.
  return rstl::optional_object_null();
}

rstl::pair< CCharAnimTime, SAdvancementDeltas >
CAnimTreeTransition::AdvanceViewForTransitionalPeriod(const CCharAnimTime& time) {
  // TODO: Advance both children and interpolate root motion between old/new transition weights.
  SAdvancementResults unchanged(time);
  return rstl::pair< CCharAnimTime, SAdvancementDeltas >(CCharAnimTime(0.f), unchanged.mDeltas);
}

SAdvancementResults CAnimTreeTransition::VAdvanceView(const CCharAnimTime& time) {
  // TODO: Split advancement at the transition boundary and update initialization/culling state.
  return SAdvancementResults(time);
}

rstl::ownership_transfer< IAnimReader > CAnimTreeTransition::VClone() const {
  return rs_new CAnimTreeTransition(CharacterSpaceBlend(), Cast(mA->VClone()), Cast(mB->VClone()),
                                    mTransDur, mTimeInTrans, mRunA, mLoopA, GetBlendRoot(), mName,
                                    mInitialized);
}

float CAnimTreeTransition::VGetBlendingWeight() const {
  if (mTransDur.GreaterThanZero()) {
    return mTimeInTrans.GetSeconds() / mTransDur.GetSeconds();
  }
  return 1.f;
}

CCharAnimTime CAnimTreeTransition::VGetTimeRemaining() const {
  return rstl::max_val(mB->VGetTimeRemaining(), mTransDur - mTimeInTrans);
}

CSteadyStateAnimInfo CAnimTreeTransition::VGetSteadyStateAnimInfo() const {
  CSteadyStateAnimInfo info = mB->VGetSteadyStateAnimInfo();
  return CSteadyStateAnimInfo(info.IsLooping(), rstl::max_val(mTransDur, info.GetDuration()),
                              info.GetOffset());
}

rstl::string CAnimTreeTransition::CreatePrimitiveName(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                      const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                                      float duration) {
  return rstl::string_l("");
}

void CAnimTreeTransition::SetBlendingWeight(float weight) {
  rstl::rc_ptr< CAnimTreeNode > right = GetRightChild();
  static_cast< CAnimTreeTweenBase* >(right.GetPtr())->SetBlendingWeight(weight);
}

rstl::rc_ptr< CAnimTreeNode > CAnimTreeTransition::VGetBestUnblendedChild() const {
  rstl::rc_ptr< CAnimTreeNode > right = GetRightChild();
  rstl::rc_ptr< CAnimTreeNode > best = right->GetBestUnblendedChild();
  return best ? best : right;
}

const int CAnimTreeTweenBase::kBlendRoot_Offset = 1;

const int CAnimTreeTweenBase::kBlendRoot_Rotation = 2;
