#include "Kyoto/Animation/CAnimMathUtils.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"

#include "rstl/math.hpp"

#include <math.h>

const float CAnimMathUtils::kInterpolationThreshold = 0.0001f;
bool CAnimMathUtils::sUseFastSlerp; // Guessed name.

CQuaternion CAnimMathUtils::Slerp(const CQuaternion& start, const CQuaternion& end, float t) {
  const float dot = CQuaternion::Dot(start, end);
  if (dot < -0.9999999f) {
    return t < 0.5f ? start : end;
  }

  if (sUseFastSlerp) {
    if (dot >= 0.866f) {
      const float scalar = start.GetScalar() + t * (end.GetScalar() - start.GetScalar());
      const CVector3f vector = start.GetVector() + t * (end.GetVector() - start.GetVector());
      const float invMag = CMath::InvSqrtF(scalar * scalar + vector.MagSquared());
      return CQuaternion(scalar * invMag, invMag * vector);
    }

    const double angle = acosf(dot);
    const double sineAngle = sin(angle);
    const double a = sin(angle * (1.f - t));
    const double b = sin(angle * t);
    const double invSine = 1.0 / sineAngle;
    const float aScale = a * invSine;
    const float bScale = b * invSine;
    return CQuaternion(aScale * start.GetScalar() + bScale * end.GetScalar(),
                       aScale * start.GetVector() + bScale * end.GetVector());
  }

  const double startW = start.GetScalar();
  const double startX = start.AxisX();
  const double startY = start.AxisY();
  const double startZ = start.AxisZ();
  const double endW = end.GetScalar();
  const double endX = end.AxisX();
  const double endY = end.AxisY();
  const double endZ = end.AxisZ();
  const double product = startW * endW + startX * endX + startY * endY + startZ * endZ;
  const double angle = acos(rstl::min_val(product, 1.0));
  const double sineAngle = sin(angle);
  if (sineAngle < 1e-99) {
    return start;
  }

  const double a = sin(angle * (1.f - t));
  const double b = sin(angle * t);
  const double invSine = 1.0 / sineAngle;
  const double bScale = b * invSine;
  const double aScale = a * invSine;
  return CQuaternion(startW * aScale + endW * bScale, startX * aScale + endX * bScale,
                     startY * aScale + endY * bScale, startZ * aScale + endZ * bScale);
}

CQuaternion CAnimMathUtils::SlerpLocal(const CQuaternion& start, const CQuaternion& end, float t) {
  return CQuaternion::Dot(start, end) >= 0.f ? Slerp(start, end, t)
                                             : Slerp(start, end.BuildEquivalent(), t);
}
