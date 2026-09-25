#ifndef _CCOLLIDABLESPHERE
#define _CCOLLIDABLESPHERE

#include "Collision/CCollisionPrimitive.hpp"
#include "Kyoto/Math/CSphere.hpp"

class CCollidableSphere : public CCollisionPrimitive {
public:
  CCollidableSphere(const CSphere& sphere, const CMaterialList& material)
  : CCollisionPrimitive(material), mSphere(sphere) {}

  uint GetTableIndex() const override;
  CAABox CalculateAABox(const CTransform4f& transform) const override;
  CAABox CalculateLocalAABox() const override;
  FourCC GetPrimType() const override;
  CRayCastResult CastRayInternal(const CInternalRayCastStructure& ray) const override;

  const CSphere& GetSphere() const { return mSphere; }
  void SetSphere(const CSphere& sphere) { mSphere = sphere; }

private:
  CSphere mSphere;
};
CHECK_SIZEOF(CCollidableSphere, 0x20)

#endif // _CCOLLIDABLESPHERE
