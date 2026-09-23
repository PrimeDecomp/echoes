#ifndef _CMAYASPLINE
#define _CMAYASPLINE

#include "Kyoto/Math/CVector2f.hpp"

#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CMayaSplineKnot {
  float x0_time;
  float x4_amplitude;
  uint x8_flagA : 8;
  uint x8_flagB : 8;
  uint x8_dirty : 1;
  // u8 x8_;
  // u8 x9_;
  // bool xa_24_dirty : 1;
  // u8 xb_;
  CVector2f xc_cachedTangentA;
  CVector2f x14_cachedTangentB;

public:
  CMayaSplineKnot(CInputStream& in);
  CMayaSplineKnot(float time, float amplitude, int flagA, int flagB, const float& tangentA = 0.f,
                  const float& tangentB = 0.f);
  bool operator<(const CMayaSplineKnot& other) const { return x0_time < other.x0_time; }

  float GetTime() const { return x0_time; }
  float GetAmplitude() const { return x4_amplitude; }
  int GetTangentModeA() const { return x8_flagA; }
  int GetTangentModeB() const { return x8_flagB; }
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
  uint x28_cachedSegmentIndex;
  bool m_dirty : 1;
  float m_cachedMinTime;
  float m_cachedHermitCoefs[4];
};

class CMayaSpline {
  int x0_preInfinity;
  int x4_postInfinity;
  rstl::vector< CMayaSplineKnot > x8_knots;
  int x18_clampMode;
  float x1c_minAmplitude;
  float x20_maxAmplitude;
  int x24_cachedKnotIndex;
  int x28_cachedSegmentIndex;
  bool x2c_24_stepSegment : 1;
  float x30_cachedMinTime;
  float x34_cachedHermiteCoefs[4];

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
