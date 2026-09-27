#include "Kyoto/Animation/CAnimTreeDoubleChild.hpp"
#include "Kyoto/Animation/CBoolPOINode.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"
#include <stdlib.h>

CAnimTreeDoubleChild::CAnimTreeDoubleChild(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                           const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                           const rstl::string& name)
: CAnimTreeNode(name), mA(a), mB(b) {}

SAdvancementResults CAnimTreeDoubleChild::VAdvanceView(const CCharAnimTime& time) {
  SAdvancementResults a = mA->VAdvanceView(time);
  SAdvancementResults b = mB->VAdvanceView(time);
  return a.mRemTime > b.mRemTime ? a : b;
}

CAnimTreeDoubleChild::~CAnimTreeDoubleChild() {}

uint CAnimTreeDoubleChild::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
                                           uint capacity, uint iterator, int additive) const {
  uint count = mA->GetBoolPOIList(time, listOut, capacity, iterator, additive);
  count += mB->GetBoolPOIList(time, listOut, capacity, iterator + count, additive);
  count = rstl::min_val(count, capacity);
  qsort(listOut, count, sizeof(CBoolPOINode), CPOINode::compare);
  return count;
}

uint CAnimTreeDoubleChild::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut,
                                            uint capacity, uint iterator, int additive) const {
  uint count = mA->GetInt32POIList(time, listOut, capacity, iterator, additive);
  count += mB->GetInt32POIList(time, listOut, capacity, iterator + count, additive);
  count = rstl::min_val(count, capacity);
  qsort(listOut, count, sizeof(CInt32POINode), CPOINode::compare);
  return count;
}

uint CAnimTreeDoubleChild::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
                                               uint capacity, uint iterator, int additive) const {
  uint count = mA->GetParticlePOIList(time, listOut, capacity, iterator, additive);
  count += mB->GetParticlePOIList(time, listOut, capacity, iterator + count, additive);
  count = rstl::min_val(count, capacity);
  qsort(listOut, count, sizeof(CParticlePOINode), CPOINode::compare);
  return count;
}

uint CAnimTreeDoubleChild::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
                                            uint capacity, uint iterator, int additive) const {
  uint count = mA->GetSoundPOIList(time, listOut, capacity, iterator, additive);
  count += mB->GetSoundPOIList(time, listOut, capacity, iterator + count, additive);
  count = rstl::min_val(count, capacity);
  qsort(listOut, count, sizeof(CSoundPOINode), CPOINode::compare);
  return count;
}

bool CAnimTreeDoubleChild::VGetBoolPOIState(uint nameHash) const {
  return mB->VGetBoolPOIState(nameHash);
}

s32 CAnimTreeDoubleChild::VGetInt32POIState(uint nameHash) const {
  return mB->VGetInt32POIState(nameHash);
}

CParticleData::EParentedMode CAnimTreeDoubleChild::VGetParticlePOIState(uint nameHash) const {
  return mB->VGetParticlePOIState(nameHash);
}

CAnimTreeEffectiveContribution CAnimTreeDoubleChild::VGetContributionOfHighestInfluence() const {
  CAnimTreeEffectiveContribution a = mA->GetContributionOfHighestInfluence();
  CAnimTreeEffectiveContribution b = mB->GetContributionOfHighestInfluence();
  a.mContributionWeight *= GetLeftChildWeight();
  b.mContributionWeight *= GetRightChildWeight();
  return a.mContributionWeight > b.mContributionWeight ? a : b;
}

uint CAnimTreeDoubleChild::VGetNumChildren() const {
  return mA->VGetNumChildren() + mB->VGetNumChildren() + 2;
}

CAnimTreeDoubleChild::CDoubleChildAdvancementResult::CDoubleChildAdvancementResult(
    const CCharAnimTime& time, const SAdvancementDeltas& left, const SAdvancementDeltas& right)
: mTrueAdvancement(time), mLeftDeltas(left), mRightDeltas(right) {}

CAnimTreeDoubleChild::CDoubleChildAdvancementResult
CAnimTreeDoubleChild::AdvanceViewBothChildren(const CCharAnimTime& time, bool runLeft,
                                              bool loopLeft) {
  // TODO: Advance/simplify both children, respecting left-child looping, and accumulate root
  // motion.
  SAdvancementResults unchanged(time);
  return CDoubleChildAdvancementResult(CCharAnimTime(0.f), unchanged.mDeltas, unchanged.mDeltas);
}

void CAnimTreeDoubleChild::VSetPhase(float phase) {
  mA->VSetPhase(phase);
  mB->VSetPhase(phase);
}

SAdvancementResults
CAnimTreeDoubleChild::VGetAdvancementResults(const CCharAnimTime& time,
                                             const CCharAnimTime& startOffset) const {
  SAdvancementResults a = mA->VGetAdvancementResults(time, startOffset);
  SAdvancementResults b = mB->VGetAdvancementResults(time, startOffset);
  return a.mRemTime > b.mRemTime ? a : b;
}

rstl::rc_ptr< CAnimTreeNode > CAnimTreeDoubleChild::VGetBestUnblendedChild() const {
  rstl::rc_ptr< CAnimTreeNode > child = GetRightChildWeight() > 0.5f ? mB : mA;
  if (!child) {
    return child;
  }

  rstl::rc_ptr< CAnimTreeNode > best = child->GetBestUnblendedChild();
  return best ? best : child;
}

void CAnimTreeDoubleChild::VGetWeightedReaders(
    float weight, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const {
  mA->VGetWeightedReaders(weight, out);
  mB->VGetWeightedReaders(weight, out);
}
