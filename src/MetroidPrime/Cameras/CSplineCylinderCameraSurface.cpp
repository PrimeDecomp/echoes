#include "MetroidPrime/Cameras/CCameraSurface.hpp"

#include "Kyoto/Math/CMath.hpp"

CSplineCylinderCameraSurface::CSplineCylinderCameraSurface(const CMayaSpline& spline,
                                                           const CCylinder& cylinder,
                                                           const CVector3f& referenceDirection,
                                                           float height)
: CCylinderCameraSurface(cylinder, height)
, mSpline(spline)
, mReferenceDirection(referenceDirection) {}

CSplineCylinderCameraSurface::~CSplineCylinderCameraSurface() {}

CVector3f CSplineCylinderCameraSurface::GetSurfacePoint(const CVector3f& point) {
  const CVector3f surfacePoint = CCylinderCameraSurface::GetSurfacePoint(point);
  const CVector3f relative = surfacePoint - GetCylinder().GetAxis().GetRefPoint();
  const float axialDistance = CMath::Limit(
      CVector3f::Dot(GetCylinder().GetAxis().GetNormal(), relative), 0.5f * GetHeight());
  const CVector3f axialOffset = axialDistance * GetCylinder().GetAxis().GetNormal();
  const CVector3f radialDirection = (relative - axialOffset).AsNormalized();
  const float cosine = CVector3f::Dot(radialDirection, mReferenceDirection);
  const CVector3f cross = CVector3f::Cross(radialDirection, mReferenceDirection);
  const float arc = CMath::ArcCosineR(cosine);
  float angle;
  if (cosine < -0.999999f) {
    angle = M_PIF;
  } else if (cosine > 0.999999f) {
    angle = 0.f;
  } else {
    angle = CVector3f::Dot(cross, GetCylinder().GetAxis().GetNormal()) < 0.f ? M_2PIF - arc : arc;
  }

  const float radius =
      mSpline.EvaluateAt(CMath::Clamp(0.f, angle / M_2PIF, 1.f)) + GetCylinder().GetRadius();
  return GetCylinder().GetAxis().GetRefPoint() + axialOffset + radialDirection * radius;
}

bool CSplineCylinderCameraSurface::IsPointInside(const CVector3f& point) {
  const CVector3f surfacePoint = GetSurfacePoint(point);
  const CVector3f relative = surfacePoint - GetCylinder().GetAxis().GetRefPoint();
  const float axialDistance =
      relative.Magnitude() *
      CVector3f::Dot(relative.AsNormalized(), GetCylinder().GetAxis().GetNormal());
  if (0.5f * GetHeight() < CMath::AbsF(axialDistance)) {
    return false;
  }

  const CVector3f radial = point - (GetCylinder().GetAxis().GetRefPoint() +
                                    axialDistance * GetCylinder().GetAxis().GetNormal());
  return !(GetCylinder().GetRadius() < radial.Magnitude());
}
