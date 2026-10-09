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

// Guessed name, corroborated by Prime and the native frame-sampling consumers.
#ifdef __MWERKS__
static float clamp_zero_to_one(register float value) {
  register float out;
  register float zero = 0.f;
  register float one = 1.f;
  asm {
    fsel out, value, value, zero
    fsubs value, value, one
    fsel out, value, one, out
  }
  return out;
}
#else
static float clamp_zero_to_one(float value) {
  return value >= 1.f ? 1.f : value >= 0.f ? value : 0.f;
}
#endif

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
  rstl::auto_ptr< uint > storage(rs_new uint[DataSizeInBytes(rotations.size() / numFrames, //
                                                             offsets.size() / numFrames,   //
                                                             numFrames                     //
                                                             ) /
                                                 4 +
                                             1]);
  CopyRotationsAndOffsets(rotations, offsets, numFrames, reinterpret_cast< float* >(storage.get()));

  return storage;
}

void RotationAndOffsetStorage::CopyRotationsAndOffsets(const rstl::vector< CQuaternion >& rotations,
                                                       const rstl::vector< CVector3f >& offsets,
                                                       const uint numFrames, float* buf) {
  const uint rotationsPerFrame = rotations.size() / numFrames;
  const uint offsetsPerFrame = offsets.size() / numFrames;

  for (int frame = 0; frame < numFrames; frame++) {
    int i = 0;
    for (int rotation = 0; i < rotationsPerFrame; rotation += numFrames, i++) {
      const CQuaternion& q = rotations[frame + rotation];
      *(buf++) = q.GetScalar();
      *(buf++) = q.AxisX();
      *(buf++) = q.AxisY();
      *(buf++) = q.AxisZ();
    }
    i = 0;
    for (int offset = 0; offset < offsetsPerFrame; offset++, i += numFrames) {
      const CVector3f& o = offsets[frame + i];
      *(buf++) = o.GetX();
      *(buf++) = o.GetY();
      *(buf++) = o.GetZ();
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

CVector3f CAnimSource::GetOffset(const CSegId& seg, const CCharAnimTime& animTime) const {
  const float frameTime = animTime.GetSeconds();
  float interval = mInterval.GetSeconds();
#ifdef __MWERKS__
  const float invTime = __fres(interval);
#else
  const float invTime = 1.f / interval;
#endif
  const uint frame = static_cast< uint >(frameTime * invTime);
  float time = interval * frame;
  time = frameTime - time;

  if (CMath::AbsF(time) < Real32::Epsilon()) {
    time = 0.f;
  }

  time = clamp_zero_to_one(time * invTime);

  int channel = mSegmentChannels[seg.val()];
  if (channel >= 0 && HasOffset(seg)) {
    const uint nextFrame = frame == mFrameCount - 1 ? 0 : frame + 1;
    channel = mOffsetChannels[channel];
    const CVector3f& a = mStorage.GetOffset(channel, frame);
    const CVector3f& b = mStorage.GetOffset(channel, nextFrame);
    return CVector3f::Lerp(a, b, time);
  }

  return CVector3f::Zero();
}

CQuaternion CAnimSource::GetRotation(const CSegId& seg, const CCharAnimTime& animTime) const {
  const float interval = GetTimePerFrame().GetSeconds();
#ifdef __MWERKS__
  const float invTime = __fres(interval);
#else
  const float invTime = 1.f / interval;
#endif
  const int channel = mSegmentChannels[seg.val()];
  if (channel >= 0 && HasRotation(seg)) {
    const float frameTime = animTime.GetSeconds();
    const uint frame = static_cast< uint >(frameTime * invTime);
    float time = interval * frame;
    time = frameTime - time;

    if (CMath::AbsF(time) < Real32::Epsilon()) {
      time = 0.f;
    }

    time = clamp_zero_to_one(time * invTime);
    const uint nextFrame = frame == mFrameCount - 1 ? 0 : frame + 1;
    const CQuaternion& a = mStorage.GetRotation(channel, frame);
    const CQuaternion& b = mStorage.GetRotation(channel, nextFrame);
    return CAnimMathUtils::Slerp(a, b, time);
  }

  return CQuaternion::NoRotation();
}

void CAnimSource::CalcAverageVelocity() {
  const float invDuration = 1.f / mDuration.GetSeconds();
  float distance = 0.f;
  const uint channel = mOffsetChannels[mSegmentChannels[0]];
  for (uint frame = 1; frame < mFrameCount; ++frame) {
    const CVector3f delta =
        mStorage.GetOffset(channel, frame) - mStorage.GetOffset(channel, frame - 1);
    const float magnitude = delta.Magnitude();
    if (!close_enough(magnitude, 0.f)) {
      distance += magnitude;
    }
  }

  distance *= invDuration;
  mAverageVelocity = distance;
}

void CAnimSource::GetSegStatement(const CSegId& seg, uint frame, uint nextFrame, float weight,
                                  CSegStatement& statement) const {
  const int channel = mSegmentChannels[seg.val()];
  const float inverseWeight = 1.f - weight;
  if (inverseWeight < CAnimMathUtils::kInterpolationThreshold) {
    if (HasRotation(seg)) {
      statement.Set(mStorage.GetRotation(channel, nextFrame));
    }
    if (HasOffset(seg)) {
      statement.Set(mStorage.GetOffset(mOffsetChannels[channel], nextFrame));
    }
  } else if (weight < CAnimMathUtils::kInterpolationThreshold) {
    if (HasRotation(seg)) {
      statement.Set(mStorage.GetRotation(channel, frame));
    }
    if (HasOffset(seg)) {
      statement.Set(mStorage.GetOffset(mOffsetChannels[channel], frame));
    }
  } else {
    if (HasRotation(seg)) {
      const CQuaternion& a = mStorage.GetRotation(channel, frame);
      const CQuaternion& b = mStorage.GetRotation(channel, nextFrame);
      statement.Set(CAnimMathUtils::Slerp(a, b, weight));
    }
    if (HasOffset(seg)) {
      const uint offsetChannel = mOffsetChannels[channel];
      const CVector3f& a = mStorage.GetOffset(offsetChannel, frame);
      const CVector3f& b = mStorage.GetOffset(offsetChannel, nextFrame);
      statement.Set(inverseWeight * a + weight * b);
    }
  }
}

void CAnimSource::GetSegStatementSet(const CSegIdList& list, CSegStatementSet& set,
                                     const CCharAnimTime& time) const {
  const float frameTime = time.GetSeconds();
  const float interval = GetTimePerFrame().GetSeconds();
#ifdef __MWERKS__
  const float inverseInterval = __fres(interval);
#else
  const float inverseInterval = 1.f / interval;
#endif
  const uint frame = static_cast< uint >(frameTime * inverseInterval);
  float remainder = interval * frame;
  remainder = frameTime - remainder;
  if (CMath::AbsF(remainder) < Real32::Epsilon()) {
    remainder = 0.f;
  }
  const float weight = clamp_zero_to_one(remainder * inverseInterval);
  const uint nextFrame = frame == mFrameCount - 1 ? 0 : frame + 1;
  const int count = list.GetCount();
  const float inverseWeight = 1.f - weight;
  for (int i = 0; i < count; ++i) {
    const CSegId seg = list.mSegList[i];
    const int channel = mSegmentChannels[seg.val()];
    if (channel >= 0) {
      CSegStatement& statement = set[seg];
      GetSegStatement(seg, frame, nextFrame, weight, statement);
      if (HasScale(seg)) {
        const uint scaleChannel = mScaleChannels[channel];
        if (inverseWeight < CAnimMathUtils::kInterpolationThreshold) {
          statement.SetScale(mScales[nextFrame * mScalesPerFrame + scaleChannel]);
        } else if (weight < CAnimMathUtils::kInterpolationThreshold) {
          statement.SetScale(mScales[frame * mScalesPerFrame + scaleChannel]);
        } else {
          const CVector3f& a = mScales[frame * mScalesPerFrame + scaleChannel];
          const CVector3f& b = mScales[nextFrame * mScalesPerFrame + scaleChannel];
          statement.SetScale(inverseWeight * a + weight * b);
        }
      }
    } else {
      set[seg].Set(CQuaternion::NoRotation());
    }
  }
}

void CAnimSource::GetJointData_Linear(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                                      const CCharAnimTime& time) const {
  const float frameTime = time.GetSeconds();
  const float interval = GetTimePerFrame().GetSeconds();
#ifdef __MWERKS__
  const float inverseInterval = __fres(interval);
#else
  const float inverseInterval = 1.f / interval;
#endif
  const uint frame = static_cast< uint >(frameTime * inverseInterval);
  float remainder = interval * frame;
  remainder = frameTime - remainder;
  if (CMath::AbsF(remainder) < Real32::Epsilon()) {
    remainder = 0.f;
  }
  const float weight = clamp_zero_to_one(remainder * inverseInterval);
  const uint nextFrame = frame == mFrameCount - 1 ? 0 : frame + 1;
  const bool hasScales = !mScales.empty();
  const bool hasOffsets = mStorage.GetOffsetCount() != 0;
  const int count = mRotationChannels.size();
  if (!hasScales && data.HasScales()) {
    data.ResetScales();
  }

  uint copyFrame = ~0u;
  const float inverseWeight = 1.f - weight;
  uchar* rotations = data.GetRotations();
  uchar* offsets = data.GetTranslations();
  uchar* scales = data.GetScales();
  const int stride = data.GetStride();
  if (inverseWeight < CAnimMathUtils::kInterpolationThreshold) {
    copyFrame = nextFrame;
  } else if (weight < CAnimMathUtils::kInterpolationThreshold) {
    copyFrame = frame;
  }
  const signed char* rotationChannels = mRotationChannels.data();
  const signed char* offsetChannels = mOffsetChannels.data();
  const signed char* scaleChannels = mScaleChannels.data();
  const CVector3f* referenceOffsets = layout.GetLinearParentOffsets().data();

  if (copyFrame != ~0u) {
    const CQuaternion* sourceRotations =
        reinterpret_cast< const CQuaternion* >(mStorage.StartForFrame(copyFrame));
    const CVector3f* sourceOffsets = reinterpret_cast< const CVector3f* >(
        mStorage.StartForFrame(copyFrame) + mStorage.GetRotationCount() * 4);
    if (!hasScales && !hasOffsets) {
      for (int i = 0; i < count; ++i) {
        if (rotationChannels[i] != -1) {
          *reinterpret_cast< CQuaternion* >(rotations) = *sourceRotations++;
        } else {
          *reinterpret_cast< CQuaternion* >(rotations) = CQuaternion::NoRotation();
        }
        rotations += stride;
      }
    } else if (!hasScales && hasOffsets) {
      data.SetHasOffsets(true);
      for (int i = 0; i < count; ++i) {
        if (rotationChannels[i] != -1) {
          *reinterpret_cast< CQuaternion* >(rotations) = *sourceRotations++;
        } else {
          *reinterpret_cast< CQuaternion* >(rotations) = CQuaternion::NoRotation();
        }
        if (offsetChannels[i] != -1) {
          *reinterpret_cast< CVector3f* >(offsets) = *sourceOffsets++;
        } else {
          *reinterpret_cast< CVector3f* >(offsets) =
              data.UsesZeroOffsets() ? CVector3f::Zero() : *referenceOffsets;
        }
        rotations += stride;
        offsets += stride;
        ++referenceOffsets;
      }
    } else {
      data.SetHasOffsets(true);
      data.SetHasScales(true);
      const CVector3f* sourceScales = mScales.data() + copyFrame * mScalesPerFrame;
      for (int i = 0; i < count; ++i) {
        if (rotationChannels[i] != -1) {
          *reinterpret_cast< CQuaternion* >(rotations) = *sourceRotations++;
        } else {
          *reinterpret_cast< CQuaternion* >(rotations) = CQuaternion::NoRotation();
        }
        if (offsetChannels[i] != -1) {
          *reinterpret_cast< CVector3f* >(offsets) = *sourceOffsets++;
        } else {
          *reinterpret_cast< CVector3f* >(offsets) =
              data.UsesZeroOffsets() ? CVector3f::Zero() : *referenceOffsets;
        }
        if (scaleChannels[i] != -1) {
          *reinterpret_cast< CVector3f* >(scales) = *sourceScales++;
        } else {
          *reinterpret_cast< CVector3f* >(scales) = CVector3f::One();
        }
        rotations += stride;
        offsets += stride;
        ++referenceOffsets;
        scales += stride;
      }
    }
  } else {
    const CQuaternion* priorRotations =
        reinterpret_cast< const CQuaternion* >(mStorage.StartForFrame(frame));
    const CQuaternion* nextRotations =
        reinterpret_cast< const CQuaternion* >(mStorage.StartForFrame(nextFrame));
    const CVector3f* priorOffsets = reinterpret_cast< const CVector3f* >(
        mStorage.StartForFrame(frame) + mStorage.GetRotationCount() * 4);
    const CVector3f* nextOffsets = reinterpret_cast< const CVector3f* >(
        mStorage.StartForFrame(nextFrame) + mStorage.GetRotationCount() * 4);
    if (!hasScales && !hasOffsets) {
      for (int i = 0; i < count; ++i) {
        if (rotationChannels[i] != -1) {
          *reinterpret_cast< CQuaternion* >(rotations) =
              CAnimMathUtils::Slerp(*priorRotations++, *nextRotations++, weight);
        } else {
          *reinterpret_cast< CQuaternion* >(rotations) = CQuaternion::NoRotation();
        }
        rotations += stride;
      }
    } else if (!hasScales && hasOffsets) {
      data.SetHasOffsets(true);
      for (int i = 0; i < count; ++i) {
        if (rotationChannels[i] != -1) {
          *reinterpret_cast< CQuaternion* >(rotations) =
              CAnimMathUtils::Slerp(*priorRotations++, *nextRotations++, weight);
        } else {
          *reinterpret_cast< CQuaternion* >(rotations) = CQuaternion::NoRotation();
        }
        if (offsetChannels[i] != -1) {
          *reinterpret_cast< CVector3f* >(offsets) =
              inverseWeight * *priorOffsets++ + weight * *nextOffsets++;
        } else {
          *reinterpret_cast< CVector3f* >(offsets) =
              data.UsesZeroOffsets() ? CVector3f::Zero() : *referenceOffsets;
        }
        rotations += stride;
        offsets += stride;
        ++referenceOffsets;
      }
    } else {
      data.SetHasOffsets(true);
      data.SetHasScales(true);
      const CVector3f* priorScales = mScales.data() + frame * mScalesPerFrame;
      const CVector3f* nextScales = mScales.data() + nextFrame * mScalesPerFrame;
      for (int i = 0; i < count; ++i) {
        if (rotationChannels[i] != -1) {
          *reinterpret_cast< CQuaternion* >(rotations) =
              CAnimMathUtils::Slerp(*priorRotations++, *nextRotations++, weight);
        } else {
          *reinterpret_cast< CQuaternion* >(rotations) = CQuaternion::NoRotation();
        }
        if (offsetChannels[i] != -1) {
          *reinterpret_cast< CVector3f* >(offsets) =
              inverseWeight * *priorOffsets++ + weight * *nextOffsets++;
        } else {
          *reinterpret_cast< CVector3f* >(offsets) =
              data.UsesZeroOffsets() ? CVector3f::Zero() : *referenceOffsets;
        }
        if (scaleChannels[i] != -1) {
          *reinterpret_cast< CVector3f* >(scales) =
              inverseWeight * *priorScales++ + weight * *nextScales++;
        } else {
          *reinterpret_cast< CVector3f* >(scales) = CVector3f::One();
        }
        rotations += stride;
        offsets += stride;
        ++referenceOffsets;
        scales += stride;
      }
    }
  }
}

uint CAnimSource::GetSize() const {
  // The original accounting omits the scale-channel map and scale keys.
  uint size = sizeof(CAnimSource) + mSegmentChannels.size();
  size += mRotationChannels.size();
  size += mOffsetChannels.size();
  size += mFrameCount * mStorage.GetFrameSizeInBytes();
  return size;
}
