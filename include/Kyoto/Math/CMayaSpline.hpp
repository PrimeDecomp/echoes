#ifndef _CMAYASPLINE
#define _CMAYASPLINE

#include "Kyoto/Math/CVector2f.hpp"

#include "rstl/vector.hpp"
#include "rstl/reserved_vector.hpp"

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
  CMayaSplineKnot(float time, float amplitude, int, int, const float*, const float*);

  float GetTime() const { return x0_time; }
  float GetAmplitude() const { return x4_amplitude; }
  u8 GetX8() const { return x8_flagA; }
  u8 GetX9() const { return x8_flagB; }
  void GetTangents(CMayaSplineKnot* prev, CMayaSplineKnot* next, CVector2f& tangentA, CVector2f& tangentB);
  void CalculateTangents(CMayaSplineKnot* prev, CMayaSplineKnot* next);
};


struct SLdrSpline {
  SLdrSpline();
  SLdrSpline(const rstl::vector<CMayaSplineKnot>& knots, int clampMode, int preInfinity, int postInfinity, float minAmplitudeTime, float maxAmplitudeTime);
  SLdrSpline(CInputStream&, int);
  ~SLdrSpline();

  SLdrSpline& operator=(const SLdrSpline&);

  static SLdrSpline CreateFor(float, float, float, float);

  int m_preInfinity;
  int m_postInfinity;
  rstl::vector<CMayaSplineKnot> m_knots;
  int m_clampMode;
  float m_minAmplitudeTime;
  float m_maxAmplitudeTime;
  uint m_cachedKnotIndex;
  uint x28_;
  bool m_dirty : 1;
  float m_cachedMinTime;
  float m_cachedHermitCoefs[4];
};


class CMayaSpline {
  uint x0_preInfinity;
  uint x4_postInfinity;
  rstl::vector<CMayaSplineKnot> x8_knots;
  uint x18_clampMode;
  float x1c_minAmplitudeTime;
  float x20_maxAmplitudeTime;
  int x24_chachedKnotIndex;
  int x28_;
  bool x2c_24_dirty;
  float x30_cachedMinTime;
  float x34_cachedHermitCoefs[4];

public:
  CMayaSpline(CInputStream& in, int count);
  
  size_t GetKnotCount() const;
  const rstl::vector<CMayaSplineKnot>& GetKnots() const;
  float GetMaxTime() const;
  float GetDuration() const;

  float EvaluateAt(float time);
  float EvaluateAtUnclamped(float time);
  float EvaluateInfinities(float time, bool Pre);
  float EvaluateHermite(float time);
  bool FindKnot(float time, int& knotIndex);
  void FindControlPoints(int knotIndex, rstl::reserved_vector<CVector2f, 4>& controlPoints);
  void CalculateHermiteCoefficients(const rstl::reserved_vector<CVector2f, 4>& controlPoits, float* coefs);
};

#endif // _CMAYASPLINE
