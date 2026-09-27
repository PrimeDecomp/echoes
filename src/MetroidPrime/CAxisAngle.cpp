#include "MetroidPrime/CAxisAngle.hpp"

#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

const CAxisAngle CAxisAngle::sIdentity(CVector3f::Zero());

CAxisAngle::CAxisAngle(const CVector3f& vec) : mVector(vec) {}

CAxisAngle::CAxisAngle(const CUnitVector3f& vec, float angle) : mVector(vec * angle) {}

CAxisAngle CAxisAngle::FromVector(const CVector3f& axis) { return CAxisAngle(axis); }

CAxisAngle CAxisAngle::FromQuaternion(const CQuaternion& quat) {
  const float angle = 2.0 * acos(quat.GetScalar());
  const float mag = quat.GetVector().Magnitude();
  if (close_enough(mag, 0.f)) {
    return Identity();
  }
  return CAxisAngle((angle / mag) * quat.GetVector());
}

const CAxisAngle& CAxisAngle::Identity() { return sIdentity; }

const CVector3f& CAxisAngle::GetVector() const { return mVector; }

float CAxisAngle::GetAngle() const { return mVector.Magnitude(); }

const CAxisAngle& CAxisAngle::operator*=(const float& rhs) {
  mVector *= rhs;
  return *this;
}

const CAxisAngle& CAxisAngle::operator+=(const CAxisAngle& rhs) {
  mVector += rhs.mVector;
  return *this;
}

CAxisAngle operator*(const CAxisAngle& lhs, const float& rhs) {
  return CAxisAngle(lhs.GetVector() * rhs);
}

CAxisAngle operator*(const float& lhs, const CAxisAngle& rhs) {
  return CAxisAngle(lhs * rhs.GetVector());
}

CAxisAngle operator+(const CAxisAngle& lhs, const CAxisAngle& rhs) {
  return CAxisAngle(lhs.GetVector() + rhs.GetVector());
}
