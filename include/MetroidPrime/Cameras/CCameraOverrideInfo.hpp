#ifndef _CCAMERAOVERRIDEINFO
#define _CCAMERAOVERRIDEINFO

#include "MetroidPrime/Cameras/CBallCamera.hpp"

class CCameraOverrideInfo {
public:
  CCameraOverrideInfo(uint flags, uint overrideFlags, CBallCamera::EBallCameraBehaviour behaviour,
                      float minDist, float maxDist, float backwardsDist,
                      const CVector3f& lookAtOffset, const CVector3f& worldOffset, float fov,
                      float attitudeRange, float azimuthRange, float anglePerSecond,
                      float elevation, float interpolateOnTime, float interpolateOffTime,
                      float controlInterpDur, int interpolateOnType, int interpolationMode,
                      int interpolateOffType);
  virtual ~CCameraOverrideInfo() {}

  CBallCamera::EBallCameraBehaviour GetBehaviourType() const { return mBehaviour; }
  uint GetFlags() const { return mFlags; }
  uint GetOverrideFlags() const { return mOverrideFlags; }
  float GetFov() const { return mFov; }
  float GetAttitudeRange() const { return mAttitudeRange; }
  float GetAzimuthRange() const { return mAzimuthRange; }
  CVector3f GetLookAtOffset() const { return mLookAtOffset; }
  CVector3f GetWorldOffset() const { return mWorldOffset; }
  float GetMinDist() const { return mMinDist; }
  float GetMaxDist() const { return mMaxDist; }
  float GetBackwardsDist() const { return mBackwardsDist; }
  float GetAnglePerSecond() const { return mAnglePerSecond; }
  float GetElevation() const { return mElevation; }
  float GetControlInterpDur() const { return mControlInterpDur; }
  float GetInterpolateOnTime() const { return mInterpolateOnTime; }
  float GetInterpolateOffTime() const { return mInterpolateOffTime; }
  int GetInterpolateOnType() const { return mInterpolateOnType; }
  int GetInterpolationMode() const { return mInterpolationMode; }
  int GetInterpolateOffType() const { return mInterpolateOffType; }

private:
  uint mFlags;
  uint mOverrideFlags;
  CBallCamera::EBallCameraBehaviour mBehaviour;
  float mMinDist;
  float mMaxDist;
  float mBackwardsDist;
  CVector3f mLookAtOffset;
  CVector3f mWorldOffset;
  float mFov;
  float mAttitudeRange;
  float mAzimuthRange;
  float mAnglePerSecond;
  float mElevation;
  float mInterpolateOnTime;
  float mInterpolateOffTime;
  float mControlInterpDur;
  int mInterpolateOnType;  // Guessed name; original enum declaration unresolved.
  int mInterpolationMode;  // Guessed name; second interpolation control, meaning unresolved.
  int mInterpolateOffType; // Guessed name; original enum declaration unresolved.
};
CHECK_SIZEOF(CCameraOverrideInfo, 0x60)

#endif // _CCAMERAOVERRIDEINFO
