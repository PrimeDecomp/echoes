#include "Kyoto/Math/CMayaSpline.hpp"

#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Streams/CInputStream.hpp"


#include "float.h"
#include "math.h"

void ValidateTangent(CVector2f& tangent) {
  if (tangent.GetX() < 0.f) {
    tangent.SetX(0.f);
  }

  const float mag = tangent.Magnitude();
  if (mag != 0.f) {
    tangent /= mag;
  }

  if (tangent.GetX() == 0.f && tangent.GetY() != 0.f) {
    const float mul = tangent.GetY() >= 0.f ? 1.f : -1.f;
    tangent.SetX(0.0001f);
    tangent.SetY(5729578.0f * tangent.GetX() * mul);
  }
}

CMayaSplineKnot::CMayaSplineKnot(CInputStream& in)
: xc_cachedTangentA(0.0f, 0.0f), x14_cachedTangentB(0.0f, 0.0f) {
  x0_time = in.ReadFloat();
  x4_amplitude = in.ReadFloat();
  x8_ = in.ReadChar();
  x9_ = in.ReadChar();
  if (x8_ == 5) {
    float x = in.ReadFloat();
    float y = in.ReadFloat();
    xc_cachedTangentA = CVector2f(x, y);
  }

  if (x9_ == 5) {
    float x = in.ReadFloat();
    float y = in.ReadFloat();
    x14_cachedTangentB = CVector2f(x, y);
  }
}

void CMayaSplineKnot::GetTangents(CMayaSplineKnot* prev, CMayaSplineKnot* next, CVector2f& tangentA,
                                  CVector2f& tangentB) {
  if (xa_24_dirty) {
    CalculateTangents(prev, next);
  }

  tangentA = xc_cachedTangentA;
  tangentB = x14_cachedTangentB;
}

void CMayaSplineKnot::CalculateTangents(CMayaSplineKnot* prev, CMayaSplineKnot* next) {
  xa_24_dirty = false;
  bool calculateTangents = false;
  if (x8_ == 4 && prev != nullptr) {
    float fVar2 = abs(prev->GetAmplitude() - GetAmplitude());
    float fVar3 = fVar2;
    if (next != nullptr) {
      fVar3 = abs(next->GetAmplitude() - GetAmplitude());
    }
    if (fVar3 <= 0.05f || fVar2 <= 0.05f) {
      x8_ = 1;
    }
  }

  if (x8_ == 0) {
    if (prev == nullptr) {
      xc_cachedTangentA = CVector2f(1.f, 0.f);
    } else {
      xc_cachedTangentA =
          CVector2f(GetTime() - prev->GetTime(), GetAmplitude() - prev->GetAmplitude());
    }
  } else if (x8_ == 1) {
    float fVar1 = 0.f;
    if (prev != nullptr) {
      fVar1 = GetTime() - prev->GetTime();
    } else if (next != nullptr) {
      fVar1 = next->GetTime() - GetTime();
    }
    xc_cachedTangentA = CVector2f(fVar1, 0.f);
  } else if (x8_ == 2) {
    calculateTangents = true;
  } else if (x8_ == 3) {
    xc_cachedTangentA = CVector2f(1.0f, 1.0f);
  } else if (x8_ == 4) {
    x8_ = 2;
    calculateTangents = true;
  }

  if (x9_ == 0) {
    if (next == nullptr) {
      x14_cachedTangentB = CVector2f(1.0f, 0.0f);
    } else {
      x14_cachedTangentB =
          CVector2f(next->GetTime() - GetTime(), next->GetAmplitude() - GetAmplitude());
    }
  } else if (x9_ == 1) {
    float fVar1 = 0.f;
    if (next != nullptr) {
      fVar1 = next->GetTime() - GetTime();
    } else if (prev != nullptr) {
      fVar1 = GetTime() - prev->GetTime();
    }

    x14_cachedTangentB = CVector2f(fVar1, 0.f);
  } else if (x9_ == 2) {
    calculateTangents = true;
  } else if (x9_ == 3) {
    x14_cachedTangentB = CVector2f(1.f, 0.f);
  } else if (x9_ == 4 && next != nullptr) {
    float fVar1 = next->GetAmplitude() - GetAmplitude();
    float fVar2 = fVar1;
    if (prev != nullptr) {
      fVar2 = prev->GetAmplitude() - GetAmplitude();
    }

    if (fVar1 <= 0.05f || fVar2 <= 0.05f) {
      x9_ = 1;
    }
    calculateTangents = true;
  }

  if (calculateTangents) {
    CVector2f tangentA(0.0f, 0.0f);
    CVector2f tangentB(0.0f, 0.0f);

    if (prev == nullptr && next != nullptr) {
      tangentA = tangentB =
          CVector2f(next->GetTime() - GetTime(), next->GetAmplitude() - GetAmplitude());
    } else if (prev != nullptr && next == nullptr) {
      tangentA = tangentB =
          CVector2f(GetTime() - prev->GetTime(), GetAmplitude() - prev->GetAmplitude());
    } else if (prev != nullptr && next != nullptr) {
      float timeDiff = next->GetTime() - prev->GetTime();
      float ampDiff = next->GetAmplitude() - prev->GetAmplitude();
      float amp =
          timeDiff >= 0.0001f ? ampDiff / timeDiff : (ampDiff <= 0.f ? -5729578.0f : 5729578.0f);
      float nextTimeDiff = next->GetTime() - GetTime();
      float prevTimeDiff = GetTime() - prev->GetTime();
      float ampA = 0.f;
      float ampB = 0.;
      float timeA = 0.;
      float timeB = 0.;
      if (nextTimeDiff >= 0.f) {
        ampA = nextTimeDiff * amp;
      } else {
        timeA = 0.f;
        ampA = amp;
      }

      if (prevTimeDiff >= 0.f) {
        ampB = prevTimeDiff * amp;
      } else {
        timeB = 0.f;
      }
      tangentB = CVector2f(timeB, ampB);
      tangentA = CVector2f(timeA, ampA);

    } else {
      // TODO
      // tangentA.zeroOut();
      // tangentB.zeroOut();
    }

    if (x8_ == 2) {
      xc_cachedTangentA = tangentA;
    }
    if (x9_ == 2) {
      x14_cachedTangentB = tangentB;
    }
  }
  ValidateTangent(xc_cachedTangentA);
  ValidateTangent(x14_cachedTangentB);
}

// CMayaSpline::CMayaSpline(CInputStream& in, int count) : x0_preInfinity(in.ReadInt8()),
// x4_postInfinity(in.ReadInt8()) {

//   u32 knotCount = in.ReadLong();
//   x8_knots.reserve(knotCount);
//   for (size_t i = 0; i < knotCount; ++i) {
//     x8_knots.emplace_back(in);
//   }
//   x18_clampMode = in.ReadInt8();
//   x1c_minAmplitudeTime = in.ReadFloat();
//   x20_maxAmplitudeTime = in.ReadFloat();
// }

size_t CMayaSpline::GetKnotCount() const { return x8_knots.size(); }

const rstl::vector< CMayaSplineKnot >& CMayaSpline::GetKnots() const { return x8_knots; }

float CMayaSpline::GetMinTime() const { return x8_knots.empty() ? 0.f : x8_knots[0].GetTime(); }
float CMayaSpline::GetMaxTime() const {
  return x8_knots.empty() ? 0.f : x8_knots[GetKnotCount() - 1].GetTime();
}
float CMayaSpline::GetDuration() const {
  return x8_knots.empty() ? 0.f : GetMaxTime() - GetMinTime();
}

float CMayaSpline::EvaluateAt(float time) {
  float amplitude = EvaluateAtUnclamped(time);
  if (x18_clampMode == 1) {
    if (x1c_minAmplitudeTime > amplitude) {
      return x1c_minAmplitudeTime;
    }
    if (x20_maxAmplitudeTime < amplitude) {
      return x20_maxAmplitudeTime;
    }
    return amplitude;
  } else if (x18_clampMode == 2) {
    float center = x20_maxAmplitudeTime - x1c_minAmplitudeTime;

    if (center > 0.f) {
      if (amplitude <= FLT_EPSILON + x20_maxAmplitudeTime) {
        return amplitude - (center * static_cast< float >(
                                         int((amplitude - x20_maxAmplitudeTime) / center) + 1));
      }
      if (amplitude < x1c_minAmplitudeTime - FLT_EPSILON) {
        return amplitude + (center * static_cast< float >(
                                         abs(int((amplitude - x1c_minAmplitudeTime) / center))));
      }
    }
  }

  return amplitude;
}

float CMayaSpline::EvaluateAtUnclamped(float time) {
  if (x8_knots.empty()) {
    return 0.f;
  }

  u32 lastIdx = x8_knots.size() - 1;
  bool bVar2 = false;
  float retVal;
  if (time < x8_knots[0].GetTime()) {
    if (x0_preInfinity == 0) {
      return x8_knots[0].GetAmplitude();
    }
    return EvaluateInfinities(time, true);
  } else if (x8_knots[lastIdx].GetTime() >= time) {
    bVar2 = false;
    int local_68 = -1;
    int iVar1 = x24_chachedKnotIndex;
    if (iVar1 != -1) {
      if (lastIdx <= iVar1 || x8_knots[lastIdx].GetTime() >= time) {
        if (iVar1 > 0 && x8_knots[iVar1].GetTime() > time) {
          int iVar3 = iVar1 - 1;
          bVar2 = x8_knots[iVar3].GetTime() < time;
          if (bVar2) {
            local_68 = iVar1;
          }
          if (x8_knots[iVar3].GetTime() == time) {
            x24_chachedKnotIndex = iVar3;
            return x8_knots[x24_chachedKnotIndex].GetAmplitude();
          }
        }
      } else {
        retVal = x8_knots[iVar1 + 1].GetTime();
        if (retVal == time) {
          x24_chachedKnotIndex = lastIdx;
          return x8_knots[x24_chachedKnotIndex].GetAmplitude();
        }

        if (retVal > time) {
          bVar2 = true;
          local_68 = iVar1 + 1;
        }
      }
    }

    if (!bVar2 && (FindKnot(time, local_68))) {
      if (local_68 == 0) {
        x24_chachedKnotIndex = 0;
        return x8_knots[0].GetAmplitude();
      }
      if (local_68 == x8_knots.size()) {
        x24_chachedKnotIndex = 0;
        return x8_knots[lastIdx].GetAmplitude();
      }
    }

    lastIdx = local_68 - 1;
    if (x28_ != lastIdx) {
      x24_chachedKnotIndex = lastIdx;
      x28_ = lastIdx;
      if (x8_knots[x24_chachedKnotIndex].GetX9() == 3) {
        x2c_24_dirty = true;
      } else {
        x2c_24_dirty = false;
        rstl::reserved_vector< CVector2f, 4 > points;
        FindControlPoints(x24_chachedKnotIndex, points);
        CalculateHermiteCoefficients(points, x34_cachedHermitCoefs);
        x30_cachedMinTime = points[0].GetX();
      }
    }

    if (x2c_24_dirty) {
      return x8_knots[x24_chachedKnotIndex].GetTime();
    } else {
      return EvaluateHermite(time);
    }
  }

  if (x4_postInfinity == 0) {
    return x8_knots[lastIdx].GetAmplitude();
  }

  return EvaluateInfinities(time, false);
}

float CMayaSpline::EvaluateInfinities(float time, bool pre) {
  if (x8_knots.empty()) {
    return 0.f;
  }

  int lastIdx = x8_knots.size() - 1;
  CMayaSplineKnot* curKnot = &x8_knots[0];
  const float startTime = x8_knots[0].GetTime();
  const float endTime = x8_knots[lastIdx].GetAmplitude();
  float center = endTime - startTime;

  if (close_enough(center, 0)) {
    return curKnot->GetAmplitude();
  }

  double tmp = 0.f;
  float divTime = (time <= endTime) ? modf((time - startTime) / center, &tmp)
                                    : modf((time - endTime) / center, &tmp);

  center = center * abs(divTime);
  tmp = 1.f + abs(tmp);

  if (!pre) {
    if (x4_postInfinity == 4) {
      divTime = fmod(tmp, 2.f);
      if (close_enough(divTime, 0.f)) {
        center = startTime + center;
      } else {
        center = endTime - center;
      }
    } else if (x4_postInfinity == 2 || x4_postInfinity == 3) {
      center = startTime + center;
    } else if (x4_postInfinity == 1) {
      center = time - endTime;
      CVector2f tangentA(0.0f, 0.0f);
      CVector2f tangentB(0.0f, 0.0f);
      x8_knots[0].GetTangents((lastIdx < 1) ? nullptr : &x8_knots[lastIdx - 2], nullptr, tangentA,
                              tangentB);
      if (!close_enough(tangentB.GetX(), 0.f)) {
        return x8_knots[lastIdx].GetAmplitude() + (center * tangentB.GetY() / tangentB.GetX());
      }
      return x8_knots[lastIdx].GetAmplitude();
    }
  } else if (x0_preInfinity == 4) {
    divTime = fmod(tmp, 2.f);
    if (close_enough(divTime, 0.f)) {
      center = endTime - center;
    } else {
      center = startTime + center;
    }
  } else if (x0_preInfinity == 2 || x0_preInfinity == 3) {
    center = endTime - center;
  } else if (x0_preInfinity == 1) {
    center = (startTime - time);
    CVector2f tangentA(0.0f, 0.0f);
    CVector2f tangentB(0.0f, 0.0f);
    x8_knots[0].GetTangents(nullptr, &x8_knots[1], tangentA, tangentB);
    if (!close_enough(tangentA.GetX(), 0)) {
      return (x8_knots[0].GetAmplitude() - (center * tangentA.GetY() / tangentA.GetX()));
    }
    return x8_knots[0].GetAmplitude();
  }

  float eval = EvaluateAt(center);
  if (pre && x0_preInfinity == 3) {
    return eval - (tmp * x8_knots[lastIdx].GetAmplitude() - x8_knots[0].GetAmplitude());
  }

  if (!pre && x4_postInfinity == 3) {
    return eval + (tmp * x8_knots[lastIdx].GetAmplitude() - x8_knots[0].GetAmplitude());
  }
  return eval;
}

float CMayaSpline::EvaluateHermite(float time) {
  const float timeDiff = time - x30_cachedMinTime;
  return x34_cachedHermitCoefs[0] + (timeDiff * x34_cachedHermitCoefs[1]) +
         (timeDiff * x34_cachedHermitCoefs[2]) + (timeDiff * x34_cachedHermitCoefs[3]);
}

bool CMayaSpline::FindKnot(float time, int& knotIndex) {
  if (x8_knots.empty()) {
    return false;
  }

  u32 lower = 0;
  u32 upper = x8_knots.size();
  while (lower < upper) {
    u32 index = (lower + upper) / 2;
    const CMayaSplineKnot& knot = x8_knots[index];
    if (knot.GetTime() > time) {
      upper = index - 1;
    } else if (time > knot.GetTime()) {
      lower = index + 1;
    } else {
      knotIndex = index;
      return true;
    }
  }

  knotIndex = lower;
  return false;
}

void CMayaSpline::FindControlPoints(int knotIndex,
                                    rstl::reserved_vector< CVector2f, 4 >& controlPoints) {
  CMayaSplineKnot* knot = &x8_knots[knotIndex];
  controlPoints.push_back(CVector2f(knot->GetTime(), knot->GetAmplitude()));

  CVector2f tangentA(0.0f, 0.0f);
  CVector2f tangentB(0.0f, 0.0f);
  CMayaSplineKnot* next = (knotIndex + 1 < x8_knots.size()) ? &x8_knots[knotIndex + 1] : nullptr;
  CMayaSplineKnot* prev = (knotIndex - 1 >= 0) ? &x8_knots[knotIndex - 1] : nullptr;
  knot->GetTangents(prev, next, tangentA, tangentB);

  knot = &x8_knots[knotIndex + 1];
  controlPoints.push_back(CVector2f(controlPoints[0] + (tangentB * 1.f / 3.f)));
  next = (knotIndex + 2 < x8_knots.size()) ? &x8_knots[knotIndex + 2] : nullptr;
  prev = (knotIndex - 2 >= 0) ? &x8_knots[knotIndex - 2] : nullptr;
  knot->GetTangents(prev, next, tangentA, tangentB);
  CVector2f knotV(knot->GetTime(), knot->GetAmplitude());
  controlPoints.push_back(CVector2f(knotV - (tangentA * 1.f / 3.f)));
  controlPoints.push_back(knotV);
}

void CMayaSpline::CalculateHermiteCoefficients(
    const rstl::reserved_vector< CVector2f, 4 >& controlPoints, float* coefs) {
  const CVector2f point1 = controlPoints[3] - controlPoints[0];
  const CVector2f point2 = controlPoints[1] - controlPoints[0];
  const CVector2f point3 = controlPoints[3] - controlPoints[2];
  float dVar7 = point2.GetX() != 0.f ? point2.GetY() / point2.GetX() : 5729578.0f;
  float dVar6 = point3.GetX() != 0.f ? point3.GetY() / point3.GetX() : 5729578.0f;
  const float point1XSq = point1.GetX() * point1.GetX();
  coefs[0] =
      ((1.f / (point1XSq)) *
       (((dVar7 * point1.GetX()) + (dVar6 * point1.GetX()) - point1.GetY()) - point1.GetY())) /
      point1.GetX();
  coefs[1] = ((1.f / (point1XSq)) *
              ((((point1.GetY() + (point1.GetY() + point1.GetY())) - (dVar7 * point1.GetX())) -
                (dVar7 * point1.GetX())) -
               (dVar6 * point1.GetX())));
  coefs[2] = dVar7;
  coefs[3] = controlPoints[0].GetY();
}
