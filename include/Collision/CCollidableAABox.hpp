#ifndef _CCOLLIDABLEAABOX
#define _CCOLLIDABLEAABOX

#include "types.h"

#include "Collision/CCollisionPrimitive.hpp"

class CCollidableAABox : public CCollisionPrimitive {
public:
  CCollidableAABox();
  CCollidableAABox(const CAABox& box, const CMaterialList& matList)
  : CCollisionPrimitive(matList), mAabb(box) {}

  static bool CollideMovingAABox(const CInternalCollisionStructure& collision, const CVector3f& dir,
                                 double& dOut, CCollisionInfo& infoOut);
  static bool CollideMovingSphere(const CInternalCollisionStructure& collision,
                                  const CVector3f& dir, double& dOut, CCollisionInfo& infoOut);

  // CCollisionPrimitive
  uint GetTableIndex() const override;
  CAABox CalculateAABox(const CTransform4f&) const override;
  CAABox CalculateLocalAABox() const override;
  FourCC GetPrimType() const override;
  ~CCollidableAABox() override;
  CRayCastResult CastRayInternal(const CInternalRayCastStructure&) const override;

  CAABox Transform(const CTransform4f& xf) const;
  const CAABox& GetBox() const { return mAabb; }
  void SetBox(const CAABox& box) { mAabb = box; } // Guessed name

  static void SetStaticTableIndex(uint idx);
  static CCollisionPrimitive::Type GetType();

private:
  static uint sTableIndex;

  CAABox mAabb;
};
CHECK_SIZEOF(CCollidableAABox, 0x28)

namespace Collide {
bool AABox_AABox_Bool(const CInternalCollisionStructure&);
bool AABox_AABox(const CInternalCollisionStructure& collision, CCollisionInfoList& list);
} // namespace Collide
#endif // _CCOLLIDABLEAABOX
