#include "Kyoto/Animation/CAnimTreeAnimReaderContainer.hpp"

SAdvancementResults CAnimTreeAnimReaderContainer::VAdvanceView(const CCharAnimTime& time) {
  return mReader->VAdvanceView(time);
}

CCharAnimTime CAnimTreeAnimReaderContainer::VGetTimeRemaining() const {
  return mReader->VGetTimeRemaining();
}

CSteadyStateAnimInfo CAnimTreeAnimReaderContainer::VGetSteadyStateAnimInfo() const {
  return mReader->VGetSteadyStateAnimInfo();
}

bool CAnimTreeAnimReaderContainer::VHasOffset(const CSegId& seg) const {
  return mReader->VHasOffset(seg);
}

CVector3f CAnimTreeAnimReaderContainer::VGetOffset(const CSegId& seg) const {
  return mReader->VGetOffset(seg);
}

CQuaternion CAnimTreeAnimReaderContainer::VGetRotation(const CSegId& seg) const {
  return mReader->VGetRotation(seg);
}

uint CAnimTreeAnimReaderContainer::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
                                                   uint capacity, uint iterator,
                                                   int additive) const {
  return mReader->GetBoolPOIList(time, listOut, capacity, iterator, additive);
}

uint CAnimTreeAnimReaderContainer::VGetInt32POIList(const CCharAnimTime& time,
                                                    CInt32POINode* listOut, uint capacity,
                                                    uint iterator, int additive) const {
  return mReader->GetInt32POIList(time, listOut, capacity, iterator, additive);
}

uint CAnimTreeAnimReaderContainer::VGetParticlePOIList(const CCharAnimTime& time,
                                                       CParticlePOINode* listOut, uint capacity,
                                                       uint iterator, int additive) const {
  return mReader->GetParticlePOIList(time, listOut, capacity, iterator, additive);
}

uint CAnimTreeAnimReaderContainer::VGetSoundPOIList(const CCharAnimTime& time,
                                                    CSoundPOINode* listOut, uint capacity,
                                                    uint iterator, int additive) const {
  return mReader->GetSoundPOIList(time, listOut, capacity, iterator, additive);
}

bool CAnimTreeAnimReaderContainer::VGetBoolPOIState(uint nameHash) const {
  return mReader->VGetBoolPOIState(nameHash);
}

s32 CAnimTreeAnimReaderContainer::VGetInt32POIState(uint nameHash) const {
  return mReader->VGetInt32POIState(nameHash);
}

CParticleData::EParentedMode
CAnimTreeAnimReaderContainer::VGetParticlePOIState(uint nameHash) const {
  return mReader->VGetParticlePOIState(nameHash);
}

void CAnimTreeAnimReaderContainer::VGetSegStatementSet(const CSegIdList& list,
                                                       CSegStatementSet& setOut) const {
  mReader->VGetSegStatementSet(list, setOut);
}

void CAnimTreeAnimReaderContainer::VGetSegStatementSet(const CSegIdList& list,
                                                       CSegStatementSet& setOut,
                                                       const CCharAnimTime& time) const {
  mReader->VGetSegStatementSet(list, setOut, time);
}

void CAnimTreeAnimReaderContainer::VGetSegData(const CCharLayoutInfo& layout,
                                               CJointData_LinearStorage& data,
                                               const CCharAnimTime& time) const {
  mReader->VGetSegData(layout, data, time);
}

void CAnimTreeAnimReaderContainer::VGetSegData(const CCharLayoutInfo& layout,
                                               CJointData_LinearStorage& data) const {
  mReader->VGetSegData(layout, data);
}

rstl::ownership_transfer< IAnimReader > CAnimTreeAnimReaderContainer::VClone() const {
  return rs_new CAnimTreeAnimReaderContainer(mReader->Clone(), mName, mAnimDbIdx);
}

CAnimTreeEffectiveContribution
CAnimTreeAnimReaderContainer::VGetContributionOfHighestInfluence() const {
  return CAnimTreeEffectiveContribution(1.f, mName, mReader->VGetSteadyStateAnimInfo(),
                                        mReader->GetTimeRemaining(), mAnimDbIdx);
}

rstl::optional_object< rstl::ownership_transfer< IAnimReader > >
CAnimTreeAnimReaderContainer::VSimplified() {
  return rstl::optional_object_null();
}

void CAnimTreeAnimReaderContainer::VSetPhase(float phase) { mReader->VSetPhase(phase); }

SAdvancementResults
CAnimTreeAnimReaderContainer::VGetAdvancementResults(const CCharAnimTime& time,
                                                     const CCharAnimTime& startOffset) const {
  return mReader->VGetAdvancementResults(time, startOffset);
}

rstl::rc_ptr< CAnimTreeNode > CAnimTreeAnimReaderContainer::VGetBestUnblendedChild() const {
  return rstl::rc_ptr< CAnimTreeNode >();
}

void CAnimTreeAnimReaderContainer::VGetWeightedReaders(
    float weight, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const {
  out.push_back(rstl::pair< float, IAnimReader* >(weight, const_cast< IAnimReader* >(&*mReader)));
}
