#include "MetroidPrime/Cameras/CCameraShakerData.hpp"

#include "MetroidPrime/ScriptLoader/Structs/SLdrCameraShakerData.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"
#include "rstl/reserved_vector.hpp"

CCameraShakerData::CCameraShakerData(uint flags, float attenuationDistance, float duration,
                                     const CVector3f& position, const CMayaSpline& horizontalMotion,
                                     const CMayaSpline& verticalMotion,
                                     const CMayaSpline& forwardMotion, int audioEffect)
: mFlags(flags)
, mDuration(duration)
, mAttenuationDistance(attenuationDistance)
, mPosition(position)
, mHorizontalMotion(horizontalMotion)
, mForwardMotion(forwardMotion)
, mVerticalMotion(verticalMotion)
, mAudioEffect(audioEffect)
, mMaxAmplitude(0.f)
, mLastThresholdTime(duration)
, mFirstThresholdTime(0.f) {
  if ((flags & kF_ExplicitDuration) == 0) {
    mDuration = mHorizontalMotion.GetMaxTime();
    float maxTime = mVerticalMotion.GetMaxTime();
    mDuration = rstl::max_val(maxTime, mDuration);
    maxTime = mForwardMotion.GetMaxTime();
    mDuration = rstl::max_val(maxTime, mDuration);
  }
  mMaxAmplitude = GetMaxAmplitude();
}

// Guessed name
void CCameraShakerData::UpdateThresholdTimes() {
  mFirstThresholdTime = FindFirstIntersection(0.2f);
  mLastThresholdTime = FindLastIntersection(0.2f);
}

CVector3f CCameraShakerData::GetPoint(float time) {
  time = CMath::Clamp(0.f, time, mDuration);
  const float x = mHorizontalMotion.EvaluateAt(time);
  const float y = mForwardMotion.EvaluateAt(time);
  const float z = mVerticalMotion.EvaluateAt(time);
  return CVector3f(x, y, z);
}

CCameraShakerData CCameraShakerData::NewTranslation(const CVector3f& position) const {
  return CCameraShakerData(mFlags, mAttenuationDistance, mDuration, position, mHorizontalMotion,
                           mVerticalMotion, mForwardMotion, mAudioEffect);
}

float CCameraShakerData::GetMaxAmplitude() {
  const float horizontal = mHorizontalMotion.FindMaximumAmplitude().second;
  const float vertical = mVerticalMotion.FindMaximumAmplitude().second;
  const float forward = mForwardMotion.FindMaximumAmplitude().second;
  return rstl::max_val(forward, rstl::max_val(vertical, horizontal));
}

// Guessed name
float CCameraShakerData::FindLastIntersection(float amplitude) {
  const float h0 = mHorizontalMotion.FindLastIntersection(-amplitude);
  const float horizontal = rstl::max_val(h0, mHorizontalMotion.FindLastIntersection(amplitude));
  const float v0 = mVerticalMotion.FindLastIntersection(-amplitude);
  const float vertical = rstl::max_val(v0, mVerticalMotion.FindLastIntersection(amplitude));
  const float f0 = mForwardMotion.FindLastIntersection(-amplitude);
  const float forward = rstl::max_val(f0, mForwardMotion.FindLastIntersection(amplitude));
  float time = rstl::max_val(forward, rstl::max_val(vertical, horizontal));
  if (time < 0.f) {
    time = mDuration;
  }
  return time;
}

// Guessed name
float CCameraShakerData::FindFirstIntersection(float amplitude) {
  rstl::reserved_vector< float, 6 > times;
  times.push_back(mHorizontalMotion.FindFirstIntersection(amplitude));
  times.push_back(mHorizontalMotion.FindFirstIntersection(-amplitude));
  times.push_back(mVerticalMotion.FindFirstIntersection(amplitude));
  times.push_back(mVerticalMotion.FindFirstIntersection(-amplitude));
  times.push_back(mForwardMotion.FindFirstIntersection(amplitude));
  times.push_back(mForwardMotion.FindFirstIntersection(-amplitude));
  rstl::sort(times.begin(), times.end());

  for (int i = 0; i < times.size(); ++i) {
    if (times[i] >= 0.f) {
      return times[i];
    }
  }
  return 0.f;
}

CCameraShakerData LdrToCameraShakerData(const SLdrCameraShakerData& data,
                                        const CVector3f& position) {
  return CCameraShakerData(data.flagsCameraShaker, data.attenuationDistance, data.duration,
                           position, data.horizontalMotion, data.verticalMotion, data.forwardMotion,
                           data.audioEffect);
}
