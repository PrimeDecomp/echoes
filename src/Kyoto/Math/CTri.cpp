#include "Kyoto/Math/CTri.hpp"

CTri::CTri(const CVector3f& a, const CVector3f& b, const CVector3f& c)
: x0_plane(a, b, c), x10_a(a), x1c_b(b), x28_c(c) {}
