#ifndef _CMOTIONSPLINE
#define _CMOTIONSPLINE

#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/reserved_vector.hpp"
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
    kST_RoundedCatmullRom,
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
  CVector3f GetPositionByLength(float distance) const; // Guessed name.
  CVector3f GetTangentByTime(float time) const;        // Guessed name.
  CVector3f GetTangentByLength(float distance) const;  // Guessed name.
  void CalculateLength();
  void Reset(uint count);
  void AddKnotAndControlPoint(const CVector3f& point);
  void SetKnotAndControlPoint(uint index, const CVector3f& point, bool recalculateLength);

  float GetLength() const { return mLength; }
  float GetDuration() const { return mDuration; }
  void SetDuration(float duration) { mDuration = duration; }
  void SetSplineType(ESplineType type) { mType = type; }
  uint GetControlPointCount() const { return mControlPoints.size(); }
  uint GetKnotCount() const { return mKnots.size(); }
  CVector3f GetKnot(uint index) const; // Guessed name; respects closed-loop index wrapping.
  CVector3f GetControlPoint(uint index) const;
  int GetKnotIndexByLength(float distance) const; // Guessed name; searches knot arc lengths.
  // Guessed names, recovered from game-spline orientation interpolation.
  uint GetKnotIndexByTime(float time) const;
  uint ValidateKnotIndex(uint index) const;
  float GetKnotTime(uint index) const;
  float GetKnotLength(uint index) const;
  bool IsClosedLoop() const { return mClosedLoop; }
  void SetClosedLoop(bool closedLoop) { mClosedLoop = closedLoop; } // Guessed name.

private:
  // Guessed names, recovered from native spline mutation and sampling behavior.
  uint ValidateControlPointIndex(uint index) const;
  void SetControlPoint(uint index, CVector3f point, bool recalculateLength);
  void SetKnot(uint index, CVector3f point, bool recalculateLength);
  void AddKnot(const CVector3f& point);
  void AddControlPoint(const CVector3f& point);
  void ResetKnots(uint count);
  void ResetControlPoints(uint count);
  void GetSurroundingPoints(int index, rstl::reserved_vector< CVector3f, 4 >& points) const;
  float CalculateCatmullRomLength(int index) const;
  float CalculateRoundedCatmullRomLength(int index) const;
  float CalculateBSplineLength(int index) const;
  float CalculateBezierLength(const CVector3f& a, const CVector3f& b, const CVector3f& c,
                              const CVector3f& d) const;
  float CalculateBezierLength(int index) const;
  CVector3f GetPositionInSegment(uint index, float t) const;
  float GetSegmentParameter(float distance, uint index) const;
  float FindClosestBezierLength(float start, const CVector3f& position) const;

  // Guessed member names; constructor, mutation and sampling offsets establish the roles.
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
