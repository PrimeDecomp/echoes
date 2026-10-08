#include "MetroidPrime/Cameras/CCameraSurface.hpp"

CSphereCameraSurface::CSphereCameraSurface(const CSphere& sphere) : mSphere(sphere) {}

CSphereCameraSurface::~CSphereCameraSurface() {}

CVector3f CSphereCameraSurface::GetSurfacePoint(CVector3f point) {
  return mSphere.GetSurfacePoint(point);
}

bool CSphereCameraSurface::IsPointInside(const CVector3f& point) {
  const float distance = CVector3f(point - mSphere.GetCenter()).Magnitude();
  return distance <= mSphere.GetRadius();
}
