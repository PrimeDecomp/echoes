#ifndef _CGAMESPLINE
#define _CGAMESPLINE

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Math/CMotionSpline.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "rstl/vector.hpp"

// Guessed name. Runtime spline, distinct from the serialized CGameSplineDesc.
class CGameSpline {
public:
  CGameSpline(float duration, uint flags, const SLdrSpline& positionTimeSpline,
              const SLdrSpline& lookAtTimeSpline, CMotionSpline::ESplineType positionType,
              CMotionSpline::ESplineType lookAtType);
  virtual ~CGameSpline();

  // Guessed accessor names, recovered from the script-camera callers.
  uint GetPositionKnotCount() const;
  uint GetLookAtKnotCount() const;
  CMotionSpline& PositionSpline();
  CMotionSpline& LookAtSpline();
  const CMotionSpline& GetPositionSpline() const;
  const CMotionSpline& GetLookAtSpline() const;
  CMayaSpline& PositionTimeSpline();
  CMayaSpline& LookAtTimeSpline();
  float GetLength() const;
  float GetDuration() const;
  float FindClosestLengthOnSpline(float start, const CVector3f& position) const;
  float ValidateLength(float distance) const;
  CVector3f GetPositionByTime(float time);
  CVector3f GetPositionByLength(float distance);
  CVector3f GetLookAtByTime(float time);
  CVector3f GetLookAtByLength(float distance);
  CQuaternion GetOrientationByTime(float time);
  CQuaternion GetOrientationByLength(float distance);

protected:
  CMotionSpline mPositionSpline;
  CMayaSpline mPositionTimeSpline;
  CMotionSpline mLookAtSpline;
  CMayaSpline mLookAtTimeSpline;
  rstl::vector< CQuaternion > mOrientations;
  float mDuration;
  uint mFlags;
};
CHECK_SIZEOF(CGameSpline, 0x12c)

#endif // _CGAMESPLINE
