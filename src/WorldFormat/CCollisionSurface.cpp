#include "WorldFormat/CCollisionSurface.hpp"

CVector3f CCollisionSurface::GetNormal() const {
  const CVector3f a = GetVert(1) - GetVert(0);
  const CVector3f b = GetVert(2) - GetVert(0);
  return CVector3f::Cross(a, b).AsNormalized();
}

CPlane CCollisionSurface::GetPlane() const {
  const CUnitVector3f normal(GetNormal());
  return CPlane(CVector3f::Dot(normal, GetVert(0)), normal);
}

// Guessed name
CPlane CCollisionSurface::GetEdgePlane(int edge) const {
  const CUnitVector3f normal(GetNormal());
  const int nextVertex[] = {1, 2, 0};
  const CVector3f& vertex = GetVert(edge);
  const CVector3f edgeDirection = GetVert(nextVertex[edge]) - vertex;
  const CUnitVector3f edgeNormal(CVector3f::Cross(normal, edgeDirection));
  return CPlane(CVector3f::Dot(edgeNormal, vertex), edgeNormal);
}

// Guessed name
bool CCollisionSurface::IsDegenerate() const {
  return mA == mB || mB == mC || mA == mC;
}
