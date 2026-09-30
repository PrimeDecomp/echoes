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

namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CMayaSplineKnot)
}

class CMayaSpline {
  struct SCache {
    SCache()
    : mKnotIndex(-1), mSegmentIndex(-1), mStepSegment(false), mMinTime(0.f) {}

    int mKnotIndex;
    int mSegmentIndex;
    bool mStepSegment : 1;
    float mMinTime;
    float mHermiteCoefs[4];
  };

  int mPreInfinity;
  int mPostInfinity;
  rstl::vector< CMayaSplineKnot > mKnots;
  int mClampMode;
  float mMinAmplitude;
  float mMaxAmplitude;
  mutable SCache mCache;

public:
  CMayaSpline();
  CMayaSpline(const rstl::vector< CMayaSplineKnot >& knots, int clampMode, int preInfinity,
              int postInfinity, float minAmplitude, float maxAmplitude);
  CMayaSpline(CInputStream& in, int count);
  ~CMayaSpline() {}

  static CMayaSpline CreateFor(float timeA, float amplitudeA, float timeB, float amplitudeB);

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

// Compatibility name used by generated script-loader declarations.
typedef CMayaSpline SLdrSpline;

CHECK_SIZEOF(CMayaSplineKnot, 0x1c)
CHECK_SIZEOF(CMayaSpline, 0x44)
CHECK_SIZEOF(SLdrSpline, 0x44)

#endif // _CMAYASPLINE
