#ifndef _CMOTIONSPLINE
#define _CMOTIONSPLINE

#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/vector.hpp"

class CMotionSpline {
public:
  // Guessed enumerator names, based on the interpolation formulas.
  enum ESplineType {
    kST_CatmullRom,
    kST_BSpline,
    kST_Linear,
    kST_Bezier,
    kST_NormalizedHermite,
  };

  CMotionSpline(bool closedLoop, float duration, ESplineType type);
  virtual ~CMotionSpline();

  void Initialise(const rstl::vector< CVector3f >& points);
  CVector3f GetInterpolatedSplinePointByTime(float time) const;

  float GetLength() const { return mLength; }
  float GetDuration() const { return mDuration; }
  int GetControlPointCount() const { return mControlPoints.size(); }

private:
  rstl::vector< CVector3f > mControlPoints;
  rstl::vector< CVector3f > mKnots;
  rstl::vector< float > mKnotDistances;
  float mLength;
  float mDuration;
  bool mClosedLoop : 1;
  ESplineType mType;
};
CHECK_SIZEOF(CMotionSpline, 0x44)

#endif // _CMOTIONSPLINE
