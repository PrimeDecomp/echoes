#ifndef _CCYLINDER
#define _CCYLINDER

#include "types.h"

#include "Kyoto/Math/CLine.hpp"

// Class name and PointInside are Echoes Wii SEL exports; the other method names are guessed.
class CCylinder {
public:
  bool PointInside(const CVector3f& point) const;
  CVector3f GetSurfacePoint(const CVector3f& point) const;
  CVector3f GetAxisPoint(CVector3f point) const;

private:
  CLine mAxis;
  float mRadius;
};
CHECK_SIZEOF(CCylinder, 0x1c)

#endif // _CCYLINDER
