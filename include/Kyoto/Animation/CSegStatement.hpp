#ifndef _CSEGSTATEMENT
#define _CSEGSTATEMENT

#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector3f.hpp"

class CSegStatement {
public:
  CSegStatement() : mRotationValid(false), mOffsetValid(false), mScaleValid(false) {}

  const CQuaternion& Orientation() const { return mRotation; }
  const CVector3f& Offset() const { return mOffset; }
  const CVector3f& Scale() const { return mScale; }
  bool RotationValid() const { return mRotationValid; }
  bool OffsetValid() const { return mOffsetValid; }
  bool ScaleValid() const { return mScaleValid; }

  void Set(const CQuaternion& rotation) {
    mRotation = rotation;
    mRotationValid = true;
  }
  void Set(const CVector3f& offset) {
    mOffset = offset;
    mOffsetValid = true;
  }
  void SetScale(const CVector3f& scale) {
    mScale = scale;
    mScaleValid = true;
  }

private:
  CQuaternion mRotation;
  CVector3f mOffset;
  CVector3f mScale;
  bool mRotationValid : 1;
  bool mOffsetValid : 1;
  bool mScaleValid : 1;
};
CHECK_SIZEOF(CSegStatement, 0x2c)

#endif // _CSEGSTATEMENT
