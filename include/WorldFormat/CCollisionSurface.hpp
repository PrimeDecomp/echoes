#ifndef _CCOLLISIONSURFACE
#define _CCOLLISIONSURFACE

#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CVector3f.hpp"

class CCollisionSurface {
public:
  CCollisionSurface() {}
  CCollisionSurface(const CVector3f& a, const CVector3f& b, const CVector3f& c, u64 flags)
  : mA(a), mB(b), mC(c), mFlags(flags) {}

  CVector3f GetNormal() const;
  CPlane GetPlane() const;
  CPlane GetEdgePlane(int edge) const; // Guessed name
  bool IsDegenerate() const;           // Guessed name

  u64 GetSurfaceFlags() const { return mFlags; }
  const CVector3f& GetVert(int index) const { return (&mA)[index]; }

private:
  CVector3f mA;
  CVector3f mB;
  CVector3f mC;
  u64 mFlags;
};
CHECK_SIZEOF(CCollisionSurface, 0x30)

#endif // _CCOLLISIONSURFACE
