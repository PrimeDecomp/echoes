#ifndef _CINTERNALRAYCASTSTRUCTURE
#define _CINTERNALRAYCASTSTRUCTURE

#include "Collision/CMRay.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

class CMaterialFilter;

class CInternalRayCastStructure {
public:
  CInternalRayCastStructure(const CVector3f& start, const CVector3f& direction, float length,
                            const CTransform4f& transform, const CMaterialFilter& filter)
  : mRay(start, direction, length), mMaxTime(length), mTransform(transform), mFilter(filter) {}

  const CMRay& GetRay() const { return mRay; }
  float GetMaxTime() const { return mMaxTime; }
  const CTransform4f& GetTransform() const { return mTransform; }
  const CMaterialFilter& GetFilter() const { return mFilter; }

private:
  CMRay mRay;
  float mMaxTime;
  CTransform4f mTransform;
  const CMaterialFilter& mFilter;
};
CHECK_SIZEOF(CInternalRayCastStructure, 0x70)

#endif // _CINTERNALRAYCASTSTRUCTURE
