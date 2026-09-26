#ifndef _CCOLLISIONACTOR
#define _CCOLLISIONACTOR

#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/single_ptr.hpp"

class CCollidableOBBTreeGroupContainer;
class CCollidableOBBTreeGroup;
class CCollidableAABox;
class CCollidableSphere;
class CCollisionActorResponse; // Guessed name: shared damage/weapon-response delegate.

class CCollisionActor : public CPhysicsActor {
public:
  enum EPrimitiveType { kPT_OBBTreeGroup, kPT_AABox, kPT_Sphere };

  CCollisionActor(TUniqueId uid, TAreaId areaId, TUniqueId owner, bool active, float radius,
                  float mass);

  // CEntity
  ~CCollisionActor() override;
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  CHealthInfo* HealthInfo() override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  const CDamageVulnerability* GetDamageVulnerability(const CVector3f&, const CVector3f&,
                                                     const CDamageInfo&) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                         const CWeaponMode&, int) const override;
  void OnScanStateChange(EScanState state, CStateManager& mgr) override;

  // CPhysicsActor
  const CCollisionPrimitive* GetCollisionPrimitive() const override;
  CTransform4f GetPrimitiveTransform() const override;

  void SetDamageVulnerability(const CDamageVulnerability& vulnerability);
  TUniqueId GetLastTouchedObject() const;
  float GetSphereRadius() const;
  void SetSphereRadius(float radius);

private:
  EPrimitiveType mPrimitiveType;
  TUniqueId mOwner;
  CVector3f mBoxSize;
  CVector3f mCenter;
  rstl::single_ptr< CCollidableOBBTreeGroupContainer > mObbContainer;
  rstl::single_ptr< CCollidableOBBTreeGroup > mObbTreeGroupPrimitive;
  rstl::single_ptr< CCollidableAABox > mAaboxPrimitive;
  rstl::single_ptr< CCollidableSphere > mSpherePrimitive;
  float mSphereRadius;
  CHealthInfo mHealthInfo;
  CDamageVulnerability mDamageVulnerability;
  TUniqueId mLastTouched;
  EWeaponCollisionResponseTypes mResponseType;
  CVector3f mExtendedTouchBounds;
  rstl::rc_ptr< CCollisionActorResponse > mCollisionResponse;
};
CHECK_SIZEOF(CCollisionActor, 0x370)

#endif // _CCOLLISIONACTOR
