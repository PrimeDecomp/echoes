#ifndef _CMOTIONSPLINE
#define _CMOTIONSPLINE

#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/vector.hpp"

class CQuaternion;

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
  void Translate(const CVector3f& offset);
  void Rotate(const CQuaternion& rotation, const CVector3f& origin);
  CVector3f GetInterpolatedSplinePointByTime(float time) const;
  float FindClosestLengthOnSpline(float start, const CVector3f& position) const;
  float ValidateLength(float distance) const;
  // Guessed names; time is mapped through arc length rather than segment parameterization.
  CVector3f GetPositionByTime(float time) const;
  CVector3f GetTangentByTime(float time) const; // Guessed name.
  void CalculateLength();
  void SetKnotAndControlPoint(uint index, const CVector3f& point, bool recalculateLength);

  float GetLength() const { return mLength; }
  float GetDuration() const { return mDuration; }
  int GetControlPointCount() const { return mControlPoints.size(); }
  int GetKnotCount() const { return mKnots.size(); }
  bool IsClosedLoop() const { return mClosedLoop; }

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
