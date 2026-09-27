#include "Kyoto/Math/CSphere.hpp"

CUnitVector3f CSphere::GetSurfaceNormal(const CVector3f& vec) const {
  return CUnitVector3f(vec - mCenter);
}

CVector3f CSphere::GetSurfacePoint(const CVector3f& vec) const {
  const CVector3f delta = vec - mCenter;
  if (delta.IsMagnitudeSafe()) {
    return mCenter + delta.AsNormalized() * mRadius;
  }
  return mCenter;
}
