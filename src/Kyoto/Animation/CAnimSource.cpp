#include "Kyoto/Animation/CAnimSource.hpp"

#include "Kyoto/Animation/CAnimMathUtils.hpp"
#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CJointData_LinearStorage.hpp"
#include "Kyoto/Animation/CSegIdList.hpp"
#include "Kyoto/Animation/CSegStatementSet.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

namespace {
const float kInterpolationThreshold = 0.0001f;

// Frame/weight calculation shared by the source samplers.
float GetFrameAndWeight(const CCharAnimTime& time, const CCharAnimTime& interval, uint& frame) {
#ifdef __MWERKS__
  const float inverseInterval = __fres(interval.GetSeconds());
#else
  const float inverseInterval = 1.f / interval.GetSeconds();
#endif
  frame = static_cast< uint >(time.GetSeconds() * inverseInterval);
  float remainder = time.GetSeconds() - interval.GetSeconds() * frame;
  if (close_enough(remainder, 0.f)) {
    remainder = 0.f;
  }
  return CMath::Clamp(0.f, remainder * inverseInterval, 1.f);
}

CQuaternion SampleRotation(const CQuaternion& a, const CQuaternion& b, float weight) {
  if (1.f - weight < kInterpolationThreshold) {
    return b;
  }
  if (weight < kInterpolationThreshold) {
    return a;
  }
  return CAnimMathUtils::Slerp(a, b, weight);
}

CVector3f SampleVector(const CVector3f& a, const CVector3f& b, float weight) {
  if (1.f - weight < kInterpolationThreshold) {
    return b;
  }
  if (weight < kInterpolationThreshold) {
    return a;
  }
  return CVector3f::Lerp(a, b, weight);
}
} // namespace

uint RotationAndOffsetStorage::DataSizeInBytes(uint rotationsPerFrame, uint offsetsPerFrame,
                                               uint numFrames) {
  return (rotationsPerFrame * sizeof(CQuaternion) + offsetsPerFrame * sizeof(CVector3f)) *
         numFrames;
}

RotationAndOffsetStorage::RotationAndOffsetStorage(const CRotationAndOffsetVectors& vectors,
                                                   uint numFrames)
: mStorage(GetRotationsAndOffsets(vectors.mRotations, vectors.mOffsets, numFrames))
, mNumFrames(numFrames)
, mRotationsPerFrame(vectors.mRotations.size() / numFrames)
, mOffsetsPerFrame(vectors.mOffsets.size() / numFrames) {}

rstl::auto_ptr< uint >
RotationAndOffsetStorage::GetRotationsAndOffsets(const rstl::vector< CQuaternion >& rotations,
                                                 const rstl::vector< CVector3f >& offsets,
                                                 uint numFrames) {
  mRotationsPerFrame = rotations.size() / numFrames;
  mOffsetsPerFrame = offsets.size() / numFrames;
  const uint words =
      DataSizeInBytes(mRotationsPerFrame, mOffsetsPerFrame, numFrames) / sizeof(uint);
  rstl::auto_ptr< uint > storage(rs_new uint[words + 1]);
  CopyRotationsAndOffsets(rotations, offsets, numFrames, reinterpret_cast< float* >(storage.get()));
  return storage;
}

void RotationAndOffsetStorage::CopyRotationsAndOffsets(const rstl::vector< CQuaternion >& rotations,
                                                       const rstl::vector< CVector3f >& offsets,
                                                       uint numFrames, float* buffer) {
  const uint rotationsPerFrame = rotations.size() / numFrames;
  const uint offsetsPerFrame = offsets.size() / numFrames;
  for (uint frame = 0; frame < numFrames; ++frame) {
    for (uint channel = 0; channel < rotationsPerFrame; ++channel) {
      const CQuaternion& rotation = rotations[channel * numFrames + frame];
      *buffer++ = rotation.GetScalar();
      *buffer++ = rotation.AxisX();
      *buffer++ = rotation.AxisY();
      *buffer++ = rotation.AxisZ();
    }
    for (uint channel = 0; channel < offsetsPerFrame; ++channel) {
      const CVector3f& offset = offsets[channel * numFrames + frame];
      *buffer++ = offset.GetX();
      *buffer++ = offset.GetY();
      *buffer++ = offset.GetZ();
    }
  }
}

uint RotationAndOffsetStorage::GetFrameSizeInBytes() const {
  return mRotationsPerFrame * sizeof(CQuaternion) + mOffsetsPerFrame * sizeof(CVector3f);
}

RotationAndOffsetStorage::CRotationAndOffsetVectors::CRotationAndOffsetVectors(CInputStream& in)
: mRotations(in), mOffsets(in) {}

CAnimSource::CAnimSource(CInputStream& in)
: mDuration(in)
, mInterval(in)
, mFrameCount(in.Get< uint >())
, mRoot(in)
, mSegmentChannels(in)
, mRotationChannels(in)
, mOffsetChannels(in)
, mScaleChannels(in)
, mScales(in)
, mScalesPerFrame(mScales.size() / mFrameCount)
, mStorage(RotationAndOffsetStorage::CRotationAndOffsetVectors(in), mFrameCount)
, mAverageVelocity(0.f) {
  CalcAverageVelocity();
  CCharAnimMemoryMetrics::AddToTotalSize(GetSize(), CCharAnimMemoryMetrics::kASS_Two);
}

CAnimSource::~CAnimSource() {
  CCharAnimMemoryMetrics::SubtractFromTotalSize(GetSize(), CCharAnimMemoryMetrics::kASS_Two);
}

bool CAnimSource::HasRotation(const CSegId& seg) const {
  return mRotationChannels[mSegmentChannels[seg.val()]] >= 0;
}

bool CAnimSource::HasOffset(const CSegId& seg) const {
  return mOffsetChannels[mSegmentChannels[seg.val()]] >= 0;
}

bool CAnimSource::HasScale(const CSegId& seg) const {
  return mScaleChannels[mSegmentChannels[seg.val()]] >= 0;
}

CVector3f CAnimSource::GetOffset(const CSegId& seg, const CCharAnimTime& time) const {
  uint frame;
  const float weight = GetFrameAndWeight(time, mInterval, frame);
  const int channel = mSegmentChannels[seg.val()];
  if (channel < 0 || !HasOffset(seg)) {
    return CVector3f::Zero();
  }
  const uint nextFrame = frame == mFrameCount - 1 ? 0 : frame + 1;
  const uint offsetChannel = mOffsetChannels[channel];
  return CVector3f::Lerp(mStorage.GetOffset(offsetChannel, frame),
                         mStorage.GetOffset(offsetChannel, nextFrame), weight);
}

CQuaternion CAnimSource::GetRotation(const CSegId& seg, const CCharAnimTime& time) const {
  const int channel = mSegmentChannels[seg.val()];
  if (channel < 0 || !HasRotation(seg)) {
    return CQuaternion::NoRotation();
  }
  uint frame;
  const float weight = GetFrameAndWeight(time, mInterval, frame);
  const uint nextFrame = frame == mFrameCount - 1 ? 0 : frame + 1;
  return CAnimMathUtils::Slerp(mStorage.GetRotation(channel, frame),
                               mStorage.GetRotation(channel, nextFrame), weight);
}

void CAnimSource::CalcAverageVelocity() {
  const uint channel = mOffsetChannels[mSegmentChannels[0]];
  float distance = 0.f;
  for (uint frame = 1; frame < mFrameCount; ++frame) {
    const CVector3f delta =
        mStorage.GetOffset(channel, frame) - mStorage.GetOffset(channel, frame - 1);
    const float magnitude = delta.Magnitude();
    if (!close_enough(magnitude, 0.f)) {
      distance += magnitude;
    }
  }
  mAverageVelocity = distance / mDuration.GetSeconds();
}

void CAnimSource::GetSegStatement(const CSegId& seg, uint frame, uint nextFrame, float weight,
                                  CSegStatement& statement) const {
  const int channel = mSegmentChannels[seg.val()];
  if (HasRotation(seg)) {
    statement.Set(SampleRotation(mStorage.GetRotation(channel, frame),
                                 mStorage.GetRotation(channel, nextFrame), weight));
  }
  if (HasOffset(seg)) {
    const uint offsetChannel = mOffsetChannels[channel];
    statement.Set(SampleVector(mStorage.GetOffset(offsetChannel, frame),
                               mStorage.GetOffset(offsetChannel, nextFrame), weight));
  }
}

void CAnimSource::GetSegStatementSet(const CSegIdList& list, CSegStatementSet& set,
                                     const CCharAnimTime& time) const {
  uint frame;
  const float weight = GetFrameAndWeight(time, mInterval, frame);
  const uint nextFrame = frame == mFrameCount - 1 ? 0 : frame + 1;
  for (int i = 0; i < list.GetCount(); ++i) {
    const CSegId& seg = list.mSegList[i];
    const int channel = mSegmentChannels[seg.val()];
    CSegStatement& statement = set[seg];
    if (channel < 0) {
      statement.Set(CQuaternion::NoRotation());
      continue;
    }
    GetSegStatement(seg, frame, nextFrame, weight, statement);
    if (HasScale(seg)) {
      const uint scaleChannel = mScaleChannels[channel];
      statement.SetScale(SampleVector(mScales[frame * mScalesPerFrame + scaleChannel],
                                      mScales[nextFrame * mScalesPerFrame + scaleChannel], weight));
    }
  }
}

void CAnimSource::GetSegData(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                             const CCharAnimTime& time) const {
  uint frame;
  const float weight = GetFrameAndWeight(time, mInterval, frame);
  const uint nextFrame = frame == mFrameCount - 1 ? 0 : frame + 1;
  const bool hasScales = !mScales.empty();
  const bool hasOffsets = mStorage.GetOffsetCount() != 0;
  if (!hasScales && data.HasScales()) {
    data.ResetScales();
  }
  if (hasScales || hasOffsets) {
    data.SetHasOffsets(true);
  }
  if (hasScales) {
    data.SetHasScales(true);
  }

  uint rotationChannel = 0;
  uint offsetChannel = 0;
  uint scaleChannel = 0;
  for (int i = 0; i < mRotationChannels.size(); ++i) {
    if (mRotationChannels[i] == -1) {
      data.Rotation(i) = CQuaternion::NoRotation();
    } else {
      data.Rotation(i) = SampleRotation(mStorage.GetRotation(rotationChannel, frame),
                                        mStorage.GetRotation(rotationChannel, nextFrame), weight);
      ++rotationChannel;
    }
    if (hasScales || hasOffsets) {
      if (mOffsetChannels[i] == -1) {
        data.Translation(i) =
            data.UsesZeroOffsets() ? CVector3f::Zero() : layout.GetLinearParentOffsets()[i];
      } else {
        data.Translation(i) = SampleVector(mStorage.GetOffset(offsetChannel, frame),
                                           mStorage.GetOffset(offsetChannel, nextFrame), weight);
        ++offsetChannel;
      }
    }
    if (hasScales) {
      if (mScaleChannels[i] == -1) {
        data.Scale(i) = CVector3f::One();
      } else {
        data.Scale(i) = SampleVector(mScales[frame * mScalesPerFrame + scaleChannel],
                                     mScales[nextFrame * mScalesPerFrame + scaleChannel], weight);
        ++scaleChannel;
      }
    }
  }
}

uint CAnimSource::GetSize() const {
  // The original accounting omits the scale-channel map and scale keys.
  return sizeof(CAnimSource) + mSegmentChannels.size() + mRotationChannels.size() +
         mOffsetChannels.size() + mFrameCount * mStorage.GetFrameSizeInBytes();
}
