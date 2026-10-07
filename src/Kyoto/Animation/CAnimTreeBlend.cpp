#include "Kyoto/Animation/CAnimTreeBlend.hpp"

rstl::ownership_transfer< IAnimReader > CAnimTreeBlend::VClone() const {
  return rs_new CAnimTreeBlend(CharacterSpaceBlend(), Cast(mA->Clone()), Cast(mB->Clone()),
                               mBlendWeight, mName);
}

float CAnimTreeBlend::VGetBlendingWeight() const { return mBlendWeight; }

CCharAnimTime CAnimTreeBlend::VGetTimeRemaining() const {
  return rstl::max_val< const CCharAnimTime& >(mA->VGetTimeRemaining(),
                                               mB->VGetTimeRemaining());
}

CSteadyStateAnimInfo CAnimTreeBlend::VGetSteadyStateAnimInfo() const {
  CSteadyStateAnimInfo infoA = mA->GetSteadyStateAnimInfo();
  CSteadyStateAnimInfo infoB = mB->GetSteadyStateAnimInfo();
  CVector3f offsetA = infoA.GetOffset();
  CVector3f offsetB = infoB.GetOffset();
  CCharAnimTime durationA = infoA.GetDuration();
  CCharAnimTime durationB = infoB.GetDuration();
  CVector3f offset;
  if (durationA < durationB) {
    const float scale = durationB / durationA;
    offset = offsetA * scale * mBlendWeight + offsetB * (1.f - mBlendWeight);
  } else if (durationB < durationA) {
    const float scale = durationA / durationB;
    offset = offsetA * mBlendWeight + offsetB * scale * (1.f - mBlendWeight);
  } else {
    offset = offsetA + offsetB;
  }

  return CSteadyStateAnimInfo(
      infoB.IsLooping(),
      rstl::max_val< const CCharAnimTime& >(infoA.GetDuration(), infoB.GetDuration()), offset);
}

rstl::string CAnimTreeBlend::CreatePrimitiveName(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                 const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                                 float weight) {
  return rstl::string_l("");
}

void CAnimTreeBlend::SetBlendingWeight(float weight) { mBlendWeight = weight; }

SAdvancementResults CAnimTreeBlend::VAdvanceView(const CCharAnimTime& time) {
  IncAdvancementDepth();
  SAdvancementResults resA = mA->AdvanceView(time);
  SAdvancementResults resB = mB->AdvanceView(time);
  DecAdvancementDepth();

  if (ShouldCullTree()) {
    if (GetBlendingWeight() < 0.5f)
      mCullSelector = 1;
    else
      mCullSelector = 2;
  }

  const CCharAnimTime remainder =
      rstl::max_val< const CCharAnimTime& >(resA.GetRemainder(), resB.GetRemainder());
  const SAdvancementDeltas& deltasA = resA.mDeltas;
  const SAdvancementDeltas& deltasB = resB.mDeltas;
  if (GetBlendRoot() & kBlendRoot_Offset)
    return SAdvancementResults(remainder,
                               SAdvancementDeltas::Blend(deltasA, deltasB, GetBlendingWeight()));
  return resB;
}
