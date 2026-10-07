#include "MetroidPrime/Cameras/CCameraSurface.hpp"

#include "Kyoto/Math/CMath.hpp"

CCylinderCameraSurface::CCylinderCameraSurface(const CCylinder& cylinder, float height)
: mCylinder(cylinder), mHeight(height) {}

CCylinderCameraSurface::~CCylinderCameraSurface() {}

CVector3f CCylinderCameraSurface::GetSurfacePoint(const CVector3f& point) {
  CVector3f surfacePoint = mCylinder.GetSurfacePoint(point);
  const CVector3f relative = surfacePoint - mCylinder.GetAxis().GetRefPoint();
  const float axialDistance =
      relative.Magnitude() *
      CVector3f::Dot(relative.AsNormalized(), mCylinder.GetAxis().GetNormal());
  if (0.5f * mHeight < CMath::AbsF(axialDistance)) {
    const float height = axialDistance > 0.f ? mHeight : -mHeight;
    surfacePoint =
        mCylinder.GetAxis().GetRefPoint() + 0.5f * height * mCylinder.GetAxis().GetNormal() +
        (surfacePoint -
         (mCylinder.GetAxis().GetRefPoint() + axialDistance * mCylinder.GetAxis().GetNormal()));
  }
  return surfacePoint;
}

bool CCylinderCameraSurface::IsPointInside(const CVector3f& point) {
  const CVector3f surfacePoint = mCylinder.GetSurfacePoint(point);
  const CVector3f relative = surfacePoint - mCylinder.GetAxis().GetRefPoint();
  const float magnitude = relative.Magnitude();
  const float axialDistance =
      magnitude * CVector3f::Dot(relative.AsNormalized(), mCylinder.GetAxis().GetNormal());
  if (CMath::AbsF(axialDistance) > 0.5f * mHeight) {
    return false;
  }

  const CVector3f radial =
      point - (mCylinder.GetAxis().GetRefPoint() + axialDistance * mCylinder.GetAxis().GetNormal());
  return !(radial.Magnitude() > mCylinder.GetRadius());
}
