#include "MetroidPrime/Cameras/CCameraSurface.hpp"

#include "Kyoto/Math/CMath.hpp"

CSplinePlaneCameraSurface::CSplinePlaneCameraSurface(const CMayaSpline& spline, const CPlane& plane,
                                                     const CVector3f& axisA, const CVector3f& axisB,
                                                     const CVector3f& center, float width,
                                                     float height)
: mSpline(spline)
, mPlane(plane)
, mAxisA(axisA)
, mAxisB(axisB)
, mCenter(center)
, mWidth(width)
, mHeight(height) {}

CSplinePlaneCameraSurface::~CSplinePlaneCameraSurface() {}

CVector3f CSplinePlaneCameraSurface::GetSurfacePoint(const CVector3f& point) {
  const CVector3f relative = mPlane.GetClosestPoint(point) - mCenter;
  const float a = CMath::Limit(CVector3f::Dot(mAxisA, relative), 0.5f * mWidth);
  const float b = CMath::Limit(CVector3f::Dot(mAxisB, relative), 0.5f * mHeight);
  const float offset = mSpline.EvaluateAt((0.5f * mWidth + a) / mWidth);
  return mCenter + a * mAxisA + b * mAxisB + offset * mPlane.GetNormal().AsNormalized();
}

bool CSplinePlaneCameraSurface::IsPointInside(const CVector3f& point) {
  if (!mPlane.IsFacing(point)) {
    return false;
  }

  const CVector3f relative = mPlane.GetClosestPoint(point) - mCenter;
  const float a = CMath::AbsF(CVector3f::Dot(mAxisA, relative));
  const float b = CMath::AbsF(CVector3f::Dot(mAxisB, relative));
  if (a > 0.5f * mWidth || b > 0.5f * mHeight) {
    return false;
  }

  const float height = CMath::AbsF(CVector3f::Dot(mPlane.GetNormal(), relative));
  return !(height < mSpline.EvaluateAt((0.5f * mWidth + a) / mWidth));
}
