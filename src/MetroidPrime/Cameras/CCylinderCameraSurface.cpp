#include "MetroidPrime/Cameras/CCameraSurface.hpp"

#include "Kyoto/Math/CMath.hpp"

CCylinderCameraSurface::CCylinderCameraSurface(const CCylinder& cylinder, float height)
: mCylinder(cylinder), mHeight(height) {}

CCylinderCameraSurface::~CCylinderCameraSurface() {}

CVector3f CCylinderCameraSurface::GetSurfacePoint(CVector3f point) {
  CVector3f surfacePoint = mCylinder.GetSurfacePoint(point);
  const CVector3f relative = surfacePoint - mCylinder.GetAxis().GetRefPoint();
  const float distance = relative.Magnitude();
  const float axialDistance =
      distance * CVector3f::Dot(relative.AsNormalized(), mCylinder.GetAxis().GetNormal());
  if (CMath::AbsF(axialDistance) > 0.5f * mHeight) {
    const CVector3f normal = mCylinder.GetAxis().GetNormal();
    const CVector3f lateral =
        surfacePoint - (mCylinder.GetAxis().GetRefPoint() + axialDistance * normal);
    const float height = axialDistance > 0.f ? mHeight : -mHeight;
    surfacePoint = mCylinder.GetAxis().GetRefPoint() + 0.5f * (height * normal) + lateral;
  }
  return surfacePoint;
}

bool CCylinderCameraSurface::IsPointInside(const CVector3f& point) {
  const CVector3f surfacePoint = mCylinder.GetSurfacePoint(point);
  const CVector3f relative = surfacePoint - mCylinder.GetAxis().GetRefPoint();
  const float distance = relative.Magnitude();
  const float axialDistance =
      distance * CVector3f::Dot(relative.AsNormalized(), mCylinder.GetAxis().GetNormal());
  if (CMath::AbsF(axialDistance) > 0.5f * mHeight) {
    return false;
  }

  const CVector3f axisPoint =
      mCylinder.GetAxis().GetRefPoint() + axialDistance * mCylinder.GetAxis().GetNormal();
  return !(CVector3f(point - axisPoint).Magnitude() > mCylinder.GetRadius());
}
