// The target classifies floats with the pre-2.4.7 MSL values.
#define MSL_OLD_FP_CLASSIFY

#include "MetroidPrime/CEulerAngles.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

#include <math.h>

CEulerAngles CEulerAngles::sIdentity(0.f, 0.f, 0.f);

CEulerAngles CEulerAngles::FromQuaternion(const CQuaternion& quat) {
  float magnitudeSquared = quat.GetVector().GetX() * quat.GetVector().GetX() +
                           quat.GetVector().GetY() * quat.GetVector().GetY() +
                           quat.GetVector().GetZ() * quat.GetVector().GetZ() +
                           quat.GetScalar() * quat.GetScalar();
  float scale = magnitudeSquared > 0.f ? 2.f / magnitudeSquared : 0.f;

  float sx = scale * quat.GetVector().GetX();
  float sy = scale * quat.GetVector().GetY();
  float sz = scale * quat.GetVector().GetZ();

  float zz = sz * quat.GetVector().GetZ();
  float xx = sx * quat.GetVector().GetX();
  float wz = sz * quat.GetScalar();
  float yy = sy * quat.GetVector().GetY();
  float xy = sy * quat.GetVector().GetX();
  float wy = sy * quat.GetScalar();
  float xz = sz * quat.GetVector().GetX();
  float yz = sz * quat.GetVector().GetY();
  float wx = sx * quat.GetScalar();

  const CMatrix3f mtx(1.f - (yy + zz), xy - wz, xz + wy, xy + wz, 1.f - (xx + zz), yz - wx, xz - wy,
                      yz + wx, 1.f - (xx + yy));
  return FromMatrix(mtx);
}

CEulerAngles CEulerAngles::FromMatrix(const CMatrix3f& mtx) {
  const float sq = sqrt(mtx.Get11() * mtx.Get11() + mtx.Get01() * mtx.Get01());

  if (!close_enough(sq, 0.f)) {
    const float yaw = atan2(mtx.Get01(), mtx.Get11());
    const float pitch = atan2(mtx.Get20(), mtx.Get22());
    const float roll = atan2(-mtx.Get21(), sq);
    return CEulerAngles(-roll, -pitch, -yaw);
  }

  const float pitch = atan2(-mtx.Get02(), mtx.Get00());
  const float roll = atan2(-mtx.Get21(), sq);
  return CEulerAngles(-roll, -pitch, 0.f);
}

float msl_sqrtf(float x) {
  const double half = .5;
  const double three = 3.0;
  if (x > 0.0f) {
    double guess = __frsqrte(x);
    guess = half * guess * (three - guess * guess * x);
    guess = half * guess * (three - guess * guess * x);
    guess = half * guess * (three - guess * guess * x);
    return x * guess;
  } else if (x < 0.0) {
    return NAN;
  } else if (isnan(x)) {
    return NAN;
  }
  return x;
}

float sqrt(float x) { return msl_sqrtf(x); }

CEulerAngles CEulerAngles::FromTransform(const CTransform4f& xf) {
  return FromMatrix(CMatrix3f::FromTransform(xf));
}
