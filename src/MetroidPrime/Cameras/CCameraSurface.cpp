#include "MetroidPrime/Cameras/CCameraSurface.hpp"

#include "Kyoto/Math/CMath.hpp"

CPlaneCameraSurface::CPlaneCameraSurface(const CPlane& plane, const CVector3f& axisA,
                                         const CVector3f& axisB, const CVector3f& center,
                                         float width, float height)
: mPlane(plane), mAxisA(axisA), mAxisB(axisB), mCenter(center), mWidth(width), mHeight(height) {}

CPlaneCameraSurface::~CPlaneCameraSurface() {}

CVector3f CPlaneCameraSurface::GetSurfacePoint(const CVector3f& point) {
  const CVector3f relative = mPlane.GetClosestPoint(point) - mCenter;
  const float a = CMath::Limit(CVector3f::Dot(mAxisA, relative), 0.5f * mWidth);
  const float b = CMath::Limit(CVector3f::Dot(mAxisB, relative), 0.5f * mHeight);
  return mCenter + a * mAxisA + b * mAxisB;
}

bool CPlaneCameraSurface::IsPointInside(const CVector3f& point) {
  if (!mPlane.IsFacing(point)) {
    return false;
  }

  const CVector3f relative = mPlane.GetClosestPoint(point) - mCenter;
  const float a = CMath::AbsF(CVector3f::Dot(mAxisA, relative));
  const float b = CMath::AbsF(CVector3f::Dot(mAxisB, relative));
  if (a > 0.5f * mWidth || b > 0.5f * mHeight) {
    return false;
  }
  return true;
}

CSphereCameraSurface::CSphereCameraSurface(const CSphere& sphere) : mSphere(sphere) {}

CSphereCameraSurface::~CSphereCameraSurface() {}

CVector3f CSphereCameraSurface::GetSurfacePoint(const CVector3f& point) {
  return mSphere.GetSurfacePoint(point);
}

bool CSphereCameraSurface::IsPointInside(const CVector3f& point) {
  const CVector3f relative = point - mSphere.GetCenter();
  const float distance = relative.Magnitude();
  return distance <= mSphere.GetRadius();
}

CCameraSurface::CCameraSurface() {}

CCameraSurface::~CCameraSurface() {}
