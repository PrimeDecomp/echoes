#include "Kyoto/Math/CQuad.hpp"

CQuad::CQuad(const CVector3f& a, const CVector3f& b, const CVector3f& c, const CVector3f& d)
: x0_plane(a, b, c), x10_a(a), x1c_b(b), x28_c(c), x34_d(d) {}
