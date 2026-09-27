#include "Kyoto/Math/CLine.hpp"

CVector3f CLine::GetClosestPoint(const CVector3f& point) const {
  const CVector3f delta = point - GetRefPoint();
  return GetRefPoint() + CVector3f::Dot(delta, GetNormal()) * GetNormal();
}
