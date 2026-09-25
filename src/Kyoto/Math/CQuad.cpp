#include "Kyoto/Math/CQuad.hpp"

CQuad::CQuad(const CVector3f& a, const CVector3f& b, const CVector3f& c, const CVector3f& d)
: mPlane(a, b, c), mA(a), mB(b), mC(c), mD(d) {}
