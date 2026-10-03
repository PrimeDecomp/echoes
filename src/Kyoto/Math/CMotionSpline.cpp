#include "Kyoto/Math/CMotionSpline.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

CMotionSpline::CMotionSpline(bool closedLoop, float duration, ESplineType type)
: mLength(0.f), mDuration(duration), mClosedLoop(closedLoop), mType(type) {}

CMotionSpline::~CMotionSpline() {}

void CMotionSpline::Initialise(const rstl::vector< CVector3f >& points) {
  ResetControlPoints(points.size());
  const int count = points.size();
  if (count == 2) {
    Reset(count);
    for (int i = 0; i < count; ++i) {
      AddKnotAndControlPoint(points[i]);
    }
  } else {
    switch (mType) {
    case kST_CatmullRom:
    case kST_Linear:
    case kST_RoundedCatmullRom:
      Reset(count);
      for (int i = 0; i < count; ++i) {
        AddKnotAndControlPoint(points[i]);
      }
      break;
    case kST_BSpline:
    case kST_Bezier:
      for (int i = 0; i < count; ++i) {
        AddControlPoint(points[i]);
      }
      ResetKnots(count);
      for (int i = 0; i < count; ++i) {
        const CVector3f point = GetPositionInSegment(i, 0.f);
        AddKnot(point);
      }
      break;
    }
  }
  CalculateLength();
}

float CMotionSpline::GetKnotLength(uint index) const {
  return mKnotDistances[ValidateKnotIndex(index)];
}

float CMotionSpline::GetKnotTime(uint index) const {
  return mDuration * (GetKnotLength(index) / mLength);
}

uint CMotionSpline::ValidateKnotIndex(uint index) const {
  if (mClosedLoop) {
    index %= GetKnotCount();
  } else {
    index = CMath::Clamp(0u, index, GetKnotCount() - 1);
  }
  return index;
}

uint CMotionSpline::ValidateControlPointIndex(uint index) const {
  if (mClosedLoop) {
    index %= GetControlPointCount();
  } else {
    index = CMath::Clamp(0u, index, GetControlPointCount() - 1);
  }
  return index;
}

int CMotionSpline::GetKnotIndexByLength(float distance) const {
  int result = 0;
  if (mKnotDistances.size() > 0) {
    int low = 0;
    int high = mKnotDistances.size() - 1;
    do {
      const int middle = (low + high) >> 1;
      const float knotDistance = mKnotDistances[middle];
      if (close_enough(distance, knotDistance, 0.001f)) {
        return middle;
      }
      if (distance < knotDistance) {
        high = middle - 1;
      } else if (distance > knotDistance) {
        low = middle + 1;
      } else {
        result = middle - 1;
        break;
      }
    } while (low <= high);
    if (low > high) {
      result = low - 1;
    }
    if (distance <= 0.f) {
      result = 0;
    }
    if (distance >= mLength) {
      result = mClosedLoop ? 0 : mKnotDistances.size() - 1;
    }
  }
  return result;
}

uint CMotionSpline::GetKnotIndexByTime(float time) const {
  float distance = 0.f;
  if (!close_enough(mDuration, 0.f)) {
    distance = mLength * CMath::Limit(time / mDuration, 1.f);
  }
  return GetKnotIndexByLength(distance);
}

CVector3f CMotionSpline::GetControlPoint(uint index) const {
  if (GetControlPointCount() == 0) {
    return CVector3f::Zero();
  }
  return mControlPoints[ValidateControlPointIndex(index)];
}

void CMotionSpline::SetControlPoint(uint index, CVector3f point, bool recalculateLength) {
  if (GetControlPointCount() != 0) {
    mControlPoints[ValidateControlPointIndex(index)] = point;
    if (recalculateLength) {
      CalculateLength();
    }
  }
}

CVector3f CMotionSpline::GetKnot(uint index) const {
  if (GetKnotCount() == 0) {
    return CVector3f::Zero();
  }
  return mKnots[ValidateKnotIndex(index)];
}

void CMotionSpline::SetKnot(uint index, CVector3f point, bool recalculateLength) {
  if (GetKnotCount() != 0) {
    mKnots[ValidateKnotIndex(index)] = point;
    if (recalculateLength) {
      CalculateLength();
    }
  }
}

void CMotionSpline::AddKnot(const CVector3f& point) { mKnots.push_back_unsafe(point); }

void CMotionSpline::AddControlPoint(const CVector3f& point) {
  mControlPoints.push_back_unsafe(point);
}

void CMotionSpline::ResetKnots(uint count) {
  mKnots.clear();
  mKnotDistances.clear();
  if (count != 0) {
    mKnots.reserve(count);
    mKnotDistances.reserve(count);
  }
}

void CMotionSpline::ResetControlPoints(uint count) {
  mControlPoints.clear();
  if (count != 0) {
    mControlPoints.reserve(count);
  }
}

void CMotionSpline::Reset(uint count) {
  ResetKnots(count);
  ResetControlPoints(count);
}

void CMotionSpline::AddKnotAndControlPoint(const CVector3f& point) {
  AddKnot(point);
  AddControlPoint(point);
}

void CMotionSpline::SetKnotAndControlPoint(uint index, const CVector3f& point,
                                           bool recalculateLength) {
  SetKnot(index, point, false);
  SetControlPoint(index, point, recalculateLength);
}

void CMotionSpline::GetSurroundingPoints(int index,
                                         rstl::reserved_vector< CVector3f, 4 >& points) const {
  int first = index - 1;
  if (mType == kST_Bezier) {
    first = index;
  }
  const int count = mControlPoints.size();
  if (count == 0) {
    return;
  }
  if (count == 1) {
    for (int i = 0; i < 4; ++i) {
      points.push_back(mControlPoints[0]);
    }
    return;
  }
  if (count == 2) {
    points.push_back(mControlPoints[0]);
    points.push_back(mControlPoints[0]);
    points.push_back(mControlPoints[1]);
    points.push_back(mControlPoints[1]);
    return;
  }
  for (int i = first; i < first + 4; ++i) {
    if (i < 0) {
      if (mType == kST_Bezier) {
        points.push_back(mControlPoints[0]);
      } else if (mClosedLoop) {
        points.push_back(mControlPoints[count + i]);
      } else {
        points.push_back(mControlPoints[0] - (mControlPoints[1] - mControlPoints[0]));
      }
    } else if (i < count) {
      points.push_back(mControlPoints[i]);
    } else if (mClosedLoop) {
      points.push_back(mControlPoints[mType == kST_Bezier ? 0 : i - count]);
    } else if (mType == kST_Bezier) {
      points.push_back(mControlPoints[count - 1]);
    } else {
      points.push_back(mControlPoints[count - 1] -
                       (mControlPoints[count - 2] - mControlPoints[count - 1]));
    }
  }
}

float CMotionSpline::CalculateCatmullRomLength(int index) const {
  // Guessed names; native five-point Gauss-Legendre quadrature tables.
  static const float kNodes[5] = {0.5f, 0.769234657f, 0.230765343f, 0.953089952f, 0.0469100773f};
  static const float kWeights[5] = {0.568888902f, 0.478628665f, 0.478628665f, 0.236926883f,
                                    0.236926883f};
  rstl::reserved_vector< CVector3f, 4 > points;
  GetSurroundingPoints(index, points);
  const CVector3f cubic = 3.f * points[1] - points[0] - 3.f * points[2] + points[3];
  const CVector3f quadratic = 2.f * points[0] - 5.f * points[1] + 4.f * points[2] - points[3];
  const CVector3f linear = 0.5f * (points[2] - points[0]);
  float length = 0.f;
  for (uint i = 0; i < 5; ++i) {
    const float t = kNodes[i];
    const CVector3f tangent = linear + t * (quadratic + (1.5f * t) * cubic);
    length += kWeights[i] * tangent.Magnitude();
  }
  return length * 0.5f;
}

float CMotionSpline::CalculateRoundedCatmullRomLength(int index) const {
  // Guessed names; each native integrator owns its own tables.
  static const float kNodes[5] = {0.5f, 0.769234657f, 0.230765343f, 0.953089952f, 0.0469100773f};
  static const float kWeights[5] = {0.568888902f, 0.478628665f, 0.478628665f, 0.236926883f,
                                    0.236926883f};
  rstl::reserved_vector< CVector3f, 4 > points;
  GetSurroundingPoints(index, points);
  const CVector3f span = points[2] - points[1];
  if (!span.CanBeNormalized()) {
    return 0.f;
  }

  CVector3f previous = points[0] - points[1];
  if (!previous.CanBeNormalized()) {
    previous = CVector3f(0.f, 1.f, 0.f);
  }
  CVector3f firstTangent = span.AsNormalized() - previous.AsNormalized();
  if (firstTangent.CanBeNormalized()) {
    firstTangent.Normalize();
  } else {
    firstTangent = CVector3f(0.f, 1.f, 0.f);
  }
  CVector3f next = points[3] - points[2];
  if (!next.CanBeNormalized()) {
    next = CVector3f(0.f, 1.f, 0.f);
  }
  const CVector3f reverseSpan = -span;
  CVector3f secondTangent = next.AsNormalized() - reverseSpan.AsNormalized();
  if (secondTangent.CanBeNormalized()) {
    secondTangent.Normalize();
  } else {
    secondTangent = CVector3f(0.f, 1.f, 0.f);
  }
  const float spanLength = span.Magnitude();
  const CVector3f firstDerivative = spanLength * firstTangent;
  const CVector3f secondDerivative = spanLength * secondTangent;
  const CVector3f cubic = 2.f * points[1] - 2.f * points[2] + firstDerivative + secondDerivative;
  const CVector3f quadratic =
      2.f * (-3.f * points[1] + 3.f * points[2] - 2.f * firstDerivative - secondDerivative);
  float length = 0.f;
  for (uint i = 0; i < 5; ++i) {
    const float t = kNodes[i];
    const CVector3f tangent = firstDerivative + t * (quadratic + (3.f * t) * cubic);
    length += kWeights[i] * tangent.Magnitude();
  }
  return length * 0.5f;
}

float CMotionSpline::CalculateBSplineLength(int index) const {
  // Guessed names; native five-point Gauss-Legendre quadrature tables.
  static const float kNodes[5] = {0.5f, 0.769234657f, 0.230765343f, 0.953089952f, 0.0469100773f};
  static const float kWeights[5] = {0.568888902f, 0.478628665f, 0.478628665f, 0.236926883f,
                                    0.236926883f};
  rstl::reserved_vector< CVector3f, 4 > points;
  GetSurroundingPoints(index, points);
  const CVector3f cubic = points[3] - 3.f * points[2] + 3.f * points[1] - points[0];
  const CVector3f quadratic = 2.f * (3.f * points[2] - 6.f * points[1] + 3.f * points[0]);
  const CVector3f linear = 3.f * points[2] - 3.f * points[0];
  float length = 0.f;
  for (uint i = 0; i < 5; ++i) {
    const float t = kNodes[i];
    const CVector3f tangent = (1.f / 6.f) * (linear + t * (quadratic + (3.f * t) * cubic));
    length += kWeights[i] * tangent.Magnitude();
  }
  return length * 0.5f;
}

float CMotionSpline::CalculateBezierLength(const CVector3f& a, const CVector3f& b,
                                           const CVector3f& c, const CVector3f& d) const {
  const float chord = (a - d).Magnitude();
  const float polygon = (a - b).Magnitude() + (b - c).Magnitude() + (c - d).Magnitude();
  const float difference = chord - polygon;
  if (difference * difference < 0.1f) {
    return 0.5f * (chord + polygon);
  }
  const CVector3f ab = 0.5f * (a + b);
  const CVector3f bc = 0.5f * (b + c);
  const CVector3f cd = 0.5f * (c + d);
  const CVector3f abc = 0.5f * (ab + bc);
  const CVector3f bcd = 0.5f * (bc + cd);
  const CVector3f middle = 0.5f * (abc + bcd);
  return CalculateBezierLength(a, ab, abc, middle) + CalculateBezierLength(middle, bcd, cd, d);
}

float CMotionSpline::CalculateBezierLength(int index) const {
  rstl::reserved_vector< CVector3f, 4 > points;
  GetSurroundingPoints(index, points);
  return CalculateBezierLength(points[0], points[1], points[2], points[3]);
}

void CMotionSpline::CalculateLength() {
  mKnotDistances.clear();
  float length = 0.f;
  mKnotDistances.reserve(mControlPoints.size());
  const int count = mControlPoints.size();
  if (count < 2) {
    if (count == 1) {
      mKnotDistances.push_back_unsafe(0.f);
    }
  } else if (count == 2) {
    mKnotDistances.push_back_unsafe(length);
    length += (mControlPoints[1] - mControlPoints[0]).Magnitude();
    mKnotDistances.push_back_unsafe(length);
    if (mClosedLoop) {
      length *= 2.f;
    }
  } else {
    switch (mType) {
    case kST_CatmullRom:
      for (int i = 0; i < mControlPoints.size() - 1; ++i) {
        mKnotDistances.push_back_unsafe(length);
        length += CalculateCatmullRomLength(i);
      }
      mKnotDistances.push_back_unsafe(length);
      if (mClosedLoop) {
        length += CalculateCatmullRomLength(mControlPoints.size() - 1);
      }
      break;
    case kST_BSpline:
      for (int i = 0; i < mControlPoints.size() - 1; ++i) {
        mKnotDistances.push_back_unsafe(length);
        length += CalculateBSplineLength(i);
      }
      mKnotDistances.push_back_unsafe(length);
      if (mClosedLoop) {
        length += CalculateBSplineLength(mControlPoints.size() - 1);
      }
      break;
    case kST_Linear:
      for (int i = 0; i < mControlPoints.size() - 1; ++i) {
        mKnotDistances.push_back_unsafe(length);
        length += (mControlPoints[i + 1] - mControlPoints[i]).Magnitude();
      }
      mKnotDistances.push_back_unsafe(length);
      if (mClosedLoop) {
        length += (mControlPoints[0] - mControlPoints[mControlPoints.size() - 1]).Magnitude();
      }
      break;
    case kST_Bezier: {
      const uint segments = count / 4;
      if (count > 3) {
        for (uint i = 0; i < segments; ++i) {
          mKnotDistances.push_back_unsafe(length);
          mKnotDistances.push_back_unsafe(length);
          mKnotDistances.push_back_unsafe(length);
          length += CalculateBezierLength(i * 3);
        }
        mKnotDistances.push_back_unsafe(length);
      }
      const uint remainder = mControlPoints.size() - segments * 4;
      if (remainder != 0) {
        for (uint i = 0; i < remainder - 1; ++i) {
          mKnotDistances.push_back_unsafe(length);
        }
        length += CalculateBezierLength(segments * 3);
        mKnotDistances.push_back_unsafe(length);
      }
      break;
    }
    case kST_RoundedCatmullRom:
      for (int i = 0; i < mControlPoints.size() - 1; ++i) {
        mKnotDistances.push_back_unsafe(length);
        length += CalculateRoundedCatmullRomLength(i);
      }
      mKnotDistances.push_back_unsafe(length);
      if (mClosedLoop) {
        length += CalculateRoundedCatmullRomLength(mControlPoints.size() - 1);
      }
      break;
    }
  }
  mLength = length;
}

float CMotionSpline::ValidateLength(float distance) const {
  if (close_enough(distance, 0.f, 0.001f)) {
    return 0.f;
  }
  float result = distance;
  if (mClosedLoop) {
    if (close_enough(distance, mLength, 0.001f)) {
      return 0.f;
    }
    if (distance >= mLength) {
      while (result >= mLength) {
        result -= mLength;
      }
    }
    if (distance < 0.f) {
      while (result < 0.f) {
        result += mLength;
      }
    }
  } else {
    result = CMath::Clamp(0.f, result, mLength);
  }
  return result;
}

CVector3f CMotionSpline::GetPositionByTime(float time) const {
  float distance = 0.f;
  if (!close_enough(mDuration, 0.f)) {
    distance = mLength * CMath::Limit(time / mDuration, 1.f);
  }
  return GetPositionByLength(distance);
}

CVector3f CMotionSpline::GetPositionInSegment(uint index, float t) const {
  if (mControlPoints.empty()) {
    return CVector3f::Zero();
  }
  uint segment = index;
  if (segment > GetControlPointCount() - 1) {
    segment = GetControlPointCount() - 1;
  }
  t = CMath::Clamp(0.f, t, 1.f);
  if (mControlPoints.size() == 1) {
    return mControlPoints[0];
  }
  if (mControlPoints.size() == 2) {
    return mControlPoints[index] + t * (mControlPoints[(index + 1) % 2] - mControlPoints[index]);
  }
  if (mType == kST_Bezier) {
    segment = (segment / 3) * 3;
  }
  rstl::reserved_vector< CVector3f, 4 > points;
  GetSurroundingPoints(segment, points);
  switch (mType) {
  case kST_CatmullRom:
    return CMath::GetCatmullRomSplinePoint(points[0], points[1], points[2], points[3], t);
  case kST_BSpline:
    return CMath::GetBSplinePoint(points[0], points[1], points[2], points[3], t);
  case kST_Linear:
    return points[1] + t * (points[2] - points[1]);
  case kST_Bezier:
    return CMath::GetBezierPoint(points[0], points[1], points[2], points[3], t);
  case kST_RoundedCatmullRom:
    return CMath::GetRoundedCatmullRomSplinePoint(points[0], points[1], points[2], points[3], t);
  }
  return CVector3f::Zero();
}

float CMotionSpline::GetSegmentParameter(float distance, uint index) const {
  rstl::reserved_vector< CVector3f, 4 > points;
  GetSurroundingPoints(index, points);
  CVector3f previous = CVector3f::Zero();
  switch (mType) {
  case kST_CatmullRom:
    previous = CMath::GetCatmullRomSplinePoint(points[0], points[1], points[2], points[3], 0.f);
    break;
  case kST_BSpline:
    previous = CMath::GetBSplinePoint(points[0], points[1], points[2], points[3], 0.f);
    break;
  case kST_Linear:
    previous = points[1];
    break;
  case kST_Bezier:
    previous = CMath::GetBezierPoint(points[0], points[1], points[2], points[3], 0.f);
    break;
  case kST_RoundedCatmullRom:
    previous =
        CMath::GetRoundedCatmullRomSplinePoint(points[0], points[1], points[2], points[3], 0.f);
    break;
  }
  float length = GetKnotLength(index);
  float t = 0.1f;
  CVector3f next = CVector3f::Zero();
  for (int i = 0; i < 10; ++i) {
    switch (mType) {
    case kST_CatmullRom:
      next = CMath::GetCatmullRomSplinePoint(points[0], points[1], points[2], points[3], t);
      break;
    case kST_BSpline:
      next = CMath::GetBSplinePoint(points[0], points[1], points[2], points[3], t);
      break;
    case kST_Linear:
      next = points[1] + t * (points[2] - points[1]);
      break;
    case kST_Bezier:
      next = CMath::GetBezierPoint(points[0], points[1], points[2], points[3], t);
      break;
    case kST_RoundedCatmullRom:
      next = CMath::GetRoundedCatmullRomSplinePoint(points[0], points[1], points[2], points[3], t);
      break;
    }
    const float stepLength = (next - previous).Magnitude();
    const float nextLength = length + stepLength;
    if (distance <= nextLength || close_enough(nextLength, distance, 0.001f)) {
      if (close_enough(stepLength, 0.f)) {
        return t - 0.1f;
      }
      return (t - 0.1f) + 0.1f * CMath::Clamp(0.f, (distance - length) / stepLength, 1.f);
    }
    t = CMath::Limit(0.1f + t, 1.f);
    length = nextLength;
    previous = next;
  }
  if (index == 0 && distance >= mLength && mType != kST_Bezier) {
    return 0.f;
  }
  return 1.f;
}

CVector3f CMotionSpline::GetPositionByLength(float distance) const {
  const int count = mKnots.size();
  if (close_enough(distance, 0.f, 0.001f)) {
    distance = 0.f;
  }
  if (close_enough(distance, mLength, 0.001f)) {
    distance = mLength;
  }
  if (count > 0) {
    if (count == 1) {
      return mKnots[0];
    }
    int index = GetKnotIndexByLength(distance);
    if (count == 2) {
      CVector3f first = mControlPoints[0];
      CVector3f second = mControlPoints[1];
      const float halfLength = 0.5f * mLength;
      if (mClosedLoop && distance > mKnotDistances[1]) {
        first = mControlPoints[1];
        second = mControlPoints[0];
      }
      float t;
      if (mClosedLoop) {
        t = distance > halfLength ? CMath::Clamp(0.f, (distance - halfLength) / halfLength, 1.f)
                                  : CMath::Clamp(0.f, distance / halfLength, 1.f);
      } else {
        t = CMath::Clamp(0.f, distance / mLength, 1.f);
      }
      return first + t * (second - first);
    }
    if (mType == kST_Bezier) {
      index = (index / 3) * 3;
    }
    rstl::reserved_vector< CVector3f, 4 > points;
    GetSurroundingPoints(index, points);
    const float t = GetSegmentParameter(distance, index);
    switch (mType) {
    case kST_CatmullRom:
      return CMath::GetCatmullRomSplinePoint(points[0], points[1], points[2], points[3], t);
    case kST_BSpline:
      return CMath::GetBSplinePoint(points[0], points[1], points[2], points[3], t);
    case kST_Linear:
      return points[1] + t * (points[2] - points[1]);
    case kST_Bezier:
      return CMath::GetBezierPoint(points[0], points[1], points[2], points[3], t);
    case kST_RoundedCatmullRom:
      return CMath::GetRoundedCatmullRomSplinePoint(points[0], points[1], points[2], points[3], t);
    }
  }
  return CVector3f::Zero();
}

float CMotionSpline::FindClosestLengthOnSpline(float start, const CVector3f& position) const {
  if (mType == kST_Bezier) {
    return FindClosestBezierLength(start, position);
  }
  if (mKnots.size() < 2) {
    return 0.f;
  }
  if (mKnots.size() == 2) {
    CVector3f direction = mKnots[1] - mKnots[0];
    if (direction.IsMagnitudeSafe()) {
      direction.Normalize();
      return CVector3f::Dot(direction, position - mKnots[0]);
    }
    return 0.f;
  }

  float result = -1.f;
  float nearestDistance = 10000.f;
  float nearestLengthDelta = 10000.f;
  int iterations = mKnots.size() - 1;
  if (mClosedLoop) {
    ++iterations;
  }
  for (int i = 0; i < iterations; ++i) {
    const CVector3f first = mKnots[i];
    const CVector3f second = mClosedLoop && i == mKnots.size() - 1 ? mKnots[0] : mKnots[i + 1];
    const CVector3f delta = second - first;
    const CVector3f reverseDelta = first - second;
    CVector3f previous;
    if (i == 0) {
      previous = mKnots[0] + (mKnots[0] - mKnots[1]);
      if (mClosedLoop) {
        previous = mKnots[mKnots.size() - 1];
      }
    } else {
      previous = mKnots[i - 1];
    }
    CVector3f forwardDirection = (first - previous) + delta;
    forwardDirection.Normalize();
    CVector3f next;
    if (i < mKnots.size() - 2) {
      next = mKnots[i + 2];
    } else if (mClosedLoop) {
      next = i == iterations - 1 ? mKnots[1] : mKnots[0];
    } else {
      next = mKnots[i + 1] + (mKnots[i + 1] - mKnots[i]);
    }
    CVector3f backwardDirection = (second - next) + reverseDelta;
    backwardDirection.Normalize();
    const float firstProjection = CVector3f::Dot(position - first, forwardDirection);
    const float firstDot =
        CMath::Limit(CVector3f::Dot(forwardDirection, delta.AsNormalized()), 1.f);
    const float secondProjection = CVector3f::Dot(position - second, backwardDirection);
    const float secondDot =
        CMath::Limit(CVector3f::Dot(backwardDirection, reverseDelta.AsNormalized()), 1.f);
    float t = (firstProjection / firstDot) /
              CMath::AbsF(firstProjection / firstDot + secondProjection / secondDot);
    if (!mClosedLoop) {
      if (i == 0 && t < 0.f) {
        t = 0.f;
      }
      if (i == mKnots.size() - 2 && t > 1.f) {
        t = 1.f;
      }
    }
    if (close_enough(t, 0.f, 0.001f)) {
      t = 0.f;
    }
    if (close_enough(t, 1.f, 0.001f)) {
      t = 1.f;
    }
    if (t >= 0.f && t <= 1.f) {
      const float span = i == mKnots.size() - 1 ? mLength - mKnotDistances[i]
                                                : mKnotDistances[i + 1] - mKnotDistances[i];
      const float length = t * span + mKnotDistances[i];
      const CVector3f offset = position - GetPositionByLength(length);
      float distance = 0.f;
      if (offset.IsMagnitudeSafe()) {
        distance = offset.Magnitude();
        if (distance < 0.f) {
          distance = 0.f;
        }
      }
      float lengthDelta = CMath::AbsF(length - start);
      if (mClosedLoop) {
        const float wrappedDelta = mLength - lengthDelta;
        if (lengthDelta > wrappedDelta) {
          lengthDelta = wrappedDelta;
        }
      }
      if (close_enough(CMath::AbsF(distance - nearestDistance), 0.f, 0.001f)) {
        if (lengthDelta < nearestLengthDelta) {
          result = length;
          nearestLengthDelta = lengthDelta;
        }
      } else if (distance < nearestDistance) {
        result = length;
        nearestDistance = distance;
        nearestLengthDelta = lengthDelta;
      }
    }
  }
  if (result < 0.f) {
    result = 0.f;
  }
  return result;
}

float CMotionSpline::FindClosestBezierLength(float start, const CVector3f& position) const {
  if (mKnots.size() < 2) {
    return 0.f;
  }
  if (mKnots.size() == 2) {
    CVector3f direction = mKnots[1] - mKnots[0];
    if (direction.IsMagnitudeSafe()) {
      direction.Normalize();
      return CVector3f::Dot(direction, position - mKnots[0]);
    }
    return 0.f;
  }

  float result = -1.f;
  float nearestDistance = 10000.f;
  float nearestLengthDelta = 10000.f;
  rstl::vector< CVector3f > knots;
  knots.reserve(GetKnotCount() / 3 + 2);
  for (uint i = 0; i < GetKnotCount(); i += 3) {
    knots.push_back_unsafe(mKnots[i]);
  }
  if (mClosedLoop) {
    knots.push_back_unsafe(knots[0]);
  }
  const int iterations = knots.size() - 1;
  for (int i = 0; i < iterations; ++i) {
    const CVector3f first = knots[i];
    const CVector3f second = mClosedLoop && i == knots.size() - 1 ? knots[0] : knots[i + 1];
    const CVector3f delta = second - first;
    const CVector3f reverseDelta = first - second;
    CVector3f previous;
    if (i == 0) {
      previous = knots[0] + (knots[0] - knots[1]);
      if (mClosedLoop) {
        previous = knots[knots.size() - 1];
      }
    } else {
      previous = knots[i - 1];
    }
    CVector3f forwardDirection = (first - previous) + delta;
    if (forwardDirection.CanBeNormalized()) {
      forwardDirection.Normalize();
    }
    CVector3f next;
    if (i < knots.size() - 2) {
      next = knots[i + 2];
    } else if (mClosedLoop) {
      next = i == iterations - 1 ? knots[1] : knots[0];
    } else {
      next = knots[i + 1] + (knots[i + 1] - knots[i]);
    }
    CVector3f backwardDirection = (second - next) + reverseDelta;
    if (backwardDirection.CanBeNormalized()) {
      backwardDirection.Normalize();
    }
    const float firstProjection = CVector3f::Dot(position - first, forwardDirection);
    const float firstDot =
        CMath::Limit(CVector3f::Dot(forwardDirection, delta.AsNormalized()), 1.f);
    const float secondProjection = CVector3f::Dot(position - second, backwardDirection);
    const float secondDot =
        CMath::Limit(CVector3f::Dot(backwardDirection, reverseDelta.AsNormalized()), 1.f);
    float t = (firstProjection / firstDot) /
              CMath::AbsF(firstProjection / firstDot + secondProjection / secondDot);
    if (!mClosedLoop) {
      if (i == 0 && t < 0.f) {
        t = 0.f;
      }
      if (i == knots.size() - 2 && t > 1.f) {
        t = 1.f;
      }
    }
    if (close_enough(t, 0.f, 0.001f)) {
      t = 0.f;
    }
    if (close_enough(t, 1.f, 0.001f)) {
      t = 1.f;
    }
    if (t >= 0.f && t <= 1.f) {
      const float span = i == knots.size() - 1
                             ? mLength - mKnotDistances[i * 3]
                             : mKnotDistances[(i + 1) * 3] - mKnotDistances[i * 3];
      const float length = t * span + mKnotDistances[i * 3];
      const CVector3f offset = position - GetPositionByLength(length);
      float distance = 0.f;
      if (offset.IsMagnitudeSafe()) {
        distance = offset.Magnitude();
        if (distance < 0.f) {
          distance = 0.f;
        }
      }
      float lengthDelta = CMath::AbsF(length - start);
      if (mClosedLoop) {
        const float wrappedDelta = mLength - lengthDelta;
        if (lengthDelta > wrappedDelta) {
          lengthDelta = wrappedDelta;
        }
      }
      if (close_enough(CMath::AbsF(distance - nearestDistance), 0.f, 0.001f)) {
        if (lengthDelta < nearestLengthDelta) {
          result = length;
          nearestLengthDelta = lengthDelta;
        }
      } else if (distance < nearestDistance) {
        result = length;
        nearestDistance = distance;
        nearestLengthDelta = lengthDelta;
      }
    }
  }
  if (result < 0.f) {
    result = 0.f;
  }
  return result;
}

void CMotionSpline::Translate(const CVector3f& offset) {
  for (uint i = 0; i < GetKnotCount(); ++i) {
    SetKnot(i, GetKnot(i) + offset, false);
  }
  for (uint i = 0; i < GetControlPointCount(); ++i) {
    SetControlPoint(i, GetControlPoint(i) + offset, false);
  }
}

void CMotionSpline::Rotate(const CQuaternion& rotation, const CVector3f& origin) {
  for (uint i = 0; i < GetKnotCount(); ++i) {
    SetKnot(i, rotation.Transform(GetKnot(i) - origin) + origin, false);
  }
  for (uint i = 0; i < GetControlPointCount(); ++i) {
    SetControlPoint(i, rotation.Transform(GetControlPoint(i) - origin) + origin, false);
  }
}

CVector3f CMotionSpline::GetTangentByTime(float time) const {
  float distance = 0.f;
  if (!close_enough(mDuration, 0.f)) {
    distance = mLength * CMath::Clamp(0.f, time / mDuration, 1.f);
  }
  return GetTangentByLength(distance);
}

CVector3f CMotionSpline::GetTangentByLength(float distance) const {
  CVector3f tangent = CVector3f::Forward();
  int index = GetKnotIndexByLength(distance);
  if (mType == kST_Bezier) {
    index = (index / 3) * 3;
  }
  if (mControlPoints.size() == 2) {
    tangent = mControlPoints[1] - mControlPoints[0];
    if (mClosedLoop && distance > 0.5f * mLength) {
      tangent = mControlPoints[0] - mControlPoints[1];
    }
  } else {
    rstl::reserved_vector< CVector3f, 4 > points;
    GetSurroundingPoints(index, points);
    const float t = GetSegmentParameter(distance, index);
    switch (mType) {
    case kST_CatmullRom:
      tangent = CMath::GetCatmullRomSplineTangent(points[0], points[1], points[2], points[3], t);
      break;
    case kST_BSpline:
      tangent = CMath::GetBSplineTangent(points[0], points[1], points[2], points[3], t);
      break;
    case kST_Linear:
      tangent = points[2] - points[1];
      break;
    case kST_Bezier:
      return CMath::GetBezierTangent(points[0], points[1], points[2], points[3], t);
    case kST_RoundedCatmullRom:
      tangent =
          CMath::GetRoundedCatmullRomSplineTangent(points[0], points[1], points[2], points[3], t);
      break;
    }
  }
  if (tangent.CanBeNormalized()) {
    tangent.Normalize();
  } else {
    tangent = CVector3f::Forward();
  }
  return tangent;
}
