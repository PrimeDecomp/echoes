#include "Kyoto/Math/CGameSpline.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"

CGameSpline::CGameSpline(float duration, uint flags, const CMayaSpline& positionTimeSpline,
                         const CMayaSpline& lookAtTimeSpline,
                         CMotionSpline::ESplineType positionType,
                         CMotionSpline::ESplineType lookAtType)
: mPositionSpline(false, duration, positionType)
, mPositionTimeSpline(positionTimeSpline)
, mLookAtSpline(false, duration, lookAtType)
, mLookAtTimeSpline(lookAtTimeSpline)
, mDuration(duration)
, mFlags(flags) {
  if (mFlags & kF_LoopPosition) {
    mPositionSpline.SetClosedLoop(true);
  }
  if (mFlags & kF_LoopLookAt) {
    mLookAtSpline.SetClosedLoop(true);
  }
}

CGameSpline::~CGameSpline() {}

void CGameSpline::Initialise(const rstl::vector< CVector3f >& positions,
                             const rstl::vector< CQuaternion >& orientations,
                             const rstl::vector< CVector3f >& lookAtPoints) {
  mPositionSpline.Initialise(positions);
  mLookAtSpline.Initialise(lookAtPoints);
  mOrientations.clear();
  mOrientations.reserve(orientations.size());
  for (int i = 0; i < orientations.size(); ++i) {
    mOrientations.push_back_unsafe(orientations[i]);
  }
}

float CGameSpline::GetLength() const { return mPositionSpline.GetLength(); }

float CGameSpline::GetDuration() const { return mDuration; }

CQuaternion CGameSpline::GetOrientationByTime(float time) {
  if (mPositionSpline.GetKnotCount() == 0) {
    return CQuaternion::AxisAngle(CUnitVector3f(0.f, 1.f, 0.f, CUnitVector3f::kN_Yes),
                                  CRelAngle::FromRadians(0.f));
  }
  if (mPositionSpline.GetKnotCount() == 1) {
    return mOrientations[0];
  }

  const float splineTime = mDuration * mPositionTimeSpline.EvaluateAt(time);
  const uint start = mPositionSpline.GetKnotIndexByTime(splineTime);
  const uint end = mPositionSpline.ValidateKnotIndex(start + 1);
  float span = mPositionSpline.GetKnotTime(end) - mPositionSpline.GetKnotTime(start);
  if (end < start) {
    span = mPositionSpline.GetKnotTime(end) + (mDuration - mPositionSpline.GetKnotTime(start));
  }
  if (span < 0.01f) {
    return mOrientations[start];
  }
  const float t = CMath::Clamp(0.f, (splineTime - mPositionSpline.GetKnotTime(start)) / span, 1.f);
  CQuaternion first = mOrientations[start];
  if (CQuaternion::Dot(first, mOrientations[end]) < 0.f) {
    first = first.BuildEquivalent();
  }
  return CQuaternion::Slerp(first, mOrientations[end], t);
}

CQuaternion CGameSpline::GetOrientationByLength(float distance) {
  if (mPositionSpline.GetKnotCount() == 0) {
    return CQuaternion::AxisAngle(CUnitVector3f(0.f, 1.f, 0.f, CUnitVector3f::kN_Yes),
                                  CRelAngle::FromRadians(0.f));
  }
  if (mPositionSpline.GetKnotCount() == 1) {
    return mOrientations[0];
  }

  const uint start = mPositionSpline.GetKnotIndexByLength(distance);
  const uint end = mPositionSpline.ValidateKnotIndex(start + 1);
  float span = mPositionSpline.GetKnotLength(end) - mPositionSpline.GetKnotLength(start);
  if (end < start) {
    span = mPositionSpline.GetKnotLength(end) +
           (mPositionSpline.GetLength() - mPositionSpline.GetKnotLength(start));
  }
  if (span < 0.01f) {
    return mOrientations[start];
  }
  const float t = CMath::Clamp(0.f, (distance - mPositionSpline.GetKnotLength(start)) / span, 1.f);
  CQuaternion first = mOrientations[start];
  if (CQuaternion::Dot(first, mOrientations[end]) < 0.f) {
    first = first.BuildEquivalent();
  }
  return CQuaternion::Slerp(first, mOrientations[end], t);
}

CVector3f CGameSpline::GetPositionByTime(float time) {
  CVector3f position = CVector3f::Zero();
  if (mPositionSpline.GetControlPointCount() != 0) {
    position = mPositionSpline.GetPositionByTime(mDuration * mPositionTimeSpline.EvaluateAt(time));
  }
  return position;
}

CVector3f CGameSpline::GetPositionByLength(float distance) {
  CVector3f position = CVector3f::Zero();
  if (mPositionSpline.GetKnotCount() >= 1) {
    position = mPositionSpline.GetPositionByLength(distance);
  }
  return position;
}

CVector3f CGameSpline::GetLookAtByTime(float time) {
  CVector3f position = CVector3f::Zero();
  float splineTime = mDuration * mLookAtTimeSpline.EvaluateAt(time);
  if (mFlags & kF_UsePositionTimeForLookAt) {
    splineTime = mDuration * mPositionTimeSpline.EvaluateAt(time);
  }
  if ((mFlags & kF_UsePositionForLookAt) && mPositionSpline.GetKnotCount() >= 1) {
    position = mPositionSpline.GetPositionByTime(splineTime);
  } else if (mLookAtSpline.GetKnotCount() >= 1) {
    position = mLookAtSpline.GetPositionByTime(splineTime);
  }
  return position;
}

CVector3f CGameSpline::GetLookAtByLength(float distance) {
  CVector3f position = CVector3f::Zero();
  if ((mFlags & kF_UsePositionForLookAt) && mPositionSpline.GetKnotCount() >= 1) {
    position = mPositionSpline.GetPositionByLength(distance);
  } else if (mLookAtSpline.GetKnotCount() >= 1) {
    position = mLookAtSpline.GetPositionByLength(distance);
  }
  return position;
}

float CGameSpline::FindClosestLengthOnSpline(float start, const CVector3f& position) const {
  return mPositionSpline.FindClosestLengthOnSpline(start, position);
}

float CGameSpline::ValidateLength(float distance) const {
  return mPositionSpline.ValidateLength(distance);
}

CMotionSpline& CGameSpline::PositionSpline() { return mPositionSpline; }

const CMotionSpline& CGameSpline::GetPositionSpline() const { return mPositionSpline; }

CMayaSpline& CGameSpline::PositionTimeSpline() { return mPositionTimeSpline; }

uint CGameSpline::GetPositionKnotCount() const { return mPositionSpline.GetKnotCount(); }

CMotionSpline& CGameSpline::LookAtSpline() { return mLookAtSpline; }

const CMotionSpline& CGameSpline::GetLookAtSpline() const { return mLookAtSpline; }

CMayaSpline& CGameSpline::LookAtTimeSpline() { return mLookAtTimeSpline; }

uint CGameSpline::GetLookAtKnotCount() const { return mLookAtSpline.GetKnotCount(); }
