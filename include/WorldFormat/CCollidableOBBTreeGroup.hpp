#ifndef _CCOLLIDABLEOBBTREEGROUP
#define _CCOLLIDABLEOBBTREEGROUP

#include "Collision/CCollisionPrimitive.hpp"
#include "WorldFormat/COBBTreeGroup.hpp"

class CCollisionCache;
class CFactoryFnReturn;
class CInputStream;
class CVParamTransfer;

class CCollidableOBBTreeGroup : public CCollisionPrimitive {
public:
  CCollidableOBBTreeGroup(COBBTreeGroup* container, const CMaterialList& material);
  CCollidableOBBTreeGroup(const COBBTreeGroup* container, const CMaterialList& material);

  // CCollisionPrimitive
  uint GetTableIndex() const override;
  CAABox CalculateAABox(const CTransform4f& xf) const override;
  CAABox CalculateLocalAABox() const override;
  FourCC GetPrimType() const override;
  ~CCollidableOBBTreeGroup() override;
  CRayCastResult CastRayInternal(const CInternalRayCastStructure& rayCast) const override;

  const COBBTreeGroup* GetContainer() const { return mContainer; }
  COBBTree* GetOBBTree(int index) const;

  static Type GetType();
  static void SetStaticTableIndex(uint index);
  static bool SphereCollide(const CInternalCollisionStructure& collision, CCollisionInfoList& list);
  static bool SphereCollideBoolean(const CInternalCollisionStructure& collision);
  static bool CollideMovingSphere(const CInternalCollisionStructure& collision,
                                  const CVector3f& direction, double& distance,
                                  CCollisionInfo& info);
  static bool AABoxCollide(const CInternalCollisionStructure& collision, CCollisionInfoList& list);
  static bool AABoxCollideBoolean(const CInternalCollisionStructure& collision);
  static bool CollideMovingAABox(const CInternalCollisionStructure& collision,
                                 const CVector3f& direction, double& distance,
                                 CCollisionInfo& info);

  // Guessed name for the Echoes collision-cache addition.
  void CacheTree(CCollisionCache& cache, const CTransform4f& xf, ushort ownerId,
                 u64 material) const;

private:
  rstl::auto_ptr< COBBTreeGroup > mOwnedContainer;
  const COBBTreeGroup* mContainer;
  static uint sTableIndex;
};
CHECK_SIZEOF(CCollidableOBBTreeGroup, 0x20)

CFactoryFnReturn FCollidableOBBTreeGroupFactory(const SObjectTag& tag, CInputStream& in,
                                                const CVParamTransfer& xfer);

#endif // _CCOLLIDABLEOBBTREEGROUP
