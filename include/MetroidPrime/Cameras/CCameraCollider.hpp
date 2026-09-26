#ifndef _CCAMERACOLLIDER
#define _CCAMERACOLLIDER

#include "Kyoto/Math/CVector3f.hpp"

class CCameraCollider {
public:
  CCameraCollider(float radius, CVector3f position, float scale);
  virtual ~CCameraCollider() {}

  float GetRadius() const { return mRadius; }
  const CVector3f& GetPosition() const { return mLastLocalPos; }
  const CVector3f& GetDesiredPosition() const { return mLocalPos; }
  const CVector3f& GetLookAtPosition() const { return mScaledWorldPos; }
  const CVector3f& GetRealPosition() const { return mLastWorldPos; }
  int GetOcclusionCount() const { return mOcclusionCount; }
  float GetScale() const { return mScale; }

  void SetPosition(const CVector3f& position) { mLastLocalPos = position; }
  void SetDesiredPosition(const CVector3f& position) { mLocalPos = position; }
  void SetLookAtPosition(const CVector3f& position) { mScaledWorldPos = position; }
  void SetRealPosition(const CVector3f& position) { mLastWorldPos = position; }
  void SetOcclusionCount(int count) { mOcclusionCount = count; }

private:
  float mRadius;
  CVector3f mLastLocalPos;
  CVector3f mLocalPos;
  CVector3f mScaledWorldPos;
  CVector3f mLastWorldPos;
  int mOcclusionCount;
  float mScale;
};
CHECK_SIZEOF(CCameraCollider, 0x40)

#endif // _CCAMERACOLLIDER
