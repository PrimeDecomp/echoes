#include "Collision/CMRay.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

CMRay::CMRay(const CVector3f& start, const CVector3f& end)
: mStart(start)
, mEnd(end)
, mDelta(mEnd - mStart)
, mLength(mDelta.Magnitude())
, mInvLength(1.f / mLength)
, mDirection(mInvLength * mDelta) {}

CMRay::CMRay(const CVector3f& start, const CVector3f& end, float length, float invLength)
: mStart(start)
, mEnd(end)
, mDelta(mEnd - mStart)
, mLength(length)
, mInvLength(invLength)
, mDirection(mInvLength * mDelta) {}

CMRay::CMRay(const CVector3f& start, const CVector3f& dir, float length)
: mStart(start)
, mEnd(start + length * dir)
, mDelta(mEnd - mStart)
, mLength(length)
, mInvLength(1.f / length)
, mDirection(dir) {}

CMRay CMRay::GetInvUnscaledTransformRay(const CTransform4f& xf) const {
  CTransform4f invXf = xf.GetQuickInverse();
  return CMRay(invXf * mStart, invXf * mEnd, mLength, mInvLength);
}
