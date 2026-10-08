#ifndef _CMATRIX4F
#define _CMATRIX4F

#include "types.h"

#include "Kyoto/Math/CVector3f.hpp"

class CMatrix4f {
public:
  CMatrix4f(float, float, float, float, float, float, float, float, float, float, float, float,
            float, float, float, float);
  CMatrix4f(const CMatrix4f& other)
  : m00(other.m00)
  , m01(other.m01)
  , m02(other.m02)
  , m03(other.m03)
  , m10(other.m10)
  , m11(other.m11)
  , m12(other.m12)
  , m13(other.m13)
  , m20(other.m20)
  , m21(other.m21)
  , m22(other.m22)
  , m23(other.m23)
  , m30(other.m30)
  , m31(other.m31)
  , m32(other.m32)
  , m33(other.m33) {}

  CVector3f operator*(const CVector3f& vec) const;

  CVector3f MultiplyOneOverW(const CVector3f& vec) const;
  float MultiplyGetW(const CVector3f& vec) const;
  CMatrix4f GetInverse() const;
  float Determinant() const;

  static const CMatrix4f& Identity() { return sIdentity; }

private:
  static const CMatrix4f sIdentity;
  float m00;
  float m01;
  float m02;
  float m03;
  float m10;
  float m11;
  float m12;
  float m13;
  float m20;
  float m21;
  float m22;
  float m23;
  float m30;
  float m31;
  float m32;
  float m33;
};
CHECK_SIZEOF(CMatrix4f, 0x40);

#endif // _CMATRIX4F
