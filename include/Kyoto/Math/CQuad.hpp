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
  CPlane x0_plane;
  CVector3f x10_a;
  CVector3f x1c_b;
  CVector3f x28_c;
  CVector3f x34_d;
};
CHECK_SIZEOF(CQuad, 0x40)

#endif // _CQUAD
