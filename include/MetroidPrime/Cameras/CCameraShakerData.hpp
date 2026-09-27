#ifndef _CCAMERASHAKERDATA
#define _CCAMERASHAKERDATA

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Math/CVector3f.hpp"

class CCameraShakerData {
public:
  // Guessed flag name
  enum EFlags { kF_ExplicitDuration = 4 };

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

#endif // _CCAMERASHAKERDATA
