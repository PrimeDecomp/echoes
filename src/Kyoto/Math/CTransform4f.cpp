#include "Kyoto/Math/CTransform4f.hpp"

#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

CTransform4f CTransform4f::sIdentity(CVector3f(1.0f, 0.0f, 0.0f), CVector3f(0.0f, 0.0f, 1.0f),
                                     CVector3f(0.0f, 0.0f, 0.0f), CVector3f(0.0f, 1.0f, 0.0f));

CTransform4f::CTransform4f(float m0x, float m0y, float m0z, float m1x, float m1y, float m1z,
                           float m2x, float m2y, float m2z, float posX, float posY, float posZ)
: m0(m0x, m0y, m0z), posX(posX), m1(m1x, m1y, m1z), posY(posY), m2(m2x, m2y, m2z), posZ(posZ) {}

CTransform4f CTransform4f::RotateX(const CRelAngle& angle) {
  float fVar1;
  double dVar2;
  double dVar3;

  float s = sine(angle);
  float c = cosine(angle);
  return CTransform4f(CVector3f(1.0f, 0.0f, 0.0f), CVector3f(0.0f, c, -s), CVector3f(0.0f, s, c),
                      CVector3f(0.0f, 0.0f, 0.0f));
}

CTransform4f CTransform4f::RotateY(const CRelAngle& angle) {
  float fVar1;
  double dVar2;
  double dVar3;

  float s = sine(angle);
  float c = cosine(angle);
  return CTransform4f(CVector3f(c, -s, 0.0f), CVector3f(s, c, 0.0f), CVector3f(0.0f, 0.0f, 1.0f),
                      CVector3f(0.0f, 0.0f, 0.0f));
}

CTransform4f CTransform4f::RotateZ(const CRelAngle& angle) {
  float fVar1;
  double dVar2;
  double dVar3;

  float s = sine(angle);
  float c = cosine(angle);
  return CTransform4f(CVector3f(c, 0.0f, s), CVector3f(0.0f, 1.0f, 0.0f), CVector3f(-s, 0.0f, c),
                      CVector3f(0.0f, 0.0f, 0.0f));
}

void CTransform4f::RotateLocalX(const CRelAngle& angle) {}

void CTransform4f::RotateLocalY(const CRelAngle& angle) {}

void CTransform4f::RotateLocalZ(const CRelAngle& angle) {
  float s = sine(angle);
  float c = cosine(angle);

  float oldM0y = m0.GetY();
  float oldM1y = m1.GetY();
  float oldM2y = m2.GetY();
  float oldM0z = m0.GetZ();
  float oldM1z = m1.GetZ();
  float oldM2z = m2.GetZ();

  m0.SetY(m0.GetY() * c);
  m1.SetY(m1.GetY() * c);
  m2.SetY(m2.GetY() * c);
  m0.SetZ(m0.GetZ() * c);
  m1.SetZ(m1.GetZ() * c);
  m2.SetZ(m2.GetZ() * c);

  m0.SetY(m0.GetY() + oldM0z * s);
  m1.SetY(m1.GetY() + oldM1z * s);
  m2.SetY(m2.GetY() + oldM2z * s);
  m0.SetZ(m0.GetZ() - oldM0y * s);
  m1.SetZ(m1.GetZ() - oldM1y * s);
  m2.SetZ(m2.GetZ() - oldM2y * s);
}

CTransform4f::CTransform4f(CInputStream& in) {
  m0.SetX(in.ReadFloat());
  m0.SetY(in.ReadFloat());
  m0.SetZ(in.ReadFloat());
  posX = in.ReadFloat();
  m1.SetX(in.ReadFloat());
  m1.SetY(in.ReadFloat());
  m1.SetZ(in.ReadFloat());
  posY = in.ReadFloat();
  m2.SetX(in.ReadFloat());
  m2.SetY(in.ReadFloat());
  m2.SetZ(in.ReadFloat());
  posZ = in.ReadFloat();
}

CTransform4f::CTransform4f(const CMatrix3f& rotation, const CVector3f& translation) {
  m0 = rotation.GetRow(kDX);
  posX = translation.GetX();
  m1 = rotation.GetRow(kDY);
  posY = translation.GetY();
  m2 = rotation.GetRow(kDZ);
  posZ = translation.GetZ();
}

CTransform4f CTransform4f::Scale(float s) {
  return CTransform4f(CVector3f(s, 0.0f, 0.0f), CVector3f(0.0f, s, 0.0f), CVector3f(0.0f, 0.0f, s),
                      CVector3f(0.0f, 0.0f, 0.0f));
}

CTransform4f CTransform4f::Scale(float x, float y, float z) {
  return CTransform4f(CVector3f(x, 0.0f, 0.0f), CVector3f(0.0f, y, 0.0f), CVector3f(0.0f, 0.0f, z),
                      CVector3f(0.0f, 0.0f, 0.0f));
}

CTransform4f CTransform4f::Scale(const CVector3f& v) {
  return CTransform4f(CVector3f(v.GetX(), 0.0f, 0.0f), CVector3f(0.0f, v.GetY(), 0.0f),
                      CVector3f(0.0f, 0.0f, v.GetZ()), CVector3f(0.0f, 0.0f, 0.0f));
}

CTransform4f CTransform4f::Translate(float x, float y, float z) {
  return CTransform4f(CVector3f(1.0f, 0.0f, 0.0f), CVector3f(0.0f, 1.0f, 0.0f),
                      CVector3f(0.0f, 0.0f, 1.0f), CVector3f(x, y, z));
}

CTransform4f CTransform4f::Translate(const CVector3f& v) {
  return CTransform4f(CVector3f(1.0f, 0.0f, 0.0f), CVector3f(0.0f, 1.0f, 0.0f),
                      CVector3f(0.0f, 0.0f, 1.0f), CVector3f(v.GetX(), v.GetY(), v.GetZ()));
}

CMatrix3f CTransform4f::BuildMatrix3f() const { return CMatrix3f(m0, m1, m2); }

CTransform4f CTransform4f::operator*(const CTransform4f& vec) const {
  // TODO
  return *this;
}

void CTransform4f::ScaleBy(float s) {
  m0 *= s;
  m1 *= s;
  m2 *= s;
}

CTransform4f CTransform4f::FromColumns(const CVector3f& v1, const CVector3f& v2,
                                       const CVector3f& v3, const CVector3f& pos) {
  return CTransform4f(CVector3f(v1.GetX(), v2.GetX(), v3.GetX()),
                      CVector3f(v1.GetY(), v2.GetY(), v3.GetY()),
                      CVector3f(v1.GetZ(), v2.GetZ(), v3.GetZ()), pos);
}

CTransform4f& CTransform4f::operator=(const CTransform4f& other) {
  m0 = other.m0;
  posX = other.posX;
  m1 = other.m1;
  posY = other.posY;
  m2 = other.m2;
  posZ = other.posZ;
}

CVector3f CTransform4f::operator*(const CVector3f& vec) const {}

CVector3f CTransform4f::Rotate(const CVector3f& in) const {}

CVector3f CTransform4f::TransposeRotate(const CVector3f& in) const {}

CTransform4f CTransform4f::GetInverse() const {}
