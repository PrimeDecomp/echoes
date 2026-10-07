#include "Kyoto/Animation/CAnimTreeDoubleChild.hpp"
#include "Kyoto/Animation/CBoolPOINode.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
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
  count += mB->GetBoolPOIList(time, listOut, capacity, count + iterator, additive);
  if (count > capacity) {
    count = capacity;
  }
  qsort(listOut, count, sizeof(CBoolPOINode), CPOINode::compare);
  return count;
}

uint CAnimTreeDoubleChild::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut,
                                            uint capacity, uint iterator, int additive) const {
  uint count = mA->GetInt32POIList(time, listOut, capacity, iterator, additive);
  count += mB->GetInt32POIList(time, listOut, capacity, count + iterator, additive);
  if (count > capacity) {
    count = capacity;
  }
  qsort(listOut, count, sizeof(CInt32POINode), CPOINode::compare);
  return count;
}

uint CAnimTreeDoubleChild::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
                                               uint capacity, uint iterator, int additive) const {
  uint count = mA->GetParticlePOIList(time, listOut, capacity, iterator, additive);
  count += mB->GetParticlePOIList(time, listOut, capacity, count + iterator, additive);
  if (count > capacity) {
    count = capacity;
  }
  qsort(listOut, count, sizeof(CParticlePOINode), CPOINode::compare);
  return count;
}

uint CAnimTreeDoubleChild::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
                                            uint capacity, uint iterator, int additive) const {
  uint count = mA->GetSoundPOIList(time, listOut, capacity, iterator, additive);
  count += mB->GetSoundPOIList(time, listOut, capacity, count + iterator, additive);
  if (count > capacity) {
    count = capacity;
  }
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
  const float leftWeight = a.GetContributionWeight() * GetLeftChildWeight();
  const float rightWeight = b.GetContributionWeight() * GetRightChildWeight();
  return leftWeight > rightWeight
             ? CAnimTreeEffectiveContribution(leftWeight, a.GetPrimitiveName(),
                                              a.GetSteadyStateAnimInfo(), a.GetTimeRemaining(),
                                              a.GetAnimDatabaseIndex())
             : CAnimTreeEffectiveContribution(rightWeight, b.GetPrimitiveName(),
                                              b.GetSteadyStateAnimInfo(), b.GetTimeRemaining(),
                                              b.GetAnimDatabaseIndex());
}

uint CAnimTreeDoubleChild::VGetNumChildren() const {
  const uint rightCount = mB->VGetNumChildren();
  return mA->VGetNumChildren() + 2 + rightCount;
}

CAnimTreeDoubleChild::CDoubleChildAdvancementResult::CDoubleChildAdvancementResult(
    const CCharAnimTime& time, const SAdvancementDeltas& left, const SAdvancementDeltas& right)
: mTrueAdvancement(time), mLeftDeltas(left), mRightDeltas(right) {}

CAnimTreeDoubleChild::CDoubleChildAdvancementResult
CAnimTreeDoubleChild::AdvanceViewBothChildren(const CCharAnimTime& time, bool runLeft,
                                              bool loopLeft) {
  CCharAnimTime leftRemaining = time;
  CCharAnimTime totalTime = !runLeft   ? CCharAnimTime::ZeroFlat()
                            : loopLeft ? CCharAnimTime::Infinity()
                                       : mA->GetTimeRemaining();
  CVector3f leftOffset(0.f, 0.f, 0.f);
  CQuaternion leftRotation = CQuaternion::NoRotation();
  CCharAnimTime rightRemaining = time;
  CVector3f rightOffset(0.f, 0.f, 0.f);
  CQuaternion rightRotation = CQuaternion::NoRotation();

  if (time.GreaterThanZero()) {
    while (leftRemaining.GreaterThanZero() && !close_enough(leftRemaining.GetSeconds(), 0.f) &&
           totalTime.GreaterThanZero() &&
           (loopLeft || !close_enough(totalTime.GetSeconds(), 0.f))) {
      SAdvancementResults result = mA->AdvanceView(leftRemaining);
      rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simplified =
          mA->Simplified();
      if (simplified.valid())
        mA = Cast(*simplified);
      SAdvancementDeltas deltas = result.mDeltas;
      leftOffset += deltas.GetOffsetDelta();
      CQuaternion rotation = deltas.GetOrientationDelta();
      leftRotation *= rotation;
      if (!loopLeft)
        totalTime = mA->GetTimeRemaining();
      leftRemaining = result.mRemTime;
    }

    while (rightRemaining.GreaterThanZero() && !close_enough(rightRemaining.GetSeconds(), 0.f)) {
      SAdvancementResults result = mB->AdvanceView(rightRemaining);
      rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simplified =
          mB->Simplified();
      if (simplified.valid())
        mB = Cast(*simplified);
      SAdvancementDeltas deltas = result.mDeltas;
      rightOffset += deltas.GetOffsetDelta();
      CQuaternion rotation = deltas.GetOrientationDelta();
      rightRotation *= rotation;
      rightRemaining = result.mRemTime;
    }
  }

  return CDoubleChildAdvancementResult(time, SAdvancementDeltas(leftOffset, leftRotation),
                                       SAdvancementDeltas(rightOffset, rightRotation));
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
  if (!best) {
    return child;
  }
  return best;
}

void CAnimTreeDoubleChild::VGetWeightedReaders(
    float weight, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const {
  mA->VGetWeightedReaders(weight, out);
  mB->VGetWeightedReaders(weight, out);
}
