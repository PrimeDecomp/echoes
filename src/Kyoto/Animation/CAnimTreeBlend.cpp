#include "Kyoto/Animation/CAnimTreeBlend.hpp"

rstl::ownership_transfer< IAnimReader > CAnimTreeBlend::VClone() const {
  return rs_new CAnimTreeBlend(CharacterSpaceBlend(), Cast(mA->VClone()), Cast(mB->VClone()),
                               mBlendWeight, mName);
}

float CAnimTreeBlend::VGetBlendingWeight() const { return mBlendWeight; }

CCharAnimTime CAnimTreeBlend::VGetTimeRemaining() const {
  return rstl::max_val(mA->VGetTimeRemaining(), mB->VGetTimeRemaining());
}

CSteadyStateAnimInfo CAnimTreeBlend::VGetSteadyStateAnimInfo() const {
  // TODO: Combine child durations and displacements with duration-relative weighting.
  return mB->VGetSteadyStateAnimInfo();
}

rstl::string CAnimTreeBlend::CreatePrimitiveName(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                 const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                                 float weight) {
  return rstl::string_l("");
}

void CAnimTreeBlend::SetBlendingWeight(float weight) { mBlendWeight = weight; }

SAdvancementResults CAnimTreeBlend::VAdvanceView(const CCharAnimTime& time) {
  // TODO: Advance both children, choose culling state and blend root motion.
  return SAdvancementResults(time);
}
