#include "Kyoto/Animation/CFBStreamedAnimReader.hpp"

#include "Kyoto/Animation/CAnimMathUtils.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CJointData_LinearStorage.hpp"
#include "Kyoto/Animation/CSegIdList.hpp"
#include "Kyoto/Animation/CSegStatementSet.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Math/CMath.hpp"

const uchar CFBStreamedAnimReaderTotals::kRotationValueCount = 4;
const uchar CFBStreamedAnimReaderTotals::kOffsetValueCount = 4;

// The target emits these CFBStreamedCompression members in this translation unit.
CCharAnimTime CFBStreamedCompression::GetAnimationDuration() const {
  return MainHeader().GetMaxTime();
}

bool CFBStreamedCompression::HasScaleData() const {
  return GetPerChannelHeaderList(TimeHeader(MainHeader())).HasScaleData();
}

bool CFBStreamedPerChannelHeaderList::HasOffsetData() const {
  for (const_iterator it = begin(); it != end(); ++it) {
    if (it->GetOffsetBitStorage().GetWidth() != 0) {
      return true;
    }
  }
  return false;
}

bool CFBStreamedPerChannelHeaderList::HasScaleData() const {
  for (const_iterator it = begin(); it != end(); ++it) {
    if (it->GetScaleBitStorage().GetWidth() != 0) {
      return true;
    }
  }
  return false;
}

CFBStreamedAnimReaderTotals::CFBStreamedAnimReaderTotals(const CFBStreamedCompression& source)
: mBuffer(nullptr)
, mCumulativeInts(nullptr)
, mHasRotation(nullptr)
, mHasOffset(nullptr)
, mHasScale(nullptr)
, mSegIds(nullptr)
, mComputedFloats(nullptr)
, mRotDiv(source.MainHeader().GetRotationValueForOne())
, mOffsetMult(source.MainHeader().GetOffsetResolution())
, mScaleMult(source.MainHeader().GetScaleResolution())
, mCurKey(0)
, mCalculated(false)
, mBoneChanCount(source.GetPerChannelHeaderList(source.TimeHeader(source.MainHeader())).size())
, mHasOffsetData(
      source.GetPerChannelHeaderList(source.TimeHeader(source.MainHeader())).HasOffsetData())
, mHasScaleData(
      source.GetPerChannelHeaderList(source.TimeHeader(source.MainHeader())).HasScaleData())
, mValuesPerChannel(GetValuesPerChannel()) {
  Allocate(mBoneChanCount);
  SetToReadStart(source);
}

uchar CFBStreamedAnimReaderTotals::GetValuesPerChannel() const {
  uchar values = 4;
  if (mHasOffsetData) {
    values = 8;
  }
  if (mHasScaleData) {
    values += 4;
  }
  return static_cast< uchar >(values);
}

void CFBStreamedAnimReaderTotals::Allocate(uint channelCount) {
  const uint shortsSize =
      channelCount * 2 * mValuesPerChannel + (4 - (channelCount * 2 * mValuesPerChannel) % 4);
  const uint rotationFlagsSize = channelCount + (4 - channelCount % 4);
  const uint offsetFlagsSize = channelCount + (4 - channelCount % 4);
  const uint scaleFlagsSize = channelCount + (4 - channelCount % 4);
  const uint idsSize = channelCount * 2 + (4 - (channelCount * 2) % 4);
  const uint floatsSize =
      channelCount * 4 * mValuesPerChannel + (4 - (channelCount * 4 * mValuesPerChannel) % 4);
  const uint size =
      shortsSize + rotationFlagsSize + offsetFlagsSize + scaleFlagsSize + idsSize + floatsSize;
  const uint bufferSize = size + (4 - size % 4);
  mBuffer = rs_new uchar[bufferSize];
  mBufferSize = bufferSize;
  CCharAnimMemoryMetrics::AddToTotalSize(mBufferSize, CCharAnimMemoryMetrics::kASS_Two);
  uint offset = 0;
  mCumulativeInts = reinterpret_cast< short* >(mBuffer + offset);
  offset += shortsSize;
  mHasRotation = reinterpret_cast< bool* >(mBuffer + offset);
  offset += rotationFlagsSize;
  mHasOffset = reinterpret_cast< bool* >(mBuffer + offset);
  offset += offsetFlagsSize;
  mHasScale = reinterpret_cast< bool* >(mBuffer + offset);
  offset += scaleFlagsSize;
  mSegIds = reinterpret_cast< short* >(mBuffer + offset);
  offset += idsSize;
  mComputedFloats = reinterpret_cast< float* >(mBuffer + offset);
}

CFBStreamedAnimReaderTotals::~CFBStreamedAnimReaderTotals() {
  if (mBuffer != nullptr) {
    delete[] mBuffer;
  }
  CCharAnimMemoryMetrics::SubtractFromTotalSize(mBufferSize, CCharAnimMemoryMetrics::kASS_Two);
}

void CFBStreamedAnimReaderTotals::SetToReadStart(const CFBStreamedCompression& source) {
  mCurKey = 0;
  mCalculated = false;
  const CFBStreamedPerChannelHeaderList& channels =
      source.GetPerChannelHeaderList(source.TimeHeader(source.MainHeader()));
  mBoneChanCount = channels.size();
  short* values = mCumulativeInts;
  uint channel = 0;
  for (CFBStreamedPerChannelHeaderList::const_iterator it = channels.begin(); it != channels.end();
       ++it, ++channel) {
    mSegIds[channel] = it->GetSegId().val();
    const CFBStreamedPerChannelHeader::RotationHeader& rotation = it->GetRotationBitStorage();
    for (uint i = 0; i < 4; ++i) {
      *values++ = rotation.GetInitialValue(i);
    }
    mHasRotation[channel] = rotation.GetWidth() != 0;
    const CFBStreamedPerChannelHeader::OffsetHeader& offset = it->GetOffsetBitStorage();
    for (uint i = 0; i < 3; ++i) {
      *values++ = offset.GetInitialValue(i);
    }
    ++values;
    mHasOffset[channel] = offset.GetWidth() != 0;
    if (mHasScaleData) {
      const CFBStreamedPerChannelHeader::ScaleHeader& scale = it->GetScaleBitStorage();
      for (uint i = 0; i < 3; ++i) {
        *values++ = scale.GetInitialValue(i);
      }
      ++values;
      mHasScale[channel] = scale.GetWidth() != 0;
    } else {
      mHasScale[channel] = false;
    }
  }
}

void CFBStreamedAnimReaderTotals::CalculateDown() {
  const float rotationScale = (M_PIF / 2.f) / mRotDiv;
  const short* values = mCumulativeInts;
  float* computed = mComputedFloats;
  for (uint i = 0; i < mBoneChanCount; ++i) {
    if (mHasRotation[i]) {
      computed[1] = CMath::FastSinR(rotationScale * CCast::StoF(values[1]));
      computed[2] = CMath::FastSinR(rotationScale * CCast::StoF(values[2]));
      computed[3] = CMath::FastSinR(rotationScale * CCast::StoF(values[3]));
      float w = CMath::SqrtF(
          CMath::Max(0.f, 1.f - (computed[1] * computed[1] + computed[2] * computed[2] +
                                 computed[3] * computed[3])));
      if (values[0] != 0) {
        computed[0] = -w;
      } else {
        computed[0] = w;
      }
    }
    if (mHasOffset[i]) {
      computed[4] = values[4] * mOffsetMult;
      computed[5] = values[5] * mOffsetMult;
      computed[6] = values[6] * mOffsetMult;
    }
    values += 8;
    computed += 8;
    if (mHasScaleData) {
      if (mHasScale[i]) {
        computed[0] = values[0] * mScaleMult;
        computed[1] = values[1] * mScaleMult;
        computed[2] = values[2] * mScaleMult;
      }
      values += 4;
      computed += 4;
    }
  }
  mCalculated = true;
}

CFBStreamedPairOfTotals::CFBStreamedPairOfTotals(
    const TSubAnimTypeToken< CFBStreamedCompression >& source)
: mSource(source)
, mNextSel(true)
, mA(*source)
, mB(*source)
, mAspects(source->TimeHeader(source->MainHeader()),
           CTimeRemainderAndFraction(CCharAnimTime::ZeroFlat(), source->FinestSample()),
           source->GetAnimationDuration())
, mCurKey(0) {}

void CFBStreamedPairOfTotals::SetTime(CMemoryInputToBitLevelLoader& input,
                                      CBitLevelLoader< CMemoryInputToBitLevelLoader >& loader,
                                      const CCharAnimTime& time) {
  mAspects.SetTime(CTimeRemainderAndFraction(time, mSource->FinestSample()));
  const CFBStreamedCompression& source = *mSource;
  const uint prevIndex = mAspects.GetPrevIndex();
  const CFBStreamedPerChannelHeaderList& channels =
      source.GetPerChannelHeaderList(source.TimeHeader(source.MainHeader()));
  if (Prior().GetFrameNumber() > prevIndex) {
    input = CMemoryInputToBitLevelLoader(source.GetBytes(channels));
    loader = CBitLevelLoader< CMemoryInputToBitLevelLoader >(input);
    Prior().SetToReadStart(source);
    mCurKey = 0;
    Prior().IncrementInto(loader, source, Next());
    ++mCurKey;
  } else if (Next().GetFrameNumber() != Prior().GetFrameNumber() + 1) {
    DoIncrement(loader);
  }
  while (Prior().GetFrameNumber() < prevIndex) {
    mNextSel = !mNextSel;
    DoIncrement(loader);
  }
}

void CFBStreamedPairOfTotals::DoIncrement(CBitLevelLoader< CMemoryInputToBitLevelLoader >& loader) {
  const CFBStreamedCompression& source = *mSource;
  ++mCurKey;
  Prior().IncrementInto(loader, source, Next());
}

float CFBStreamedPairOfTotals::GetT() const { return mAspects.GetT(); }

void CFBStreamedAnimReaderTotals::IncrementInto(
    CBitLevelLoader< CMemoryInputToBitLevelLoader >& loader, const CFBStreamedCompression& source,
    CFBStreamedAnimReaderTotals& out) {
  out.mCalculated = false;
  const CFBStreamedPerChannelHeaderList& channels =
      source.GetPerChannelHeaderList(source.TimeHeader(source.MainHeader()));
  const short* input = mCumulativeInts;
  short* output = out.mCumulativeInts;
  uint channel = 0;
  for (CFBStreamedPerChannelHeaderList::const_iterator it = channels.begin(); it != channels.end();
       ++it, ++channel) {
    if (mHasRotation[channel]) {
      const CFBStreamedPerChannelHeader::RotationHeader& rotation = it->GetRotationBitStorage();
      output[0] = loader.LoadUnsigned(1);
      output[1] = input[1] + loader.LoadSigned(rotation.GetBitCount(1));
      output[2] = input[2] + loader.LoadSigned(rotation.GetBitCount(2));
      output[3] = input[3] + loader.LoadSigned(rotation.GetBitCount(3));
    }
    if (mHasOffset[channel]) {
      const CFBStreamedPerChannelHeader::OffsetHeader& offset = it->GetOffsetBitStorage();
      output[4] = input[4] + loader.LoadSigned(offset.GetBitCount(0));
      output[5] = input[5] + loader.LoadSigned(offset.GetBitCount(1));
      output[6] = input[6] + loader.LoadSigned(offset.GetBitCount(2));
    }
    output += 8;
    input += 8;
    if (mHasScaleData) {
      if (mHasScale[channel]) {
        const CFBStreamedPerChannelHeader::ScaleHeader& scale = it->GetScaleBitStorage();
        output[0] = input[0] + loader.LoadSigned(scale.GetBitCount(0));
        output[1] = input[1] + loader.LoadSigned(scale.GetBitCount(1));
        output[2] = input[2] + loader.LoadSigned(scale.GetBitCount(2));
      }
      output += 4;
      input += 4;
    }
  }
  out.mCurKey = mCurKey + 1;
}

CSegIdToIndexConverter::~CSegIdToIndexConverter() {
  CCharAnimMemoryMetrics::SubtractFromTotalSize(sizeof(mIndices), CCharAnimMemoryMetrics::kASS_Two);
}

CSegIdToIndexConverter::CSegIdToIndexConverter(const CFBStreamedAnimReaderTotals& totals) {
  for (uint i = 0; i < 100; ++i) {
    mIndices[i] = ~0u;
  }
  uint count = totals.NumEntries();
  for (uint i = 0; i < count; ++i) {
    const uint segId = totals.GetSegId(i);
    mIndices[segId] = i;
  }
  CCharAnimMemoryMetrics::AddToTotalSize(sizeof(mIndices), CCharAnimMemoryMetrics::kASS_Two);
}

CFBStreamedAnimReader::CFBStreamedAnimReader(
    const TSubAnimTypeToken< CFBStreamedCompression >& source, CCharAnimTime time,
    const CAnimPOIData* poiData)
: CAnimSourceReaderBase(rs_new TAnimSourceInfo< CFBStreamedCompression >(source), poiData)
, mSource(source)
, mSteadyStateInfo(mSource->GetSteadyStateAnimInfo())
, mTotals(source)
, mInput(
      source->GetBytes(source->GetPerChannelHeaderList(source->TimeHeader(source->MainHeader()))))
, mBitLoader(mInput)
, mSegIdToIndex(mTotals.Prior()) {
  PostConstruct(time);
}

CFBStreamedAnimReader::~CFBStreamedAnimReader() {}

rstl::ownership_transfer< IAnimReader > CFBStreamedAnimReader::VClone() const {
  return rstl::ownership_transfer< IAnimReader >(
      rs_new CFBStreamedAnimReader(mSource, mCurTime, mPOIData));
}

CCharAnimTime CFBStreamedAnimReader::VGetTimeRemaining() const {
  return mSource->GetAnimationDuration() - mCurTime;
}

CSteadyStateAnimInfo CFBStreamedAnimReader::VGetSteadyStateAnimInfo() const {
  return mSteadyStateInfo;
}

bool CFBStreamedAnimReader::VHasOffset(const CSegId& seg) const {
  const uint index = mSegIdToIndex.SegIdToIndex(seg.val());
  if (index == ~0u) {
    return false;
  }
  return mTotals.Next().HasOffset(index);
}

CVector3f CFBStreamedAnimReader::VGetOffset(const CSegId& seg) const {
  SetReadTime(mCurTime);
  const uint index = mSegIdToIndex.SegIdToIndex(seg.val());
  if (index == ~0u) {
    return CVector3f::Zero();
  }
  return CVector3f::Lerp(mTotals.Prior().GetVector(index), mTotals.Next().GetVector(index),
                         mTotals.GetT());
}

CQuaternion CFBStreamedAnimReader::VGetRotation(const CSegId& seg) const {
  SetReadTime(mCurTime);
  const uint index = mSegIdToIndex.SegIdToIndex(seg.val());
  if (mSegIdToIndex.SegIdToIndex(index) == ~0u) {
    return CQuaternion::NoRotation();
  }
  return CQuaternion::Slerp(mTotals.Prior().GetQuat(index), mTotals.Next().GetQuat(index),
                            mTotals.GetT());
}

bool CFBStreamedAnimReader::VSupportsReverseView() const { return false; }

SAdvancementResults CFBStreamedAnimReader::VReverseView(const CCharAnimTime&) {
  return SAdvancementResults(CCharAnimTime::ZeroFlat(),
                             SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
}

SAdvancementResults CFBStreamedAnimReader::VAdvanceView(const CCharAnimTime& time) {
  const CCharAnimTime curTime = mCurTime;
  const CCharAnimTime& duration = mSource->GetAnimationDuration();
  if (curTime == duration) {
    mCurTime = CCharAnimTime::ZeroFlat();
    SetReadTime(mCurTime);
    mPassedBoolCount = 0;
    mPassedIntCount = 0;
    mPassedParticleCount = 0;
    mPassedSoundCount = 0;
    return SAdvancementResults(time,
                               SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
  }
  if (time.EqualsZero()) {
    return SAdvancementResults(CCharAnimTime::ZeroFlat(),
                               SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
  }
  CSegStatement prior;
  if (!mTotals.Prior().AmCalculatedDown()) {
    mTotals.Prior().CalculateDown();
  }
  if (!mTotals.Next().AmCalculatedDown()) {
    mTotals.Next().CalculateDown();
  }
  GetSegStatement(prior, CSegId(0));
  mCurTime += time;
  CCharAnimTime remainder = CCharAnimTime::ZeroFlat();
  if (mCurTime > duration) {
    remainder = mCurTime - duration;
    mCurTime = duration;
  }
  SetReadTime(mCurTime);
  UpdatePOIStates();
  const CCharAnimTime interval = mSource->FinestSample();
  if (!mTotals.Prior().AmCalculatedDown()) {
    mTotals.Prior().CalculateDown();
  }
  if (!mTotals.Next().AmCalculatedDown()) {
    mTotals.Next().CalculateDown();
  }
  CSegStatement next;
  GetSegStatement(next, CSegId(0));
  const CQuaternion priorRotation = prior.Orientation();
  const CQuaternion nextRotation = next.Orientation();
  const CQuaternion priorInverse = priorRotation.BuildInverted();
  CVector3f offset(0.f, 0.f, 0.f);
  if (VHasOffset(CSegId(0))) {
    offset = next.Offset() - prior.Offset();
    const CQuaternion nextInverse = nextRotation.BuildInverted();
    offset = nextInverse.Transform(offset);
  }
  return SAdvancementResults(remainder, SAdvancementDeltas(offset, nextRotation * priorInverse));
}

void CFBStreamedAnimReader::VSetPhase(float phase) {
  mCurTime = CCharAnimTime(phase * mSteadyStateInfo.GetDuration().GetSeconds());
  SetReadTime(mCurTime);
  UpdatePOIStates();
  if (!mCurTime.GreaterThanZero()) {
    mPassedBoolCount = 0;
    mPassedIntCount = 0;
    mPassedParticleCount = 0;
    mPassedSoundCount = 0;
  }
}

void CFBStreamedAnimReader::VGetSegStatementSet(const CSegIdList& list,
                                                CSegStatementSet& set) const {
  VGetSegStatementSet(list, set, mCurTime);
}

void CFBStreamedAnimReader::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& set,
                                                const CCharAnimTime& time) const {
  SetReadTime(time);
  if (!mTotals.Prior().AmCalculatedDown()) {
    mTotals.Prior().CalculateDown();
  }
  if (!mTotals.Next().AmCalculatedDown()) {
    mTotals.Next().CalculateDown();
  }
  for (rstl::vector< CSegId >::const_iterator it = list.mSegList.begin(); it != list.mSegList.end();
       ++it) {
    GetSegStatement(set[*it], *it);
  }
}

void CFBStreamedAnimReader::SetReadTime(const CCharAnimTime& time) const {
  mTotals.SetTime(
      mInput, mBitLoader,
      CCharAnimTime(rstl::min_val(time.GetSeconds(), mSteadyStateInfo.GetDuration().GetSeconds())));
}

CFBFullBodyAspectsForStream::CFBFullBodyAspectsForStream(
    const CFBKeyFrameReductionPerChannel_HeaderForAll& header,
    const CTimeRemainderAndFraction& time, const CCharAnimTime& duration)
: mHeader(&header), mSampleTime(time.FinestSample()) {
  mLastFrame = static_cast< uint >(0.5f + duration.GetSeconds() / time.FinestSample());
  mPriorFrame = 0;
  mNextFrame = mHeader->FrameAfter(0);
  mPriorKey = 0;
  mNextKey = 1;
  SetTime(time);
}

void CFBFullBodyAspectsForStream::SetTime(const CTimeRemainderAndFraction& time) {
  const float realTime = time.RealTime();
  const uint frame = rstl::min_val(time.IntegerTime(), mLastFrame);
  if (frame < mPriorFrame) {
    mPriorFrame = 0;
    mNextFrame = mHeader->FrameAfter(0);
    mPriorKey = 0;
    mNextKey = 1;
  }
  while (realTime > mNextFrame * mSampleTime && mNextFrame < mLastFrame) {
    uint nextFrame = mHeader->FrameAfter(mNextFrame);
    mPriorFrame = mNextFrame;
    mNextFrame = nextFrame;
    ++mPriorKey;
    ++mNextKey;
  }
  if (mNextFrame == mLastFrame) {
    mT = (realTime / mSampleTime - mPriorFrame) / (mNextFrame - mPriorFrame);
  } else {
    mT = (realTime / mSampleTime - mPriorFrame) / (mNextFrame - mPriorFrame);
  }
  mT = rstl::min_val(mT, 1.f);
}

SAdvancementResults
CFBStreamedAnimReader::VGetAdvancementResults(const CCharAnimTime& time,
                                              const CCharAnimTime& startOffset) const {
  const CCharAnimTime startTime = mCurTime + startOffset;
  CCharAnimTime curTime = mCurTime + startOffset;
  const CCharAnimTime& duration = mSource->GetAnimationDuration();
  if (startTime >= duration) {
    return SAdvancementResults(time,
                               SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
  }
  if (time.EqualsZero()) {
    return SAdvancementResults(CCharAnimTime::ZeroFlat(),
                               SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
  }
  SetReadTime(startTime);
  CSegStatement prior;
  if (!mTotals.Prior().AmCalculatedDown()) {
    mTotals.Prior().CalculateDown();
  }
  if (!mTotals.Next().AmCalculatedDown()) {
    mTotals.Next().CalculateDown();
  }
  GetSegStatement(prior, CSegId(0));
  curTime += time;
  CCharAnimTime remainder = CCharAnimTime::ZeroFlat();
  if (curTime > duration) {
    remainder = curTime - duration;
    curTime = duration;
  }
  SetReadTime(curTime);
  const CCharAnimTime interval = mSource->FinestSample();
  if (!mTotals.Prior().AmCalculatedDown()) {
    mTotals.Prior().CalculateDown();
  }
  if (!mTotals.Next().AmCalculatedDown()) {
    mTotals.Next().CalculateDown();
  }
  CSegStatement next;
  GetSegStatement(next, CSegId(0));
  const CQuaternion priorRotation = prior.Orientation();
  const CQuaternion nextRotation = next.Orientation();
  const CQuaternion priorInverse = priorRotation.BuildInverted();
  CVector3f offset(0.f, 0.f, 0.f);
  if (VHasOffset(CSegId(0))) {
    offset = next.Offset() - prior.Offset();
    const CQuaternion nextInverse = nextRotation.BuildInverted();
    offset = nextInverse.Transform(offset);
  }
  SetReadTime(mCurTime);
  return SAdvancementResults(remainder, SAdvancementDeltas(offset, nextRotation * priorInverse));
}

inline void CFBStreamedAnimReader::GetSegStatement(CSegStatement& statement,
                                                   const CSegId& seg) const {
  const unsigned long index = mSegIdToIndex.SegIdToIndex(seg.val());
  if (index == ~0u) {
    statement.Set(CQuaternion::NoRotation());
  } else {
    if (mTotals.Next().HasRotation(index)) {
      statement.Set(CAnimMathUtils::Slerp(mTotals.Prior().GetQuat(index),
                                          mTotals.Next().GetQuat(index), mTotals.GetT()));
    } else {
      statement.Set(CQuaternion::NoRotation());
    }
    if (mTotals.Next().HasOffset(index)) {
      const CVector3f& prior = mTotals.Prior().GetVector(index);
      const CVector3f& next = mTotals.Next().GetVector(index);
      statement.Set(CVector3f::Lerp(prior, next, mTotals.GetT()));
    }
    if (mTotals.Next().HasScale(index)) {
      const CVector3f& prior = mTotals.Prior().GetScale(index);
      const CVector3f& next = mTotals.Next().GetScale(index);
      statement.SetScale(CVector3f::Lerp(prior, next, mTotals.GetT()));
    }
  }
}

void CFBStreamedAnimReader::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                                 CJointData_LinearStorage& data,
                                                 const CCharAnimTime& time) const {
  SetReadTime(time);
  if (!mTotals.Prior().AmCalculatedDown()) {
    mTotals.Prior().CalculateDown();
  }
  if (!mTotals.Next().AmCalculatedDown()) {
    mTotals.Next().CalculateDown();
  }
  const CFBStreamedAnimReaderTotals& next = mTotals.Next();
  const CFBStreamedAnimReaderTotals& prior = mTotals.Prior();
  const bool hasScale = next.HasScaleData();
  const bool hasOffset = next.HasOffsetData();
  uchar* rotations = data.GetRotations();
  uchar* translations = data.GetTranslations();
  uchar* scales = data.GetScales();
  const int stride = data.GetStride();
  const uint count = next.NumEntries();
  if (!hasScale && data.HasScales()) {
    data.ResetScales();
  }
  const float t = mTotals.GetT();
  if (!hasScale && !hasOffset) {
    for (uint i = 0; i < count; ++i) {
      if (next.HasRotation(i)) {
        *reinterpret_cast< CQuaternion* >(rotations) =
            CAnimMathUtils::Slerp(prior.GetQuat(i), next.GetQuat(i), t);
      } else {
        *reinterpret_cast< CQuaternion* >(rotations) = CQuaternion::NoRotation();
      }
      rotations += stride;
    }
  } else if (!hasScale && hasOffset) {
    data.SetHasOffsets(true);
    if (data.UsesZeroOffsets()) {
      for (uint i = 0; i < count; ++i) {
        if (next.HasRotation(i)) {
          *reinterpret_cast< CQuaternion* >(rotations) =
              CAnimMathUtils::Slerp(prior.GetQuat(i), next.GetQuat(i), t);
        } else {
          *reinterpret_cast< CQuaternion* >(rotations) = CQuaternion::NoRotation();
        }
        rotations += stride;
        if (next.HasOffset(i)) {
          *reinterpret_cast< CVector3f* >(translations) =
              CVector3f::Lerp(prior.GetVector(i), next.GetVector(i), t);
        } else {
          *reinterpret_cast< CVector3f* >(translations) = CVector3f::Zero();
        }
        translations += stride;
      }
    } else {
      const CVector3f* referenceOffsets = layout.GetLinearParentOffsets().data();
      for (uint i = 0; i < count; ++i) {
        if (next.HasRotation(i)) {
          *reinterpret_cast< CQuaternion* >(rotations) =
              CAnimMathUtils::Slerp(prior.GetQuat(i), next.GetQuat(i), t);
        } else {
          *reinterpret_cast< CQuaternion* >(rotations) = CQuaternion::NoRotation();
        }
        rotations += stride;
        if (next.HasOffset(i)) {
          *reinterpret_cast< CVector3f* >(translations) =
              CVector3f::Lerp(prior.GetVector(i), next.GetVector(i), t);
        } else {
          *reinterpret_cast< CVector3f* >(translations) = *referenceOffsets;
        }
        translations += stride;
        ++referenceOffsets;
      }
    }
  } else {
    data.SetHasOffsets(true);
    data.SetHasScales(true);
    if (data.UsesZeroOffsets()) {
      for (uint i = 0; i < count; ++i) {
        if (next.HasRotation(i)) {
          *reinterpret_cast< CQuaternion* >(rotations) =
              CAnimMathUtils::Slerp(prior.GetQuat(i), next.GetQuat(i), t);
        } else {
          *reinterpret_cast< CQuaternion* >(rotations) = CQuaternion::NoRotation();
        }
        rotations += stride;
        if (next.HasOffset(i)) {
          *reinterpret_cast< CVector3f* >(translations) =
              CVector3f::Lerp(prior.GetVector(i), next.GetVector(i), t);
        } else {
          *reinterpret_cast< CVector3f* >(translations) = CVector3f::Zero();
        }
        translations += stride;
        if (next.HasScale(i)) {
          *reinterpret_cast< CVector3f* >(scales) =
              CVector3f::Lerp(prior.GetScale(i), next.GetScale(i), t);
        } else {
          *reinterpret_cast< CVector3f* >(scales) = CVector3f::One();
        }
        scales += stride;
      }
    } else {
      const CVector3f* referenceOffsets = layout.GetLinearParentOffsets().data();
      for (uint i = 0; i < count; ++i) {
        if (next.HasRotation(i)) {
          *reinterpret_cast< CQuaternion* >(rotations) =
              CAnimMathUtils::Slerp(prior.GetQuat(i), next.GetQuat(i), t);
        } else {
          *reinterpret_cast< CQuaternion* >(rotations) = CQuaternion::NoRotation();
        }
        rotations += stride;
        if (next.HasOffset(i)) {
          *reinterpret_cast< CVector3f* >(translations) =
              CVector3f::Lerp(prior.GetVector(i), next.GetVector(i), t);
        } else {
          *reinterpret_cast< CVector3f* >(translations) = *referenceOffsets;
        }
        translations += stride;
        ++referenceOffsets;
        if (next.HasScale(i)) {
          *reinterpret_cast< CVector3f* >(scales) =
              CVector3f::Lerp(prior.GetScale(i), next.GetScale(i), t);
        } else {
          *reinterpret_cast< CVector3f* >(scales) = CVector3f::One();
        }
        scales += stride;
      }
    }
  }
  if (time != mCurTime) {
    SetReadTime(mCurTime);
  }
}

void CFBStreamedAnimReader::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                                 CJointData_LinearStorage& data) const {
  VGetJointData_Linear(layout, data, mCurTime);
}
