#include "Kyoto/Math/CTri.hpp"

#include "Kyoto/Math/CQuad.hpp"

CTri::CTri(const CVector3f& a, const CVector3f& b, const CVector3f& c)
: mPlane(a, b, c), mA(a), mB(b), mC(c) {}

CTri CQuad::GetTri(int index) const {
  switch (index) {
  default:
    return CTri(mA, mB, mC);
  case 0:
  case 4:
    return CTri(mA, mB, mC);
  case 1:
  case 5:
    return CTri(mB, mC, mD);
  case 2:
    return CTri(mC, mD, mA);
  case 3:
    return CTri(mD, mA, mB);
  }
}
