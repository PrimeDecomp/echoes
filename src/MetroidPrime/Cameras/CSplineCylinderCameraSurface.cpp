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

CVector3f CSplineCylinderCameraSurface::GetSurfacePoint(CVector3f point) {
  const CVector3f surfacePoint = CCylinderCameraSurface::GetSurfacePoint(point);
  const CVector3f relative = surfacePoint - CCylinder(GetCylinder()).GetAxis().GetRefPoint();
  const float axialDistance = CMath::Limit(
      CVector3f::Dot(CCylinder(GetCylinder()).GetAxis().GetNormal(), relative), 0.5f * GetHeight());
  const CVector3f axialOffset = axialDistance * CCylinder(GetCylinder()).GetAxis().GetNormal();
  CVector3f radialDirection = CVector3f(relative - axialOffset).AsNormalized();
  const float cosine = CVector3f::Dot(radialDirection, mReferenceDirection);
  const CVector3f cross = CVector3f::Cross(radialDirection, mReferenceDirection);
  float angle = CMath::ArcCosineR(cosine);
  if (cosine < -0.999999f) {
    angle = M_PIF;
  } else if (cosine > 0.999999f) {
    angle = 0.f;
  } else if (CVector3f::Dot(cross, CCylinder(GetCylinder()).GetAxis().GetNormal()) < 0.f) {
    angle = M_2PIF - angle;
  }

  float radius = mSpline.EvaluateAt(CMath::Clamp(0.f, angle / M_2PIF, 1.f));
  radius += CCylinder(GetCylinder()).GetRadius();
  radialDirection *= radius;
  return CCylinder(GetCylinder()).GetAxis().GetRefPoint() + axialOffset + radialDirection;
}

bool CSplineCylinderCameraSurface::IsPointInside(const CVector3f& point) {
  const CVector3f surfacePoint = GetSurfacePoint(point);
  const CVector3f relative = surfacePoint - CCylinder(GetCylinder()).GetAxis().GetRefPoint();
  const float distance = relative.Magnitude();
  const float axialDistance =
      distance *
      CVector3f::Dot(relative.AsNormalized(), CCylinder(GetCylinder()).GetAxis().GetNormal());
  if (CMath::AbsF(axialDistance) > 0.5f * GetHeight()) {
    return false;
  }

  const CVector3f axisPoint = CCylinder(GetCylinder()).GetAxis().GetRefPoint() +
                              axialDistance * CCylinder(GetCylinder()).GetAxis().GetNormal();
  const float radius = CCylinder(GetCylinder()).GetRadius();
  return !(CVector3f(point - axisPoint).Magnitude() > radius);
}
