#ifndef _CMRAY
#define _CMRAY

#include "Kyoto/Math/CVector3f.hpp"

class CTransform4f;

class CMRay {
public:
  CMRay(const CVector3f& start, const CVector3f& end);
  CMRay(const CVector3f& start, const CVector3f& end, float length, float invLength);
  CMRay(const CVector3f& start, const CVector3f& direction, float length);
  CMRay GetInvUnscaledTransformRay(const CTransform4f& xf) const;

  // Guessed name: interpolate using the full start-to-end delta.
  CVector3f GetPoint(float t) const { return mStart + t * mDelta; }

  const CVector3f& GetStart() const { return mStart; }
  const CVector3f& GetDelta() const { return mDelta; }
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
