#include "WorldFormat/CCollisionSurface.hpp"

CUnitVector3f CCollisionSurface::GetNormal() const {
  return CUnitVector3f(
      CVector3f::Cross(mVertices[1] - mVertices[0], mVertices[2] - mVertices[0]).AsNormalized(),
      CUnitVector3f::kN_No);
}

CPlane CCollisionSurface::GetPlane() const {
  // TODO: Form the surface plane using CUnitVector3f's zero-safe normalization.
  return CPlane(0.f, CVector3f::Right());
}

// Guessed name
CPlane CCollisionSurface::GetEdgePlane(int edge) const {
  // TODO: Cross the normalized surface normal with the edge direction, normalize
  // with zero fallback and form a plane through that edge's first vertex.
  return CPlane(0.f, CVector3f::Right());
}

// Guessed name
bool CCollisionSurface::IsDegenerate() const {
  return mVertices[0] == mVertices[1] || mVertices[1] == mVertices[2] ||
         mVertices[0] == mVertices[2];
}
