#ifndef _CUNITVECTOR3F
#define _CUNITVECTOR3F

#include "types.h"

#include "Kyoto/Math/CVector3f.hpp"

class CUnitVector3f : public CVector3f {
public:
  enum ENormalize {
    kN_No,
    kN_Yes,
  };
  CUnitVector3f(CInputStream& in) : CVector3f(in) { Normalize(); }
  CUnitVector3f(float x, float y, float z) : CVector3f(x, y, z) {}
  CUnitVector3f(float x, float y, float z, ENormalize normalize) : CVector3f(x, y, z) {
    if (normalize == kN_Yes) {
      Normalize();
    }
  }
  CUnitVector3f(const CVector3f& vec, ENormalize normalize) : CVector3f(vec) {
    if (normalize == kN_Yes) {
      Normalize();
    }
  }
  CUnitVector3f(const CVector3f& vec);

  static CUnitVector3f Forward() {
    return CUnitVector3f(CVector3f::Forward().GetX(), CVector3f::Forward().GetY(),
                         CVector3f::Forward().GetZ(), kN_No);
  }
};
CHECK_SIZEOF(CUnitVector3f, 0xc)

inline CUnitVector3f operator-(const CUnitVector3f& vec) {
  return CUnitVector3f(-vec.GetX(), -vec.GetY(), -vec.GetZ(), CUnitVector3f::kN_No);
}

inline const CUnitVector3f& CVector3f::Up() { return sUpVector; }
inline const CUnitVector3f& CVector3f::Down() { return sDownVector; }
inline const CUnitVector3f& CVector3f::Left() { return sLeftVector; }
inline const CUnitVector3f& CVector3f::Right() { return sRightVector; }
inline const CUnitVector3f& CVector3f::Forward() { return sForwardVector; }
inline const CUnitVector3f& CVector3f::Back() { return sBackVector; }

#endif // _CUNITVECTOR3F
