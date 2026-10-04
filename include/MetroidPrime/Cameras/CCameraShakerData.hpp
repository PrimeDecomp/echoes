#ifndef _CCAMERASHAKERDATA
#define _CCAMERASHAKERDATA

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Math/CVector3f.hpp"

struct SLdrCameraShakerData;

class CCameraShakerData {
public:
  // Guessed names for target-supported flag uses; other bits remain unresolved.
  enum EFlags {
    kF_DistanceAttenuation = 1,
    kF_ExplicitDuration = 4,
    kF_AmplitudeScaledVolume = 8,
    kF_NonPositionalSound = 0x10,
    kF_AllowCinematic = 0x20,
    kF_RumbleDistanceAttenuation = 0x40
  };

  CCameraShakerData(float attenuationDistance, float duration, uint flags,
                    const CVector3f& position, const CMayaSpline& horizontalMotion,
                    const CMayaSpline& verticalMotion, const CMayaSpline& forwardMotion,
                    int audioEffect);

  CCameraShakerData NewTranslation(const CVector3f& position) const;
  CVector3f GetPoint(float time);
  float GetMaxAmplitude();

  // Guessed names: threshold crossings of the three motion splines.
  float FindFirstIntersection(float amplitude);
  float FindLastIntersection(float amplitude);
  void UpdateThresholdTimes();

  uint GetFlags() const { return mFlags; }
  float GetDuration() const { return mDuration; }
  float GetAttenuationDistance() const { return mAttenuationDistance; }
  const CVector3f& GetPosition() const { return mPosition; }
  int GetAudioEffect() const { return mAudioEffect; }
  float GetCachedMaxAmplitude() const { return mMaxAmplitude; }
  float GetFirstThresholdTime() const { return mFirstThresholdTime; }
  float GetLastThresholdTime() const { return mLastThresholdTime; }

private:
  uint mFlags;
  float mDuration;
  float mAttenuationDistance;
  CVector3f mPosition;
  CMayaSpline mHorizontalMotion;
  CMayaSpline mForwardMotion;
  CMayaSpline mVerticalMotion;
  int mAudioEffect;
  float mMaxAmplitude;
  float mLastThresholdTime;
  float mFirstThresholdTime;
};
CHECK_SIZEOF(CCameraShakerData, 0xf4)

// Original Wii-exported name and parameters; return type correlated to GC callers.
CCameraShakerData LdrToCameraShakerData(const SLdrCameraShakerData& data,
                                      const CVector3f& position);

#endif // _CCAMERASHAKERDATA
