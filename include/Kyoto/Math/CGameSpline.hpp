#ifndef _CGAMESPLINE
#define _CGAMESPLINE

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Math/CMotionSpline.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "rstl/vector.hpp"

// Guessed name. Runtime spline, distinct from the serialized CGameSplineDesc.
class CGameSpline {
public:
  // Guessed flag names, based on native construction and sampling controls.
  enum EFlags {
    kF_LoopPosition = 0x1,
    kF_LoopLookAt = 0x2,
    kF_UsePositionTimeForLookAt = 0x4,
    kF_UsePositionForLookAt = 0x8,
  };

  CGameSpline(float duration, uint flags, const CMayaSpline& positionTimeSpline,
              const CMayaSpline& lookAtTimeSpline, CMotionSpline::ESplineType positionType,
              CMotionSpline::ESplineType lookAtType);
  virtual ~CGameSpline();

  // Guessed name; replaces both motion paths and their position-key orientations.
  void Initialise(const rstl::vector< CVector3f >& positions,
                  const rstl::vector< CQuaternion >& orientations,
                  const rstl::vector< CVector3f >& lookAtPoints);

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
  uint GetFlags() const { return mFlags; }
  float FindClosestLengthOnSpline(float start, const CVector3f& position) const;
  float ValidateLength(float distance) const;
  CVector3f GetPositionByTime(float time);
  CVector3f GetPositionByLength(float distance);
  CVector3f GetLookAtByTime(float time);
  CVector3f GetLookAtByLength(float distance);
  CQuaternion GetOrientationByTime(float time);
  CQuaternion GetOrientationByLength(float distance);

protected:
  // Guessed semantic names, supported by construction and native samplers.
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
