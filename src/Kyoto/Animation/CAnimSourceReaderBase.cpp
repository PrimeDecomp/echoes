#include "Kyoto/Animation/CAnimSourceReaderBase.hpp"

#include "Kyoto/Animation/CAnimPOIData.hpp"
#include "rstl/math.hpp"

template < class T >
uint _getPOIList(const CCharAnimTime& time, T* listOut, uint capacity, uint iterator, int additive,
                 const rstl::vector< T >& stream, const CCharAnimTime& curTime,
                 const IAnimSourceInfo& sourceInfo, int passedCount) {
  uint ret = 0;
  int count = stream.size();
  if (count > 0) {
    const CCharAnimTime& duration = sourceInfo.GetAnimationDuration();
    CCharAnimTime totalTime = curTime + time;
    CCharAnimTime endTime = rstl::min_val(duration, totalTime);
    if (passedCount < count) {
      int index = passedCount;
      const int initialIndex = index;
      CCharAnimTime nodeTime(stream[initialIndex].GetTime());
      while (index < count && nodeTime <= endTime) {
        const T& node = stream[index];
        if (ret + iterator < capacity) {
          listOut[iterator + ret] = T::CopyNodeMinusStartTime(node, curTime);
          ++ret;
        }
        ++index;
        if (index < count) {
          nodeTime = stream[index].GetTime();
        }
      }
    }
  }
  return ret;
}

uint CAnimSourceReaderBase::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
                                            uint capacity, uint iterator, int additive) const {
  return _getPOIList(time, listOut, capacity, iterator, additive, mPOIData->GetBoolPOIStream(),
                     mCurTime, *mSourceInfo, mPassedBoolCount);
}

uint CAnimSourceReaderBase::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut,
                                             uint capacity, uint iterator, int additive) const {
  return _getPOIList(time, listOut, capacity, iterator, additive, mPOIData->GetInt32POIStream(),
                     mCurTime, *mSourceInfo, mPassedIntCount);
}

uint CAnimSourceReaderBase::VGetParticlePOIList(const CCharAnimTime& time,
                                                CParticlePOINode* listOut, uint capacity,
                                                uint iterator, int additive) const {
  return _getPOIList(time, listOut, capacity, iterator, additive, mPOIData->GetParticlePOIStream(),
                     mCurTime, *mSourceInfo, mPassedParticleCount);
}

uint CAnimSourceReaderBase::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
                                             uint capacity, uint iterator, int additive) const {
  return _getPOIList(time, listOut, capacity, iterator, additive, mPOIData->GetSoundPOIStream(),
                     mCurTime, *mSourceInfo, mPassedSoundCount);
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
  const rstl::vector< CBoolPOINode >& boolNodes = mPOIData->GetBoolPOIStream();
  const rstl::vector< CInt32POINode >& int32Nodes = mPOIData->GetInt32POIStream();
  const rstl::vector< CParticlePOINode >& particleNodes = mPOIData->GetParticlePOIStream();
  const rstl::vector< CSoundPOINode >& soundNodes = mPOIData->GetSoundPOIStream();
  int boolCount = boolNodes.size();
  int int32Count = int32Nodes.size();
  int particleCount = particleNodes.size();
  int soundCount = soundNodes.size();
  while (mPassedBoolCount < boolCount && boolNodes[mPassedBoolCount].GetTime() <= mCurTime) {
    const CBoolPOINode& node = boolNodes[mPassedBoolCount];
    int index = node.GetIndex();
    if (index >= 0 && index < mBoolStates.size()) {
      mBoolStates[index].second = node.GetValue();
    }
    ++mPassedBoolCount;
  }
  while (mPassedIntCount < int32Count && int32Nodes[mPassedIntCount].GetTime() <= mCurTime) {
    const CInt32POINode& node = int32Nodes[mPassedIntCount];
    int index = node.GetIndex();
    if (index >= 0 && index < mInt32States.size()) {
      mInt32States[index].second = node.GetValue();
    }
    ++mPassedIntCount;
  }
  while (mPassedParticleCount < particleCount &&
         particleNodes[mPassedParticleCount].GetTime() <= mCurTime) {
    const CParticlePOINode& node = particleNodes[mPassedParticleCount];
    int index = node.GetIndex();
    if (index >= 0 && index < mParticleStates.size()) {
      mParticleStates[index] = rstl::pair< uint, CParticleData::EParentedMode >(
          mParticleStates[index].first, node.GetParticleData().GetParentedMode());
    }
    ++mPassedParticleCount;
  }
  while (mPassedSoundCount < soundCount && soundNodes[mPassedSoundCount].GetTime() <= mCurTime) {
    ++mPassedSoundCount;
  }
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
  const rstl::set< rstl::pair< uint, int > > boolPOIs = GetUniqueBoolPOIs();
  const rstl::set< rstl::pair< uint, int > > int32POIs = GetUniqueInt32POIs();
  const rstl::set< rstl::pair< uint, int > > particlePOIs = GetUniqueParticlePOIs();
  int boolCount = boolPOIs.size();
  int int32Count = int32POIs.size();
  int particleCount = particlePOIs.size();
  mBoolStates.resize(boolCount, rstl::pair< uint, bool >(uint(-1), false));
  mInt32States.resize(int32Count, rstl::pair< uint, int >(uint(-1), 0));
  mParticleStates.resize(particleCount, rstl::pair< uint, CParticleData::EParentedMode >(
                                            uint(-1), CParticleData::kPM_Initial));
  for (rstl::set< rstl::pair< uint, int > >::const_iterator it = boolPOIs.begin();
       it != boolPOIs.end();) {
    uint name = it->first;
    int index = it->second;
    if (index >= 0 && index < mBoolStates.size()) {
      mBoolStates[index] = rstl::pair< uint, bool >(name, false);
    }
    ++it;
  }
  for (rstl::set< rstl::pair< uint, int > >::const_iterator it = int32POIs.begin();
       it != int32POIs.end();) {
    uint name = it->first;
    int index = it->second;
    if (index >= 0 && index < mInt32States.size()) {
      mInt32States[index] = rstl::pair< uint, int >(name, 0);
    }
    ++it;
  }
  for (rstl::set< rstl::pair< uint, int > >::const_iterator it = particlePOIs.begin();
       it != particlePOIs.end();) {
    uint name = it->first;
    int index = it->second;
    if (index >= 0 && index < mParticleStates.size()) {
      mParticleStates[index] =
          rstl::pair< uint, CParticleData::EParentedMode >(name, CParticleData::kPM_Initial);
    }
    ++it;
  }
  CCharAnimTime remaining = time;
  if (remaining.GreaterThanZero()) {
    while (remaining.GreaterThanZero()) {
      remaining = VAdvanceView(remaining).GetRemainder();
    }
  } else {
    UpdatePOIStates();
    if (!time.GreaterThanZero()) {
      mPassedBoolCount = 0;
      mPassedIntCount = 0;
      mPassedParticleCount = 0;
      mPassedSoundCount = 0;
    }
  }
}
