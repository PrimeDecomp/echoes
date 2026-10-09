#ifndef _CGRENCHLERTAIL
#define _CGRENCHLERTAIL

#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CRELFileToken.hpp"
#include "rstl/optional_object.hpp"

// Severed tail of a Grenchler; it is thrown off, bounces and fades out.
class CGrenchlerTail : public CPhysicsActor {
public:
  CGrenchlerTail(TUniqueId uid, TAreaId areaId, const CModelData& modelData, int animId,
                 const CTransform4f& xf, bool isSmall);

  // CEntity

  void Think(float dt, CStateManager& mgr) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;

  // CPhysicsActor
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

private:
  void StopMoving(); // Guessed name

  float mElapsedTime;      // Guessed name
  float mBounceCount;      // Guessed name
  CRELFileToken mRelToken; // Guessed name
  bool mIsSmall;           // Guessed name
};
CHECK_SIZEOF(CGrenchlerTail, 0x2e8)

#endif // _CGRENCHLERTAIL
