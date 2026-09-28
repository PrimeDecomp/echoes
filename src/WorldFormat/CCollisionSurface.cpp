#include "WorldFormat/CCollisionSurface.hpp"

CUnitVector3f CCollisionSurface::GetNormal() const {
  return CUnitVector3f(
      CVector3f::Cross(mVertices[1] - mVertices[0], mVertices[2] - mVertices[0]).AsNormalized(),
      CUnitVector3f::kN_No);
}

CPlane CCollisionSurface::GetPlane() const {
  const CVector3f surfaceNormal = GetNormal();
  const CUnitVector3f normal(surfaceNormal);
  return CPlane(CVector3f::Dot(normal, mVertices[0]), normal);
}

// Guessed name
CPlane CCollisionSurface::GetEdgePlane(int edge) const {
  const CVector3f surfaceNormal = GetNormal();
  const CUnitVector3f normal(surfaceNormal);
  const int nextVertex[] = {1, 2, 0};
  const CVector3f edgeDirection = mVertices[nextVertex[edge]] - mVertices[edge];
  const CUnitVector3f edgeNormal(CVector3f::Cross(normal, edgeDirection));
  return CPlane(CVector3f::Dot(edgeNormal, mVertices[edge]), edgeNormal);
}

// Guessed name
bool CCollisionSurface::IsDegenerate() const {
  return mVertices[0] == mVertices[1] || mVertices[1] == mVertices[2] ||
         mVertices[0] == mVertices[2];
}
