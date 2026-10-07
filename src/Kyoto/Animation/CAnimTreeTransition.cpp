#include "Kyoto/Animation/CAnimTreeTransition.hpp"

#include "Kyoto/Math/CloseEnough.hpp"

uint CAnimTreeTransition::GetLoopPOIHash() {
  static uint hash = CPOINode::GetHashForString("Loop");
  return hash;
}

CAnimTreeTransition::CAnimTreeTransition(const bool characterSpaceBlend,
                                         const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                         const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                         const CCharAnimTime& duration, bool runA, int flags,
                                         const rstl::string& name)
: CAnimTreeTweenBase(characterSpaceBlend, a, b, flags, name)
, mTransDur(duration)
, mTimeInTrans(0.f)
, mRunA(runA)
, mLoopA(a->GetBoolPOIState(GetLoopPOIHash()))
, mInitialized(false) {}

CAnimTreeTransition::CAnimTreeTransition(const bool characterSpaceBlend,
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
  if (close_enough(GetBlendingWeight(), 1.f)) {
    rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simp = mB->Simplified();
    if (simp)
      return simp;
    return mB->Clone();
  }
  return CAnimTreeTweenBase::VSimplified();
}

rstl::optional_object< rstl::ownership_transfer< IAnimReader > >
CAnimTreeTransition::VReverseSimplified() {
  if (close_enough(GetBlendingWeight(), 0.f))
    return mA->Clone();
  return CAnimTreeTweenBase::VReverseSimplified();
}

rstl::pair< CCharAnimTime, SAdvancementDeltas >
CAnimTreeTransition::AdvanceViewForTransitionalPeriod(const CCharAnimTime& time) {
  IncAdvancementDepth();
  CDoubleChildAdvancementResult res = AdvanceViewBothChildren(time, mRunA, mLoopA);
  DecAdvancementDepth();
  const CCharAnimTime& trueAdvancement = res.GetTrueAdvancement();
  if (trueAdvancement.EqualsZero())
    return rstl::pair< CCharAnimTime, SAdvancementDeltas >(
        CCharAnimTime::ZeroFlat(),
        SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));

  float oldWeight = GetBlendingWeight();
  mTimeInTrans += trueAdvancement;
  float newWeight = GetBlendingWeight();
  if (ShouldCullTree()) {
    if (newWeight < 0.5f)
      mCullSelector = 1;
    else
      mCullSelector = 2;
  }

  const SAdvancementDeltas& leftDeltas = res.GetLeftAdvancementDeltas();
  const SAdvancementDeltas& rightDeltas = res.GetRightAdvancementDeltas();
  if (GetBlendRoot() & kBlendRoot_Offset)
    return rstl::pair< CCharAnimTime, SAdvancementDeltas >(
        trueAdvancement,
        SAdvancementDeltas::Interpolate(leftDeltas, rightDeltas, oldWeight, newWeight));
  return rstl::pair< CCharAnimTime, SAdvancementDeltas >(trueAdvancement, rightDeltas);
}

SAdvancementResults CAnimTreeTransition::VAdvanceView(const CCharAnimTime& time) {
  if (time.EqualsZero()) {
    IncAdvancementDepth();
    mB->AdvanceView(time);
    if (mRunA)
      mA->AdvanceView(time);
    DecAdvancementDepth();
    if (ShouldCullTree())
      mCullSelector = 1;
    return SAdvancementResults(CCharAnimTime::ZeroFlat(),
                               SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
  }

  if (!mInitialized)
    mInitialized = true;
  if (mTimeInTrans + time < mTransDur) {
    rstl::pair< CCharAnimTime, SAdvancementDeltas > res = AdvanceViewForTransitionalPeriod(time);
    return SAdvancementResults(time - res.first, res.second);
  }

  CCharAnimTime transTimeRem = mTransDur - mTimeInTrans;
  rstl::pair< CCharAnimTime, SAdvancementDeltas > res(
      CCharAnimTime(0.f), SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
  if (transTimeRem.GreaterThanZero()) {
    res = AdvanceViewForTransitionalPeriod(transTimeRem);
    if (res.first != transTimeRem)
      return SAdvancementResults(res.first, res.second);
  }
  CCharAnimTime remainder = time - transTimeRem;
  return SAdvancementResults(remainder, res.second);
}

rstl::ownership_transfer< IAnimReader > CAnimTreeTransition::VClone() const {
  return rs_new CAnimTreeTransition(CharacterSpaceBlend(), Cast(mA->Clone()), Cast(mB->Clone()),
                                    mTransDur, mTimeInTrans, mRunA, mLoopA, GetBlendRoot(), mName,
                                    mInitialized);
}

float CAnimTreeTransition::VGetBlendingWeight() const {
  if (mTransDur.GreaterThanZero()) {
    return (1.f / mTransDur.GetSeconds()) * mTimeInTrans.GetSeconds();
  }
  return 1.f;
}

CCharAnimTime CAnimTreeTransition::VGetTimeRemaining() const {
  return rstl::max_val< const CCharAnimTime& >(mB->VGetTimeRemaining(), mTransDur - mTimeInTrans);
}

CSteadyStateAnimInfo CAnimTreeTransition::VGetSteadyStateAnimInfo() const {
  CSteadyStateAnimInfo info = mB->VGetSteadyStateAnimInfo();
  return CSteadyStateAnimInfo(info.IsLooping(),
                              rstl::max_val< const CCharAnimTime& >(mTransDur, info.GetDuration()),
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
  if (!best) {
    return right;
  }
  return best;
}

const int CAnimTreeTweenBase::kBlendRoot_Offset = 1;

const int CAnimTreeTweenBase::kBlendRoot_Rotation = 2;
