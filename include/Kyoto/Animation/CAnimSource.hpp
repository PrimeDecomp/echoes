#ifndef _CANIMSOURCE
#define _CANIMSOURCE

#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/vector.hpp"

class CCharLayoutInfo;
class CInputStream;
class CJointData_LinearStorage;
class CSegIdList;
class CSegStatement;
class CSegStatementSet;

class RotationAndOffsetStorage {
public:
  struct CRotationAndOffsetVectors {
    explicit CRotationAndOffsetVectors(CInputStream& in);

    rstl::vector< CQuaternion > mRotations;
    rstl::vector< CVector3f > mOffsets;
  };

  RotationAndOffsetStorage(const CRotationAndOffsetVectors& vectors, uint numFrames);
  static uint DataSizeInBytes(uint rotationsPerFrame, uint offsetsPerFrame, uint numFrames);
  rstl::auto_ptr< uint > GetRotationsAndOffsets(const rstl::vector< CQuaternion >& rotations,
                                                const rstl::vector< CVector3f >& offsets,
                                                uint numFrames);
  static void CopyRotationsAndOffsets(const rstl::vector< CQuaternion >& rotations,
                                      const rstl::vector< CVector3f >& offsets, uint numFrames,
                                      float* buffer);
  uint GetFrameSizeInBytes() const;

  const uint* StartForFrame(uint frame) const {
    if (frame >= mNumFrames) {
      frame = mNumFrames - 1;
    }
    return mStorage.get() + frame * (mRotationsPerFrame * 4 + mOffsetsPerFrame * 3);
  }
  const CQuaternion& GetRotation(uint channel, uint frame) const {
    return *reinterpret_cast< const CQuaternion* >(StartForFrame(frame) + channel * 4);
  }
  const CVector3f& GetOffset(uint channel, uint frame) const {
    return *reinterpret_cast< const CVector3f* >(StartForFrame(frame) + mRotationsPerFrame * 4 +
                                                 channel * 3);
  }
  uint GetOffsetCount() const { return mOffsetsPerFrame; }

private:
  rstl::auto_ptr< uint > mStorage;
  uint mNumFrames;
  uint mRotationsPerFrame;
  uint mOffsetsPerFrame;
};
CHECK_SIZEOF(RotationAndOffsetStorage, 0x14)

class CAnimSource {
public:
  explicit CAnimSource(CInputStream& in);
  ~CAnimSource();

  bool HasRotation(const CSegId& seg) const;
  bool HasOffset(const CSegId& seg) const;
  bool HasScale(const CSegId& seg) const;
  CVector3f GetOffset(const CSegId& seg, const CCharAnimTime& time) const;
  CQuaternion GetRotation(const CSegId& seg, const CCharAnimTime& time) const;
  void CalcAverageVelocity();
  void GetSegStatementSet(const CSegIdList& list, CSegStatementSet& set,
                          const CCharAnimTime& time) const;
  // Guessed name.
  void GetJointData(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                    const CCharAnimTime& time) const;

  const CCharAnimTime& GetAnimationDuration() const { return mDuration; }
  const CCharAnimTime& GetTimePerFrame() const { return mInterval; }
  CSegId GetPrimaryOffsetChannel() const { return mRoot; }
  float GetAverageVelocity() const { return mAverageVelocity; }

private:
  // Guessed names.
  void GetSegStatement(const CSegId& seg, uint frame, uint nextFrame, float weight,
                       CSegStatement& statement) const;
  uint GetSize() const;

  CCharAnimTime mDuration;
  CCharAnimTime mInterval;
  uint mFrameCount;
  CSegId mRoot;
  rstl::vector< signed char > mSegmentChannels;
  rstl::vector< signed char > mRotationChannels;
  rstl::vector< signed char > mOffsetChannels;
  rstl::vector< signed char > mScaleChannels;
  rstl::vector< CVector3f > mScales;
  uint mScalesPerFrame;
  RotationAndOffsetStorage mStorage;
  float mAverageVelocity;
};
CHECK_SIZEOF(CAnimSource, 0x84)

#endif // _CANIMSOURCE
