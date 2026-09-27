#ifndef _CGAMESPLINE
#define _CGAMESPLINE

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Math/CMotionSpline.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "rstl/vector.hpp"

// Guessed name. Runtime spline, distinct from the serialized CGameSplineDesc.
class CGameSpline {
public:
  virtual ~CGameSpline();

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
