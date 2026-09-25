#include "Kyoto/Math/CTri.hpp"

CTri::CTri(const CVector3f& a, const CVector3f& b, const CVector3f& c)
: mPlane(a, b, c), mA(a), mB(b), mC(c) {}
