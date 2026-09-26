#ifndef _CMRAY
#define _CMRAY

#include "Kyoto/Math/CVector3f.hpp"

class CTransform4f;

class CMRay {
public:
  CMRay(const CVector3f& start, const CVector3f& direction, float length);
  CMRay GetInvUnscaledTransformRay(const CTransform4f& xf) const;

  const CVector3f& GetStart() const { return mStart; }
  const CVector3f& GetDirection() const { return mDirection; }

private:
  CVector3f mStart;
  CVector3f mEnd;
  CVector3f mDelta;
  float mLength;
  float mInvLength;
  CVector3f mDirection;
};
CHECK_SIZEOF(CMRay, 0x38)

#endif // _CMRAY
