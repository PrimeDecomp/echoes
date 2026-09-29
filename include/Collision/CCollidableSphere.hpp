#ifndef _CCOLLIDABLESPHERE
#define _CCOLLIDABLESPHERE

#include "Collision/CCollisionPrimitive.hpp"
#include "Kyoto/Math/CSphere.hpp"

class CCollidableSphere : public CCollisionPrimitive {
public:
  CCollidableSphere(const CSphere& sphere, const CMaterialList& material)
  : CCollisionPrimitive(material), mSphere(sphere) {}

  static bool CollideMovingAABox(const CInternalCollisionStructure& collision, const CVector3f& dir,
                                 double& distance, CCollisionInfo& info);
  static bool CollideMovingSphere(const CInternalCollisionStructure& collision,
                                  const CVector3f& dir, double& distance, CCollisionInfo& info);
  static void SetStaticTableIndex(uint index);
  static Type GetType();
  static bool Sphere_AABox_Bool(const CSphere& sphere, const CAABox& box);

  // CCollisionPrimitive
  uint GetTableIndex() const override;
  CAABox CalculateAABox(const CTransform4f& transform) const override;
  CAABox CalculateLocalAABox() const override;
  FourCC GetPrimType() const override;
  ~CCollidableSphere() override;
  CRayCastResult CastRayInternal(const CInternalRayCastStructure& ray) const override;

  const CSphere& GetSphere() const { return mSphere; }
  CSphere Transform(const CTransform4f& xf) const;
  void SetSphere(const CSphere& sphere) { mSphere = sphere; }

private:
  static uint sTableIndex;
  CSphere mSphere;
};
CHECK_SIZEOF(CCollidableSphere, 0x20)

namespace Collide {
bool Sphere_Sphere(const CInternalCollisionStructure& collision, CCollisionInfoList& list);
bool Sphere_Sphere_Bool(const CInternalCollisionStructure& collision);
bool Sphere_AABox(const CInternalCollisionStructure& collision, CCollisionInfoList& list);
bool Sphere_AABox_Bool(const CInternalCollisionStructure& collision);
} // namespace Collide

#endif // _CCOLLIDABLESPHERE
