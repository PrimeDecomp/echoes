#ifndef _CBOMB
#define _CBOMB

#include "MetroidPrime/Weapons/CWeapon.hpp"

#include "Kyoto/TToken.hpp"

class CElementGen;
class CGenDescription;

class CBomb : public CWeapon {
public:
  CBomb(TToken< CGenDescription > particle1, TToken< CGenDescription > particle2, TUniqueId uid,
        TAreaId areaId, TUniqueId ownerId, const CMaterialList& triggerMaterials, EWeaponType type,
        int attribs, float fuseTime, float triggerRadius, const CTransform4f& xf,
        const CDamageInfo& damageInfo);

  // CEntity
  ~CBomb() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  void Explode(CStateManager& mgr, const rstl::optional_object< CVector3f >& position);
  void UpdateLight(float dt, CStateManager& mgr);

  void SetVelocityWR(const CVector3f& velocity) { mVelocity = velocity; }
  void SetConstantAccelerationWR(const CVector3f& acceleration) { mAcceleration = acceleration; }
  bool IsDetonated() const { return !mIsNotDetonated; }
  bool IsBeingDragged() const { return mBeingDragged; }
  void SetFuseDisabled(bool disabled) { mDisableFuse = disabled; }
  void SetIsBeingDragged(bool dragged) { mBeingDragged = dragged; }

private:
  CMaterialList mTriggerMaterials;
  CVector3f mVelocity;
  CVector3f mAcceleration;
  CVector3f mPrevLocation;
  float mFuseTime;
  float mTriggerRadius;
  rstl::single_ptr< CElementGen > mParticle1;
  rstl::single_ptr< CElementGen > mParticle2;
  TUniqueId mLightId;
  CAssetId mParticle2Id;
  bool mIsNotDetonated : 1;
  bool mBeingDragged : 1;
  bool mDisableFuse : 1;
};
CHECK_SIZEOF(CBomb, 0x210)

#endif // _CBOMB
