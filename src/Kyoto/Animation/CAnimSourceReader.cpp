#include "Kyoto/Animation/CAnimSourceReader.hpp"

CAnimSourceReader::CAnimSourceReader(const TSubAnimTypeToken< CAnimSource >& source,
                                     const CCharAnimTime& time, const CAnimPOIData* poiData)
: CAnimSourceReaderBase(rs_new CAnimSourceInfo(source), poiData)
, mSource(source)
, mSteadyStateInfo(mSource->GetSteadyStateAnimInfo(time)) {
  PostConstruct(time);
}

SAdvancementResults CAnimSourceReader::VAdvanceView(const CCharAnimTime& time) {
  // TODO: Advance time, process POIs, and extract root-motion deltas.
  return SAdvancementResults(time);
}

CCharAnimTime CAnimSourceReader::VGetTimeRemaining() const {
  return mSource->GetAnimationDuration() - mCurTime;
}

CSteadyStateAnimInfo CAnimSourceReader::VGetSteadyStateAnimInfo() const { return mSteadyStateInfo; }

bool CAnimSourceReader::VHasOffset(const CSegId& seg) const { return mSource->HasOffset(seg); }

CVector3f CAnimSourceReader::VGetOffset(const CSegId& seg) const {
  return mSource->GetOffset(seg, mCurTime);
}

CVector3f CAnimSourceReader::VGetOffset(const CSegId& seg, const CCharAnimTime& time) const {
  return mSource->GetOffset(seg, time);
}

CQuaternion CAnimSourceReader::VGetRotation(const CSegId& seg) const {
  return mSource->GetRotation(seg, mCurTime);
}

void CAnimSourceReader::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& set) const {
  mSource->GetSegStatementSet(list, set, mCurTime);
}

void CAnimSourceReader::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& set,
                                            const CCharAnimTime& time) const {
  mSource->GetSegStatementSet(list, set, time);
}

void CAnimSourceReader::VGetSegData(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                                    const CCharAnimTime& time) const {
  mSource->GetSegData(layout, data, time);
}

void CAnimSourceReader::VGetSegData(const CCharLayoutInfo& layout,
                                    CJointData_LinearStorage& data) const {
  mSource->GetSegData(layout, data, mCurTime);
}

rstl::ownership_transfer< IAnimReader > CAnimSourceReader::VClone() const {
  return rs_new CAnimSourceReader(mSource, mPOIData, mCurTime, mSteadyStateInfo, mPassedBoolCount,
                                  mPassedIntCount, mPassedParticleCount, mPassedSoundCount,
                                  mBoolStates, mInt32States, mParticleStates);
}

SAdvancementResults CAnimSourceReader::VReverseView(const CCharAnimTime& time) {
  // TODO: Reverse time and extract the corresponding root-motion deltas.
  return SAdvancementResults(time);
}

void CAnimSourceReader::VSetPhase(float phase) {
  mCurTime =
      CCharAnimTime(phase * mSource->GetSteadyStateAnimInfo(mCurTime).GetDuration().GetSeconds());
  UpdatePOIStates();
  if (!mCurTime.GreaterThanZero()) {
    mPassedBoolCount = 0;
    mPassedIntCount = 0;
    mPassedParticleCount = 0;
    mPassedSoundCount = 0;
  }
}

bool CAnimSourceReader::VSupportsReverseView() const { return true; }

SAdvancementResults
CAnimSourceReader::VGetAdvancementResults(const CCharAnimTime& time,
                                          const CCharAnimTime& startOffset) const {
  // TODO: Sample root-motion deltas without mutating the reader.
  return SAdvancementResults(time);
}

CAnimSourceReader::~CAnimSourceReader() {}
