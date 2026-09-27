#include "Kyoto/Animation/CAnimSourceReaderBase.hpp"

#include "Kyoto/Animation/CAnimPOIData.hpp"

uint CAnimSourceReaderBase::VGetBoolPOIList(const CCharAnimTime&, CBoolPOINode*, uint, uint,
                                            int) const {
  // TODO: Copy the pending bool events with times relative to mCurTime.
  return 0;
}

uint CAnimSourceReaderBase::VGetInt32POIList(const CCharAnimTime&, CInt32POINode*, uint, uint,
                                             int) const {
  // TODO: Copy the pending integer events with times relative to mCurTime.
  return 0;
}

uint CAnimSourceReaderBase::VGetParticlePOIList(const CCharAnimTime&, CParticlePOINode*, uint, uint,
                                                int) const {
  // TODO: Copy the pending particle events with times relative to mCurTime.
  return 0;
}

uint CAnimSourceReaderBase::VGetSoundPOIList(const CCharAnimTime&, CSoundPOINode*, uint, uint,
                                             int) const {
  // TODO: Copy the pending sound events with times relative to mCurTime.
  return 0;
}

bool CAnimSourceReaderBase::VGetBoolPOIState(uint nameHash) const {
  for (int i = 0; i < mBoolStates.size(); ++i) {
    if (mBoolStates[i].first == nameHash) {
      return mBoolStates[i].second;
    }
  }
  return false;
}

s32 CAnimSourceReaderBase::VGetInt32POIState(uint nameHash) const {
  for (int i = 0; i < mInt32States.size(); ++i) {
    if (mInt32States[i].first == nameHash) {
      return mInt32States[i].second;
    }
  }
  return 0;
}

CParticleData::EParentedMode CAnimSourceReaderBase::VGetParticlePOIState(uint nameHash) const {
  for (int i = 0; i < mParticleStates.size(); ++i) {
    if (mParticleStates[i].first == nameHash) {
      return mParticleStates[i].second;
    }
  }
  return CParticleData::kPM_Initial;
}

void CAnimSourceReaderBase::UpdatePOIStates() {
  // TODO: Consume each event stream up to mCurTime and update its indexed state.
}

rstl::set< rstl::pair< uint, int > > CAnimSourceReaderBase::GetUniqueBoolPOIs() const {
  rstl::set< rstl::pair< uint, int > > result;
  const rstl::vector< CBoolPOINode >& nodes = mPOIData->GetBoolPOIStream();
  for (int i = 0; i < nodes.size(); ++i) {
    if (nodes[i].GetSaveState()) {
      result.insert(rstl::pair< uint, int >(nodes[i].GetNameHash(), nodes[i].GetIndex()));
    }
  }
  return result;
}

rstl::set< rstl::pair< uint, int > > CAnimSourceReaderBase::GetUniqueInt32POIs() const {
  rstl::set< rstl::pair< uint, int > > result;
  const rstl::vector< CInt32POINode >& nodes = mPOIData->GetInt32POIStream();
  for (int i = 0; i < nodes.size(); ++i) {
    if (nodes[i].GetSaveState()) {
      result.insert(rstl::pair< uint, int >(nodes[i].GetNameHash(), nodes[i].GetIndex()));
    }
  }
  return result;
}

rstl::set< rstl::pair< uint, int > > CAnimSourceReaderBase::GetUniqueParticlePOIs() const {
  rstl::set< rstl::pair< uint, int > > result;
  const rstl::vector< CParticlePOINode >& nodes = mPOIData->GetParticlePOIStream();
  for (int i = 0; i < nodes.size(); ++i) {
    if (nodes[i].GetSaveState()) {
      result.insert(rstl::pair< uint, int >(nodes[i].GetNameHash(), nodes[i].GetIndex()));
    }
  }
  return result;
}

void CAnimSourceReaderBase::PostConstruct(const CCharAnimTime& time) {
  mPassedBoolCount = 0;
  mPassedIntCount = 0;
  mPassedParticleCount = 0;
  mPassedSoundCount = 0;
  // TODO: Build indexed POI states and advance to the requested starting time.
}
