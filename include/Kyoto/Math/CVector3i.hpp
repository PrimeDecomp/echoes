#ifndef _CVECTOR3I
#define _CVECTOR3I

#include "rstl/construct.hpp"

class CVector3i {
public:
  CVector3i(int x, int y, int z);
  const int& operator[](int index) const { return (&mX)[index]; }
  int& operator[](int index) { return (&mX)[index]; }

  int GetX() const { return mX; }
  int GetY() const { return mY; }
  int GetZ() const { return mZ; }

  static const CVector3i& Zero() { return sZeroVector; }

private:
  int mX;
  int mY;
  int mZ;

  static CVector3i sZeroVector;
  static CVector3i sUpVector;
  static CVector3i sDownVector;
  static CVector3i sLeftVector;
  static CVector3i sRightVector;
  static CVector3i sForwardVector;
  static CVector3i sBackVector;
  static CVector3i sOneVector;
};

bool operator==(const CVector3i& lhs, const CVector3i& rhs);

namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CVector3i)
} // namespace rstl

#endif // _CVECTOR3I
