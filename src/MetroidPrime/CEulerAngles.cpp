#include "MetroidPrime/CEulerAngles.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

#include <math.h>

CEulerAngles CEulerAngles::sIdentity(0.f, 0.f, 0.f);

CEulerAngles CEulerAngles::FromTransform(const CTransform4f& xf) {
  return FromMatrix(CMatrix3f::FromTransform(xf));
}

CEulerAngles CEulerAngles::FromMatrix(const CMatrix3f& mtx) {
  const float sq = sqrt(mtx.Get11() * mtx.Get11() + mtx.Get01() * mtx.Get01());

  if (!close_enough(sq, 0.f)) {
    float yaw = atan2(mtx.Get01(), mtx.Get11());
    float pitch = atan2(mtx.Get20(), mtx.Get22());
    float roll = atan2(-mtx.Get21(), sq);
    return CEulerAngles(-roll, -pitch, -yaw);
  }

  float pitch = atan2(-mtx.Get02(), mtx.Get00());
  float roll = atan2(-mtx.Get21(), sq);
  return CEulerAngles(-roll, -pitch, 0.f);
}

CEulerAngles CEulerAngles::FromQuaternion(const CQuaternion& quat) {
  float magnitudeSquared = quat.GetVector().GetX() * quat.GetVector().GetX() +
                           quat.GetVector().GetY() * quat.GetVector().GetY() +
                           quat.GetVector().GetZ() * quat.GetVector().GetZ() +
                           quat.GetScalar() * quat.GetScalar();
  float scale = magnitudeSquared > 0.f ? 2.f / magnitudeSquared : 0.f;

  float sx = scale * quat.GetVector().GetX();
  float sy = scale * quat.GetVector().GetY();
  float sz = scale * quat.GetVector().GetZ();

  float wx = sx * quat.GetScalar();
  float wy = sy * quat.GetScalar();
  float wz = sz * quat.GetScalar();
  float xx = sx * quat.GetVector().GetX();
  float xy = sy * quat.GetVector().GetX();
  float xz = sz * quat.GetVector().GetX();
  float yy = sy * quat.GetVector().GetY();
  float yz = sz * quat.GetVector().GetY();
  float zz = sz * quat.GetVector().GetZ();

  return FromMatrix(CMatrix3f(1.f - (yy + zz), xy - wz, xz + wy, xy + wz, 1.f - (xx + zz), yz - wx,
                              xz - wy, yz + wx, 1.f - (xx + yy)));
}

static float hack() {
  static float hack = 1.f;
  static float hack2 = 0.f;
  static float hack3 = 2.f;
  return hack;
}
