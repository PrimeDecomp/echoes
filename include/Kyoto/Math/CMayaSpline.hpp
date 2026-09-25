#ifndef _CMAYASPLINE
#define _CMAYASPLINE

#include "Kyoto/Math/CVector2f.hpp"

#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CMayaSplineKnot {
  float mTime;
  float mAmplitude;
  uint mFlagA : 8;
  uint mFlagB : 8;
  uint mDirty : 1;
  // u8 x8_;
  // u8 x9_;
  // bool xa_24_dirty : 1;
  // u8 xb_;
  CVector2f mCachedTangentA;
  CVector2f mCachedTangentB;

public:
  CMayaSplineKnot(CInputStream& in);
  CMayaSplineKnot(float time, float amplitude, int flagA, int flagB, const float& tangentA = 0.f,
                  const float& tangentB = 0.f);
  bool operator<(const CMayaSplineKnot& other) const { return mTime < other.mTime; }

  float GetTime() const { return mTime; }
  float GetAmplitude() const { return mAmplitude; }
  int GetTangentModeA() const { return mFlagA; }
  int GetTangentModeB() const { return mFlagB; }
  void GetTangents(CMayaSplineKnot* prev, CMayaSplineKnot* next, CVector2f& tangentA,
                   CVector2f& tangentB);
  void CalculateTangents(CMayaSplineKnot* prev, CMayaSplineKnot* next);
};

struct SLdrSpline {
  SLdrSpline();
  SLdrSpline(const rstl::vector< CMayaSplineKnot >& knots, int clampMode, int preInfinity,
             int postInfinity, float minAmplitudeTime, float maxAmplitudeTime);
  SLdrSpline(CInputStream&, int);
  ~SLdrSpline();

  SLdrSpline& operator=(const SLdrSpline&);

  static SLdrSpline CreateFor(float, float, float, float);

  int m_preInfinity;
  int m_postInfinity;
  rstl::vector< CMayaSplineKnot > m_knots;
  int m_clampMode;
  float m_minAmplitudeTime;
  float m_maxAmplitudeTime;
  uint m_cachedKnotIndex;
  uint mCachedSegmentIndex;
  bool m_dirty : 1;
  float m_cachedMinTime;
  float m_cachedHermitCoefs[4];
};

class CMayaSpline {
  int mPreInfinity;
  int mPostInfinity;
  rstl::vector< CMayaSplineKnot > mKnots;
  int mClampMode;
  float mMinAmplitude;
  float mMaxAmplitude;
  int mCachedKnotIndex;
  int mCachedSegmentIndex;
  bool mStepSegment : 1;
  float mCachedMinTime;
  float mCachedHermiteCoefs[4];

public:
  CMayaSpline(CInputStream& in, int count);

  size_t GetKnotCount() const;
  const rstl::vector< CMayaSplineKnot >& GetKnots() const;
  float GetMaxTime() const;
  float GetDuration() const;

  rstl::pair< float, float > FindMaximumAmplitude();
  float FindFirstIntersection(float amplitude);
  float FindLastIntersection(float amplitude);
  void FindIntersections(float amplitude, rstl::reserved_vector< float, 8 >& intersections);
  rstl::reserved_vector< float, 8 >
  FilterLeftIntersections(const rstl::reserved_vector< float, 8 >& intersections);
  rstl::reserved_vector< float, 8 >
  FilterRightIntersections(const rstl::reserved_vector< float, 8 >& intersections);
  bool IsSegmentConstant(int knotIndex);
  void FindSegmentExtrema(int knotIndex,
                          rstl::reserved_vector< rstl::pair< float, float >, 2 >& extrema);
  void FindSegmentIntersections(float amplitude, int knotIndex,
                                rstl::reserved_vector< float, 3 >& intersections);

  float EvaluateAt(float time);
  float EvaluateAtUnclamped(float time);
  float EvaluateInfinities(float time, bool Pre);
  float EvaluateHermite(float time);
  bool FindKnot(float time, int& knotIndex);
  void FindControlPoints(int knotIndex, rstl::reserved_vector< CVector2f, 4 >& controlPoints);
  void CalculateHermiteCoefficients(const rstl::reserved_vector< CVector2f, 4 >& controlPoits,
                                    float* coefs);
};

CHECK_SIZEOF(CMayaSplineKnot, 0x1c)
CHECK_SIZEOF(CMayaSpline, 0x44)
CHECK_SIZEOF(SLdrSpline, 0x44)

#endif // _CMAYASPLINE
