#ifndef _CRAYCASTRESULT
#define _CRAYCASTRESULT

#include "Collision/CMaterialList.hpp"
#include "Kyoto/Math/CPlane.hpp"

class CTransform4f;

class CRayCastResult {
public:
  CRayCastResult()
  : mTime(0.f)
  , mPoint(CVector3f::Zero())
  , mPlane(0.f, CVector3f::Right())
  , mMaterial()
  , mValid(false) {}

  CRayCastResult(float time, const CVector3f& point, const CPlane& plane,
                 const CMaterialList& material)
  : mTime(time), mPoint(point), mPlane(plane), mMaterial(material), mValid(true) {}

  float GetTime() const { return mTime; }
  const CVector3f& GetPoint() const { return mPoint; }
  const CPlane& GetPlane() const { return mPlane; }
  const CMaterialList& GetMaterial() const { return mMaterial; }
  bool IsValid() const { return mValid; }
  void Transform(const CTransform4f& xf);

private:
  float mTime;
  CVector3f mPoint;
  CPlane mPlane;
  CMaterialList mMaterial;
  bool mValid;
};
CHECK_SIZEOF(CRayCastResult, 0x30)

#endif // _CRAYCASTRESULT
