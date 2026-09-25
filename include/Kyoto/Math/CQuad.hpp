#ifndef _CQUAD
#define _CQUAD

#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "types.h"

// Guessed name; the G2ME01 object contains a plane and four corners.
class CQuad {
public:
  CQuad(const CVector3f& a, const CVector3f& b, const CVector3f& c, const CVector3f& d);

private:
  CPlane mPlane;
  CVector3f mA;
  CVector3f mB;
  CVector3f mC;
  CVector3f mD;
};
CHECK_SIZEOF(CQuad, 0x40)

#endif // _CQUAD
