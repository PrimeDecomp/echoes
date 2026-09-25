#include "Kyoto/Math/CMayaSpline.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#include "rstl/algorithm.hpp"

#include "float.h"
#include "math.h"

// Polynomial solvers; original GameCube names have not been established.
extern "C" int fn_802CB608(double c0, double c1, double c2, double c3, double tolerance,
                           double* roots);
extern "C" bool fn_802CC064(float a, float b, float c, float& rootA, float& rootB);

CMayaSplineKnot::CMayaSplineKnot(CInputStream& in)
: mTime(in.ReadFloat())
, mAmplitude(in.ReadFloat())
, mFlagA(in.ReadInt8())
, mFlagB(in.ReadInt8())
, mDirty(true)
, mCachedTangentA(CVector2f(0.f, 0.f))
, mCachedTangentB(CVector2f(0.f, 0.f)) {
  if (mFlagA == 5) {
    float x = in.ReadFloat();
    float y = in.ReadFloat();
    mCachedTangentA.SetX(x);
    mCachedTangentA.SetY(y);
  }
  if (mFlagB == 5) {
    float x = in.ReadFloat();
    float y = in.ReadFloat();
    mCachedTangentB.SetX(x);
    mCachedTangentB.SetY(y);
  }
}

CMayaSplineKnot::CMayaSplineKnot(float time, float amplitude, int flagA, int flagB, const float& v1,
                                 const float& v2)
: mTime(time)
, mAmplitude(amplitude)
, mFlagA(flagA)
, mFlagB(flagB)
, mDirty(true)
, mCachedTangentA(CVector2f(0.0f, 0.0f))
, mCachedTangentB(CVector2f(0.0f, 0.0f)) {
  if (flagA == 5) {
    float s = sin(v1);
    float c = cos(v1);
    mCachedTangentA = CVector2f(3.0f * c, 3.0f * s);
  }
  if (flagB == 5) {
    float s = sin(v2);
    float c = cos(v2);
    mCachedTangentB = CVector2f(3.0f * c, 3.0f * s);
  }
}

void CMayaSplineKnot::GetTangents(CMayaSplineKnot* prev, CMayaSplineKnot* next, CVector2f& tangentA,
                                  CVector2f& tangentB) {
  if (mDirty) {
    CalculateTangents(prev, next);
  }

  tangentA = mCachedTangentA;
  tangentB = mCachedTangentB;
}

void ValidateTangent(CVector2f& tangent) {
  if (tangent.GetX() < 0.f) {
    tangent.SetX(0.f);
  }

  const float mag = tangent.Magnitude();
  if (mag != 0.f) {
    tangent /= mag;
  }

  if (tangent.GetX() == 0.f && tangent.GetY() != 0.f) {
    tangent.SetX(0.0001f);
    tangent.SetY(5729578.0f * tangent.GetX() * (tangent.GetY() < 0.f ? -1.f : 1.f));
  }
}

void CMayaSplineKnot::CalculateTangents(CMayaSplineKnot* prev, CMayaSplineKnot* next) {
  mDirty = false;
  bool calculateTangents = false;
  if (mFlagA == 4 && prev != nullptr) {
    float prevAmplitude = CMath::AbsF(prev->GetAmplitude() - GetAmplitude());
    float nextAmplitude = prevAmplitude;
    if (next != nullptr) {
      nextAmplitude = CMath::AbsF(next->GetAmplitude() - GetAmplitude());
    }
    if (nextAmplitude <= 0.05f || prevAmplitude <= 0.05f) {
      mFlagA = 1;
    }
  }

  switch (mFlagA) {
  case 0:
    if (prev == nullptr) {
      mCachedTangentA = CVector2f(1.f, 0.f);
    } else {
      mCachedTangentA =
          CVector2f(GetTime() - prev->GetTime(), GetAmplitude() - prev->GetAmplitude());
    }
    break;
  case 1: {
    float time = 0.f;
    if (prev != nullptr) {
      time = GetTime() - prev->GetTime();
    } else if (next != nullptr) {
      time = next->GetTime() - GetTime();
    }
    mCachedTangentA = CVector2f(time, 0.f);
    break;
  }
  case 3:
    mCachedTangentA = CVector2f(1.f, 0.f);
    break;
  case 4:
    mFlagA = 2;
  case 2:
    calculateTangents = true;
    break;
  }

  if (mFlagB == 4 && next != nullptr) {
    float nextAmplitude = CMath::AbsF(next->GetAmplitude() - GetAmplitude());
    float prevAmplitude = nextAmplitude;
    if (prev != nullptr) {
      prevAmplitude = CMath::AbsF(prev->GetAmplitude() - GetAmplitude());
    }
    if (nextAmplitude <= 0.05f || prevAmplitude <= 0.05f) {
      mFlagB = 1;
    }
  }

  switch (mFlagB) {
  case 0:
    if (next == nullptr) {
      mCachedTangentB = CVector2f(1.f, 0.f);
    } else {
      mCachedTangentB =
          CVector2f(next->GetTime() - GetTime(), next->GetAmplitude() - GetAmplitude());
    }
    break;
  case 1: {
    float time = 0.f;
    if (next != nullptr) {
      time = next->GetTime() - GetTime();
    } else if (prev != nullptr) {
      time = GetTime() - prev->GetTime();
    }
    mCachedTangentB = CVector2f(time, 0.f);
    break;
  }
  case 3:
    mCachedTangentB = CVector2f(1.f, 0.f);
    break;
  case 4:
    mFlagB = 2;
  case 2:
    calculateTangents = true;
    break;
  }

  if (calculateTangents) {
    CVector2f tangentA(0.f, 0.f);
    CVector2f tangentB(0.f, 0.f);
    if (prev == nullptr && next != nullptr) {
      tangentA = tangentB =
          CVector2f(next->GetTime() - GetTime(), next->GetAmplitude() - GetAmplitude());
    } else if (prev != nullptr && next == nullptr) {
      tangentA = tangentB =
          CVector2f(GetTime() - prev->GetTime(), GetAmplitude() - prev->GetAmplitude());
    } else if (prev != nullptr && next != nullptr) {
      float timeDiff = next->GetTime() - prev->GetTime();
      float amplitudeDiff = next->GetAmplitude() - prev->GetAmplitude();
      float slope = timeDiff >= 0.0001f ? amplitudeDiff / timeDiff
                                        : (amplitudeDiff <= 0.f ? -5729578.f : 5729578.f);
      float nextTime = next->GetTime() - GetTime();
      float prevTime = GetTime() - prev->GetTime();
      float nextAmplitude;
      float prevAmplitude = slope;
      if (nextTime >= 0.0001f) {
        nextAmplitude = nextTime * slope;
      } else {
        nextTime = 0.f;
        nextAmplitude = slope;
      }
      if (prevTime >= 0.0001f) {
        prevAmplitude = prevTime * slope;
      } else {
        prevTime = 0.f;
      }
      tangentB = CVector2f(prevTime, prevAmplitude);
      tangentA = CVector2f(nextTime, nextAmplitude);
    } else {
      tangentA = CVector2f(1.f, 0.f);
      tangentB = CVector2f(1.f, 0.f);
    }
    if (mFlagA == 2) {
      mCachedTangentA = tangentA;
    }
    if (mFlagB == 2) {
      mCachedTangentB = tangentB;
    }
  }
  ValidateTangent(mCachedTangentA);
  ValidateTangent(mCachedTangentB);
}

SLdrSpline::SLdrSpline(CInputStream& in, int count)
: m_preInfinity(in.ReadInt8())
, m_postInfinity(in.ReadInt8())
, m_knots(in)
, m_clampMode(in.ReadInt8())
, m_minAmplitudeTime(in.ReadFloat())
, m_maxAmplitudeTime(in.ReadFloat())

, m_cachedKnotIndex(0xFFFFFFFF)
, mCachedSegmentIndex(0xFFFFFFFF)
, m_dirty(false)
, m_cachedMinTime(0.0f) {}

SLdrSpline::SLdrSpline()
: m_preInfinity(0)
, m_postInfinity(0)
, m_knots()
, m_clampMode(0)
, m_cachedKnotIndex(0xFFFFFFFF)
, mCachedSegmentIndex(0xFFFFFFFF)
, m_dirty(false)
, m_cachedMinTime(0.0f) {}

SLdrSpline::SLdrSpline(const rstl::vector< CMayaSplineKnot >& knots, int clampMode, int preInfinity,
                       int postInfinity, float minAmplitudeTime, float maxAmplitudeTime)
: m_preInfinity(preInfinity)
, m_postInfinity(postInfinity)
, m_knots(knots)
, m_clampMode(clampMode)
, m_minAmplitudeTime(minAmplitudeTime)
, m_maxAmplitudeTime(maxAmplitudeTime)

, m_cachedKnotIndex(0xFFFFFFFF)
, mCachedSegmentIndex(0xFFFFFFFF)
, m_dirty(false)
, m_cachedMinTime(0.0f) {
  rstl::sort(m_knots.begin(), m_knots.end(), rstl::less< CMayaSplineKnot >());
}

float CMayaSpline::EvaluateHermite(float time) {
  const float timeDiff = time - mCachedMinTime;
  return ((timeDiff * mCachedHermiteCoefs[0] + mCachedHermiteCoefs[1]) * timeDiff +
          mCachedHermiteCoefs[2]) *
             timeDiff +
         mCachedHermiteCoefs[3];
}

float CMayaSpline::EvaluateInfinities(float time, bool pre) {
  if (mKnots.empty()) {
    return 0.f;
  }

  int lastIdx = mKnots.size() - 1;
  CMayaSplineKnot* curKnot = &mKnots[0];
  const float startTime = mKnots[0].GetTime();
  const float endTime = mKnots[lastIdx].GetTime();
  float center = endTime - startTime;

  if (CMath::IsEpsilon(center, 0, 1.e-5f)) {
    return curKnot->GetAmplitude();
  }

  double tmp;
  float divTime;
  if (time > endTime) {
    divTime = CMath::AbsF(modf((time - endTime) / center, &tmp));
  } else {
    divTime = CMath::AbsF(modf((time - startTime) / center, &tmp));
  }
  center *= divTime;
  tmp = 1.f + CMath::AbsF(tmp);

  if (pre) {
    if (mPreInfinity == 4) {
      divTime = CMath::ModF(tmp, 2.f);
      if (!CMath::IsEpsilon(divTime, 0.f, 1.e-5f)) {
        center = startTime + center;
      } else {
        center = endTime - center;
      }
    } else if (mPreInfinity == 2 || mPreInfinity == 3) {
      center = endTime - center;
    } else if (mPreInfinity == 1) {
      center = (startTime - time);
      CVector2f tangentA(0.0f, 0.0f);
      CVector2f tangentB(0.0f, 0.0f);
      mKnots[0].GetTangents(nullptr, &mKnots[1], tangentA, tangentB);
      const float amplitude = mKnots[0].GetAmplitude();
      return !CMath::IsEpsilon(tangentA.GetX(), 0.f, 1.e-5f)
                 ? amplitude - center * tangentA.GetY() / tangentA.GetX()
                 : amplitude;
    }
  } else {
    if (mPostInfinity == 4) {
      divTime = CMath::ModF(tmp, 2.f);
      if (!CMath::IsEpsilon(divTime, 0.f, 1.e-5f)) {
        center = endTime - center;
      } else {
        center = startTime + center;
      }
    } else if (mPostInfinity == 2 || mPostInfinity == 3) {
      center = startTime + center;
    } else if (mPostInfinity == 1) {
      center = time - endTime;
      CVector2f tangentA(0.0f, 0.0f);
      CVector2f tangentB(0.0f, 0.0f);
      mKnots[lastIdx].GetTangents(lastIdx > 0 ? &mKnots[lastIdx - 1] : nullptr, nullptr,
                                    tangentA, tangentB);
      const float amplitude = mKnots[lastIdx].GetAmplitude();
      return !CMath::IsEpsilon(tangentB.GetX(), 0.f, 1.e-5f)
                 ? amplitude + center * tangentB.GetY() / tangentB.GetX()
                 : amplitude;
    }
  }

  float eval = EvaluateAt(center);
  if (pre && mPreInfinity == 3) {
    return eval - (float(tmp) * (mKnots[lastIdx].GetAmplitude() - mKnots[0].GetAmplitude()));
  }

  if (!pre && mPostInfinity == 3) {
    return eval + (float(tmp) * (mKnots[lastIdx].GetAmplitude() - mKnots[0].GetAmplitude()));
  }
  return eval;
}

bool CMayaSpline::FindKnot(float time, int& knotIndex) {
  knotIndex = 0;
  int count = mKnots.size();
  if (count != 0) {
    int lower = 0;
    int upper = count - 1;
    do {
      int index = (lower + upper) >> 1;
      const CMayaSplineKnot& knot = mKnots[index];
      if (time < knot.GetTime()) {
        upper = index - 1;
      } else if (time > knot.GetTime()) {
        lower = index + 1;
      } else {
        knotIndex = index;
        return true;
      }
    } while (lower <= upper);
    knotIndex = lower;
  }

  return false;
}

float CMayaSpline::EvaluateAt(float time) {
  float amplitude = EvaluateAtUnclamped(time);
  switch (mClampMode) {
  case 0:
    break;
  case 1: {
    const float min = mMinAmplitude;
    const float max = mMaxAmplitude;
    if (min > amplitude) {
      return min;
    }
    if (max < amplitude) {
      return max;
    }
    return amplitude;
  }
  case 2: {
    const float max = mMaxAmplitude;
    const float min = mMinAmplitude;
    float center = max - min;

    if (center > 0.f) {
      if (amplitude > FLT_EPSILON + mMaxAmplitude) {
        return amplitude -
               (center * static_cast< float >(int((amplitude - mMaxAmplitude) / center) + 1));
      }
      if (amplitude < min - FLT_EPSILON) {
        return amplitude +
               (center * static_cast< float >(abs(int((amplitude - min) / center)) + 1));
      }
      return amplitude;
    }
    return min;
  }
  }

  return amplitude;
}

float CMayaSpline::EvaluateAtUnclamped(float time) {
  if (mKnots.empty()) {
    return 0.f;
  }

  int lastIdx = mKnots.size() - 1;
  bool segmentKnown = false;
  float nextTime;
  if (time < mKnots[0].GetTime()) {
    if (mPreInfinity == 0) {
      return mKnots[0].GetAmplitude();
    }
    return EvaluateInfinities(time, true);
  } else if (mKnots[lastIdx].GetTime() >= time) {
    segmentKnown = false;
    int nextKnotIndex = -1;
    int cachedKnotIndex = mCachedKnotIndex;
    if (cachedKnotIndex != -1) {
      if (lastIdx <= cachedKnotIndex || mKnots[lastIdx].GetTime() >= time) {
        if (cachedKnotIndex > 0 && mKnots[cachedKnotIndex].GetTime() > time) {
          int previousKnotIndex = cachedKnotIndex - 1;
          segmentKnown = mKnots[previousKnotIndex].GetTime() < time;
          if (segmentKnown) {
            nextKnotIndex = cachedKnotIndex;
          }
          if (mKnots[previousKnotIndex].GetTime() == time) {
            mCachedKnotIndex = previousKnotIndex;
            return mKnots[mCachedKnotIndex].GetAmplitude();
          }
        }
      } else {
        nextTime = mKnots[cachedKnotIndex + 1].GetTime();
        if (nextTime == time) {
          mCachedKnotIndex = lastIdx;
          return mKnots[mCachedKnotIndex].GetAmplitude();
        }

        if (nextTime > time) {
          segmentKnown = true;
          nextKnotIndex = cachedKnotIndex + 1;
        }
      }
    }

    if (!segmentKnown && (FindKnot(time, nextKnotIndex))) {
      if (nextKnotIndex == 0) {
        mCachedKnotIndex = 0;
        return mKnots[0].GetAmplitude();
      }
      if (nextKnotIndex == mKnots.size()) {
        mCachedKnotIndex = 0;
        return mKnots[lastIdx].GetAmplitude();
      }
    }

    lastIdx = nextKnotIndex - 1;
    if (mCachedSegmentIndex != lastIdx) {
      mCachedKnotIndex = lastIdx;
      mCachedSegmentIndex = lastIdx;
      if (mKnots[mCachedKnotIndex].GetTangentModeB() == 3) {
        mStepSegment = true;
      } else {
        mStepSegment = false;
        rstl::reserved_vector< CVector2f, 4 > points;
        FindControlPoints(mCachedKnotIndex, points);
        CalculateHermiteCoefficients(points, mCachedHermiteCoefs);
        mCachedMinTime = points[0].GetX();
      }
    }

    if (mStepSegment) {
      return mKnots[mCachedKnotIndex].GetAmplitude();
    } else {
      return EvaluateHermite(time);
    }
  }

  if (mPostInfinity == 0) {
    return mKnots[lastIdx].GetAmplitude();
  }

  return EvaluateInfinities(time, false);
}

SLdrSpline SLdrSpline::CreateFor(float timeA, float amplitudeA, float timeB, float amplitudeB) {
  rstl::vector< CMayaSplineKnot > knots;
  knots.reserve(2);
  knots.push_back_unsafe(CMayaSplineKnot(timeA, amplitudeA, 2, 2, 0.f, 0.f));
  knots.push_back_unsafe(CMayaSplineKnot(timeB, amplitudeB, 2, 2, 0.f, 0.f));
  return SLdrSpline(knots, 0, 0, 0, -FLT_MAX, FLT_MAX);
}

const rstl::vector< CMayaSplineKnot >& CMayaSpline::GetKnots() const { return mKnots; }

float CMayaSpline::GetMaxTime() const {
  int count = mKnots.size();
  if (count == 0) {
    return 0.0f;
  }
  return mKnots[count - 1].GetTime();
}

float CMayaSpline::GetDuration() const {
  if (mKnots.empty()) {
    return 0.0f;
  }
  return GetMaxTime() - mKnots[0].GetTime();
}

size_t CMayaSpline::GetKnotCount() const { return mKnots.size(); }

void CMayaSpline::FindControlPoints(int knotIndex,
                                    rstl::reserved_vector< CVector2f, 4 >& controlPoints) {
  CMayaSplineKnot* knot = &mKnots[knotIndex];
  controlPoints.push_back(CVector2f(knot->GetTime(), knot->GetAmplitude()));

  CVector2f tangentA(0.f, 0.f);
  CVector2f tangentB(0.f, 0.f);
  CMayaSplineKnot* next = knotIndex + 1 < mKnots.size() ? &mKnots[knotIndex + 1] : nullptr;
  CMayaSplineKnot* prev = knotIndex - 1 >= 0 ? &mKnots[knotIndex - 1] : nullptr;
  knot->GetTangents(prev, next, tangentA, tangentB);
  controlPoints.push_back(controlPoints[0] + tangentB * (1.f / 3.f));

  knot = &mKnots[knotIndex + 1];
  CVector2f nextTangentA(0.f, 0.f);
  CVector2f nextTangentB(0.f, 0.f);
  next = knotIndex + 2 < mKnots.size() ? &mKnots[knotIndex + 2] : nullptr;
  prev = knotIndex >= 0 ? &mKnots[knotIndex] : nullptr;
  knot->GetTangents(prev, next, nextTangentA, nextTangentB);
  CVector2f knotPoint(knot->GetTime(), knot->GetAmplitude());
  controlPoints.push_back(knotPoint - nextTangentA * (1.f / 3.f));
  controlPoints.push_back(knotPoint);
}

void CMayaSpline::CalculateHermiteCoefficients(
    const rstl::reserved_vector< CVector2f, 4 >& controlPoints, float* coefs) {
  const CVector2f span = controlPoints[3] - controlPoints[0];
  const float time = span.GetX();
  const float amplitude = span.GetY();
  const CVector2f tangentA = controlPoints[1] - controlPoints[0];
  float slopeA = 5729578.f;
  if (tangentA.GetX() != 0.f) {
    slopeA = tangentA.GetY() / tangentA.GetX();
  }
  const CVector2f tangentB = controlPoints[3] - controlPoints[2];
  float slopeB = 5729578.f;
  if (tangentB.GetX() != 0.f) {
    slopeB = tangentB.GetY() / tangentB.GetX();
  }
  const float invTimeSq = 1.f / (time * time);
  const float amplitudeA = slopeA * time;
  const float amplitudeB = slopeB * time;
  coefs[0] = invTimeSq * ((amplitudeA + amplitudeB - amplitude) - amplitude) / time;
  coefs[1] =
      invTimeSq * (((amplitude + amplitude + amplitude - amplitudeA) - amplitudeA) - amplitudeB);
  coefs[2] = slopeA;
  coefs[3] = controlPoints[0].GetY();
}

void CMayaSpline::FindSegmentIntersections(float amplitude, int knotIndex,
                                           rstl::reserved_vector< float, 3 >& intersections) {
  if (mKnots[knotIndex].GetTangentModeB() == 3) {
    if (CMath::IsEpsilon(mKnots[knotIndex].GetAmplitude(), amplitude, 0.002f)) {
      intersections.push_back(mKnots[knotIndex].GetTime());
      intersections.push_back(mKnots[knotIndex + 1].GetTime());
    }
    return;
  }

  rstl::reserved_vector< CVector2f, 4 > points;
  FindControlPoints(knotIndex, points);
  float coefs[4];
  CalculateHermiteCoefficients(points, coefs);
  coefs[3] -= amplitude;
  double roots[4];
  int count = fn_802CB608(coefs[3], coefs[2], coefs[1], coefs[0], FLT_EPSILON, roots);
  const float start = mKnots[knotIndex].GetTime();
  const float end = mKnots[knotIndex + 1].GetTime();
  if (count < 0 && CMath::IsEpsilon(mKnots[knotIndex].GetAmplitude(), amplitude, 0.002f)) {
    count = 2;
    roots[0] = 0.0;
    roots[1] = end - start;
  }
  if (count > 0 && intersections.size() != 3) {
    for (int i = 0; i < count; ++i) {
      float time = start + float(roots[i]);
      if (CMath::IsEpsilon(time, start, 0.002f)) {
        time = start;
      }
      if (CMath::IsEpsilon(time, end, 0.002f)) {
        time = end;
      }
      if (time >= start && time <= end) {
        intersections.push_back(time);
      }
    }
  }
  rstl::sort(intersections.begin(), intersections.end());
}

void CMayaSpline::FindIntersections(float amplitude,
                                    rstl::reserved_vector< float, 8 >& intersections) {
  if (mKnots.size() < 2) {
    return;
  }
  rstl::reserved_vector< float, 3 > segmentIntersections;
  for (int i = 0; i < mKnots.size() - 1; ++i) {
    segmentIntersections.clear();
    FindSegmentIntersections(amplitude, i, segmentIntersections);
    if (!segmentIntersections.empty()) {
      for (int j = 0; j < segmentIntersections.size(); ++j) {
        if (intersections.size() != 8) {
          intersections.push_back(segmentIntersections[j]);
        }
      }
    }
  }

  float* last = intersections.begin();
  for (float* next = last + 1; next < intersections.end(); ++next) {
    if (!CMath::IsEpsilon(*next, *last, 0.002f)) {
      *++last = *next;
    }
  }
  const int count = last - intersections.begin() + 1;
  intersections.resize(count, 0.f);
}

rstl::reserved_vector< float, 8 >
CMayaSpline::FilterLeftIntersections(const rstl::reserved_vector< float, 8 >& intersections) {
  rstl::reserved_vector< float, 8 > result;
  for (int i = 0; i < intersections.size(); ++i) {
    const float time = intersections[i];
    for (int j = 0; j < mKnots.size() - 1; ++j) {
      const float start = mKnots[j].GetTime();
      const float end = mKnots[j + 1].GetTime();
      bool atEndpoint =
          CMath::IsEpsilon(time, start, 0.002f) || CMath::IsEpsilon(time, end, 0.002f);
      if (atEndpoint || (start <= time && time <= end)) {
        if (!IsSegmentConstant(j)) {
          result.push_back(time);
          break;
        }
        if (!CMath::IsEpsilon(time, start, 0.002f)) {
          break;
        }
        result.push_back(time);
        if (result.size() == 8) {
          break;
        }
      }
    }
  }
  return result;
}

rstl::reserved_vector< float, 8 >
CMayaSpline::FilterRightIntersections(const rstl::reserved_vector< float, 8 >& intersections) {
  rstl::reserved_vector< float, 8 > result;
  for (int i = 0; i < intersections.size(); ++i) {
    const float time = intersections[i];
    for (int j = mKnots.size() - 2; j >= 0; --j) {
      const float start = mKnots[j].GetTime();
      const float end = mKnots[j + 1].GetTime();
      bool atEndpoint =
          CMath::IsEpsilon(time, start, 0.002f) || CMath::IsEpsilon(time, end, 0.002f);
      if (atEndpoint || (start <= time && time <= end)) {
        if (!IsSegmentConstant(j)) {
          result.push_back(time);
          break;
        }
        if (!CMath::IsEpsilon(time, end, 0.002f)) {
          break;
        }
        result.push_back(time);
      }
    }
  }
  return result;
}

bool CMayaSpline::IsSegmentConstant(int knotIndex) {
  if (knotIndex > mKnots.size() - 2) {
    return false;
  }
  CVector2f tangentA = CVector2f::Zero();
  CVector2f tangentB = CVector2f::Zero();
  mKnots[knotIndex].GetTangents(nullptr, &mKnots[knotIndex + 1], tangentA, tangentB);
  if (CMath::IsEpsilon(mKnots[knotIndex].GetAmplitude(), 0.f, 1.e-5f) && tangentB.GetY() < 0.f) {
    tangentB.SetY(0.f);
  }
  if (CMath::IsEpsilon(mKnots[knotIndex].GetAmplitude(), 1.f, 1.e-5f) && tangentB.GetY() > 0.f) {
    tangentB.SetY(0.f);
  }
  CVector2f nextTangentA = CVector2f::Zero();
  CVector2f nextTangentB = CVector2f::Zero();
  mKnots[knotIndex + 1].GetTangents(&mKnots[knotIndex], nullptr, nextTangentA, nextTangentB);
  if (CMath::IsEpsilon(mKnots[knotIndex + 1].GetAmplitude(), 0.f, 1.e-5f) &&
      nextTangentA.GetY() > 0.f) {
    nextTangentA.SetY(0.f);
  }
  if (CMath::IsEpsilon(mKnots[knotIndex + 1].GetAmplitude(), 1.f, 1.e-5f) &&
      nextTangentA.GetY() < 0.f) {
    nextTangentA.SetY(0.f);
  }
  if (mKnots[knotIndex].GetTangentModeB() == 3 ||
      (CMath::IsEpsilon(mKnots[knotIndex].GetAmplitude(), mKnots[knotIndex + 1].GetAmplitude(),
                        1.e-5f) &&
       CMath::IsEpsilon(tangentB.GetY(), 0.f, 1.e-5f) &&
       CMath::IsEpsilon(nextTangentA.GetY(), 0.f, 1.e-5f))) {
    return true;
  }
  return false;
}

void CMayaSpline::FindSegmentExtrema(
    int knotIndex, rstl::reserved_vector< rstl::pair< float, float >, 2 >& extrema) {
  typedef rstl::pair< float, float > Point;
  const CMayaSplineKnot& knot = mKnots[knotIndex];
  if (knot.GetTangentModeB() == 3) {
    extrema.push_back(Point(knot.GetTime(), knot.GetAmplitude()));
    extrema.push_back(Point(mKnots[knotIndex + 1].GetTime(), knot.GetAmplitude()));
  } else if (knot.GetTangentModeB() == 0) {
    extrema.push_back(Point(knot.GetTime(), knot.GetAmplitude()));
    const CMayaSplineKnot& next = mKnots[knotIndex + 1];
    extrema.push_back(Point(next.GetTime(), next.GetAmplitude()));
  } else {
    rstl::reserved_vector< CVector2f, 4 > points;
    FindControlPoints(knotIndex, points);
    float coefs[4];
    CalculateHermiteCoefficients(points, coefs);
    float rootA = 0.f;
    float rootB = 0.f;
    bool found = fn_802CC064(3.f * coefs[0], 2.f * coefs[1], coefs[2], rootA, rootB);
    const float start = mKnots[knotIndex].GetTime();
    const float end = mKnots[knotIndex + 1].GetTime();
    if (found) {
      rootA += start;
      if (CMath::IsEpsilon(rootA, start, 0.002f)) {
        rootA = start;
      }
      if (CMath::IsEpsilon(rootA, end, 0.002f)) {
        rootA = end;
      }
      rootB += start;
      if (CMath::IsEpsilon(rootB, start, 0.002f)) {
        rootB = start;
      }
      if (CMath::IsEpsilon(rootB, end, 0.002f)) {
        rootB = end;
      }
      if (rootA >= start && rootA <= end) {
        extrema.push_back(Point(rootA, EvaluateAt(rootA)));
      }
      if (rootB >= start && rootB <= end) {
        extrema.push_back(Point(rootB, EvaluateAt(rootB)));
      }
    }
    rstl::sort(extrema.begin(), extrema.end());
  }
}

float CMayaSpline::FindFirstIntersection(float amplitude) {
  if (mKnots.size() < 2) {
    return -1.f;
  }
  rstl::reserved_vector< float, 3 > intersections;
  for (int i = 0; i < mKnots.size() - 1; ++i) {
    intersections.clear();
    FindSegmentIntersections(amplitude, i, intersections);
    if (!intersections.empty()) {
      return intersections.front();
    }
  }
  return -1.f;
}

float CMayaSpline::FindLastIntersection(float amplitude) {
  if (mKnots.size() < 2) {
    return -1.f;
  }
  rstl::reserved_vector< float, 3 > intersections;
  for (int i = mKnots.size() - 2; i >= 0; --i) {
    intersections.clear();
    FindSegmentIntersections(amplitude, i, intersections);
    if (!intersections.empty()) {
      return intersections.back();
    }
  }
  return -1.f;
}

rstl::pair< float, float > CMayaSpline::FindMaximumAmplitude() {
  rstl::pair< float, float > result(0.f, 0.f);
  if (mKnots.size() < 2) {
    return result;
  }
  rstl::reserved_vector< rstl::pair< float, float >, 2 > extrema;
  for (int i = 0; i < mKnots.size() - 1; ++i) {
    extrema.clear();
    FindSegmentExtrema(i, extrema);
    if (!extrema.empty()) {
      for (int j = 0; j < extrema.size(); ++j) {
        float amplitude = CMath::AbsF(extrema[j].second);
        if (result.second < amplitude) {
          result.first = extrema[j].first;
          result.second = amplitude;
        }
      }
    }
  }
  return result;
}
