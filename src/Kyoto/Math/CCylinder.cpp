#include "Kyoto/Math/CCylinder.hpp"

CVector3f CCylinder::GetSurfacePoint(CVector3f point) const {
  CVector3f axisPoint(GetAxisPoint(point));
  return axisPoint + CVector3f(point - axisPoint).AsNormalized() * mRadius;
}

CVector3f CCylinder::GetAxisPoint(CVector3f point) const { return mAxis.GetClosestPoint(point); }

bool CCylinder::PointInside(const CVector3f& point) const {
  CVector3f axisPoint(GetAxisPoint(point));
  return CVector3f(axisPoint - point).Magnitude() <= mRadius;
}
