#include "Kyoto/Math/CPlane.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CPlane::CPlane(const CVector3f& a, const CVector3f& b, const CVector3f& c)
: x0_normal(CVector3f::Cross(b - a, c - a)), xc_constant(CVector3f::Dot(x0_normal, a)) {}

CPlane::CPlane(CInputStream& in)
: x0_normal(in), xc_constant(in.ReadFloat()) {}

float CPlane::ClipLineSegment(const CVector3f& start, const CVector3f& end) const {
  float dist = -(CVector3f::Dot(start, GetNormal()) - GetConstant()) /
               CVector3f::Dot(end - start, GetNormal());
  return dist <= 0.f ? 0.f : (dist >= 1.f ? 1.f : dist);
}

CVector3f CPlane::GetClosestPoint(const CVector3f& point) const {
  return point - GetHeight(point) * GetNormal();
}
