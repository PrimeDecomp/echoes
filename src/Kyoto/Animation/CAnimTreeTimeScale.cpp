#include "Kyoto/Animation/CAnimTreeTimeScale.hpp"

SAdvancementResults CAnimTreeTimeScale::VAdvanceView(const CCharAnimTime& time) {
  // TODO: Integrate the time-scale function and translate the child's remainder back to real time.
  return SAdvancementResults(time);
}

CCharAnimTime CAnimTreeTimeScale::VGetTimeRemaining() const {
  // TODO: Convert remaining child time through the integral's inverse.
  return mChild->VGetTimeRemaining();
}

CSteadyStateAnimInfo CAnimTreeTimeScale::VGetSteadyStateAnimInfo() const {
  // TODO: Adjust duration for the acceleration interval and mInitialTime.
  return mChild->VGetSteadyStateAnimInfo();
}

rstl::ownership_transfer< IAnimReader > CAnimTreeTimeScale::VClone() const {
  return rs_new CAnimTreeTimeScale(Cast(mChild->VClone()), mTimeScale->Clone(), mCurAccelTime,
                                   mTargetAccelTime, mInitialTime, mName);
}

rstl::rc_ptr< CAnimTreeNode > CAnimTreeTimeScale::VGetBestUnblendedChild() const {
  rstl::rc_ptr< CAnimTreeNode > child = mChild->GetBestUnblendedChild();
  if (child) {
    return rs_new CAnimTreeTimeScale(Cast(child->VClone()), mTimeScale->Clone(), mCurAccelTime,
                                     mTargetAccelTime, mInitialTime, mName);
  }
  return child;
}

CAnimTreeEffectiveContribution CAnimTreeTimeScale::VGetContributionOfHighestInfluence() const {
  CAnimTreeEffectiveContribution child = mChild->GetContributionOfHighestInfluence();
  return CAnimTreeEffectiveContribution(child.GetContributionWeight(), child.GetPrimitiveName(),
                                        VGetSteadyStateAnimInfo(), VGetTimeRemaining(),
                                        child.GetAnimDatabaseIndex());
}

uint CAnimTreeTimeScale::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
                                         uint capacity, uint iterator, int additive) const {
  // TODO: Query the child in scaled time and convert each returned POI timestamp.
  return 0;
}

uint CAnimTreeTimeScale::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut,
                                          uint capacity, uint iterator, int additive) const {
  // TODO: Query the child in scaled time and convert each returned POI timestamp.
  return 0;
}

uint CAnimTreeTimeScale::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
                                             uint capacity, uint iterator, int additive) const {
  // TODO: Query the child in scaled time and convert each returned POI timestamp.
  return 0;
}

uint CAnimTreeTimeScale::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
                                          uint capacity, uint iterator, int additive) const {
  // TODO: Query the child in scaled time and convert each returned POI timestamp.
  return 0;
}

bool CAnimTreeTimeScale::VGetBoolPOIState(uint nameHash) const {
  return mChild->VGetBoolPOIState(nameHash);
}

s32 CAnimTreeTimeScale::VGetInt32POIState(uint nameHash) const {
  return mChild->VGetInt32POIState(nameHash);
}

CParticleData::EParentedMode CAnimTreeTimeScale::VGetParticlePOIState(uint nameHash) const {
  return mChild->VGetParticlePOIState(nameHash);
}

rstl::optional_object< rstl::ownership_transfer< IAnimReader > > CAnimTreeTimeScale::VSimplified() {
  // TODO: Preserve time-scale state when simplifying the child; remove completed scaling.
  return rstl::optional_object_null();
}

void CAnimTreeTimeScale::VSetPhase(float phase) { mChild->VSetPhase(phase); }

CCharAnimTime CAnimTreeTimeScale::GetRealLifeTime(const CCharAnimTime& time) const {
  // TODO: Resolve the integral across the remaining acceleration interval.
  return time;
}

rstl::string CAnimTreeTimeScale::CreatePrimitiveName(const rstl::ncrc_ptr< CAnimTreeNode >& node,
                                                     float scaleA, const CCharAnimTime& time,
                                                     float scaleB) {
  return rstl::string_l("");
}
