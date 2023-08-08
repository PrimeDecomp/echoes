#include "Kyoto/Math/CVector3i.hpp"

CVector3i CVector3i::sZeroVector = CVector3i(0, 0, 0);
CVector3i CVector3i::sUpVector = CVector3i(0, 0, 1);
CVector3i CVector3i::sDownVector = CVector3i(0, 0, -1);
CVector3i CVector3i::sLeftVector = CVector3i(-1, 0, 0);
CVector3i CVector3i::sRightVector = CVector3i(1, 0, 0);
CVector3i CVector3i::sForwardVector = CVector3i(0, 1, 0);
CVector3i CVector3i::sBackVector = CVector3i(0, -1, 0);
CVector3i CVector3i::sOneVector = CVector3i(1, 1, 1);

CVector3i::CVector3i(int x, int y, int z) : mX(x), mY(y), mZ(z) {}

bool operator==(const CVector3i& lhs, const CVector3i& rhs) {
  return lhs.GetX() == rhs.GetX() && lhs.GetY() == rhs.GetY() && lhs.GetZ() == rhs.GetZ();
}
