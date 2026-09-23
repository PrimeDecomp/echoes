#include "Kyoto/Math/CloseEnough.hpp"

#include "Kyoto/Math/CQuaternion.hpp"

static CVector2f svector2_Identity(0.f, 0.f);

bool close_enough(const CQuaternion& a, const CQuaternion& b, float epsilon) {
  return close_enough(a.AxisX(), b.AxisX(), epsilon) &&
         close_enough(a.AxisY(), b.AxisY(), epsilon) &&
         close_enough(a.AxisZ(), b.AxisZ(), epsilon) &&
         close_enough(a.GetScalar(), b.GetScalar(), epsilon);
}

bool operator==(const CQuaternion& a, const CQuaternion& b) {
  return a.AxisX() == b.AxisX() && a.AxisY() == b.AxisY() && a.AxisZ() == b.AxisZ() &&
         a.GetScalar() == b.GetScalar();
}

bool close_enough(const CVector3f& a, const CVector3f& b, float epsilon) {
  return close_enough(a.GetX(), b.GetX(), epsilon) && close_enough(a.GetY(), b.GetY(), epsilon) &&
         close_enough(a.GetZ(), b.GetZ(), epsilon);
}

bool close_enough(const CVector2f& a, const CVector2f& b, float epsilon) {
  return close_enough(a.GetX(), b.GetX(), epsilon) && close_enough(a.GetY(), b.GetY(), epsilon);
}
