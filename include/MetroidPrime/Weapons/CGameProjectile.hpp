#ifndef _CGAMEPROJECTILE
#define _CGAMEPROJECTILE

#include "Collision/CRayCastResult.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "Weapons/CProjectileWeapon.hpp"

class CLight;

class CProjectileTouchResult {
public:
  CProjectileTouchResult(TUniqueId actorId, const rstl::optional_object< CRayCastResult >& result)
  : mActorId(actorId), mRayCastResult(result) {}

  TUniqueId GetActorId() const { return mActorId; }
  bool HasRayCastResult() const { return mRayCastResult.valid(); }
  const CRayCastResult& GetRayCastResult() const { return *mRayCastResult; }

private:
  TUniqueId mActorId;
  rstl::optional_object< CRayCastResult > mRayCastResult;
};
CHECK_SIZEOF(CProjectileTouchResult, 0x38)

class CGameProjectile : public CWeapon {
public:
  enum EStaticGeometryTest {
    kSGT_None = 0,
    kSGT_CollisionGeometry = 1, // Guessed name
    kSGT_RenderGeometry = 2,    // Guessed name
  };

  CGameProjectile(bool active, const TToken< CWeaponDescription >& description,
                  const rstl::string& name, EWeaponType weaponType, const CTransform4f& xf,
                  EMaterialTypes excludeMaterial, const CDamageInfo& damageInfo, TUniqueId uid,
                  TAreaId areaId, TUniqueId owner, TUniqueId homingTarget, uint attribs,
                  bool underwater, const CVector3f& scale, const CImpactVisorEffect& visorEffect);

  // CEntity
  ~CGameProjectile() override;
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  void FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) override;

  // CGameProjectile
  virtual void StopProjectile(CStateManager& mgr);
  virtual CRayCastResult
  RayCollisionCheckWithWorld(TUniqueId& idOut, const CVector3f& start, const CVector3f& end,
                             float magnitude, rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                             CStateManager& mgr, EStaticGeometryTest staticTest);
  virtual void ResolveCollisionWithActor(const CRayCastResult& result, CActor& actor,
                                         CStateManager& mgr);
  virtual void ApplyDamageToOneActor(CStateManager& mgr, const CDamageInfo& damageInfo,
                                     TUniqueId id, const CVector3f& direction);

  CAABox GetProjectileBounds() const;
  CProjectileTouchResult CanCollideWithTrigger(CActor& actor, CStateManager& mgr);
  CProjectileTouchResult CanCollideWithGameObject(CActor& actor, CStateManager& mgr);
  CProjectileTouchResult CanCollideWithComplexCollision(CActor& actor, CStateManager& mgr);
  // Guessed name: tests the oriented box exposed by a door.
  CProjectileTouchResult CanCollideWithDoor(CActor& actor, CStateManager& mgr);
  CProjectileTouchResult CanCollideWith(CActor& actor, CStateManager& mgr);
  void ApplyDamageToActors(CStateManager& mgr, const CDamageInfo& damageInfo);
  CRayCastResult DoCollisionCheck(TUniqueId& idOut, CStateManager& mgr);
  void UpdateProjectileMovement(float dt, CStateManager& mgr);
  void UpdateHoming(float dt, CStateManager& mgr);
  void Chase(float dt, CStateManager& mgr);
  void CreateProjectileLight(const rstl::string& name, const CLight& light, CStateManager& mgr);
  void DeleteProjectileLight(CStateManager& mgr);
  static EProjectileAttrib GetBeamAttribType(EWeaponType type);

protected:
  CTransform4f mInitialTransform; // Guessed name
  CImpactVisorEffect mVisorEffect;
  CProjectileWeapon mProjectile;
  CVector3f mPreviousPos;
  float mProjExtent;
  float mHomingDt;
  double mTargetHomingTime;
  double mCurHomingTime;
  TUniqueId mHomingTargetId;
  TUniqueId mLastResolvedObj;
  TUniqueId mHitProjectileOwner;
  TUniqueId mPendingDamagee;
  TUniqueId mProjectileLight;
  CAssetId mWpscId;
  TUniqueId mTouchedDock; // Guessed name
  int x404_;              // Copied from the state manager on XCRT; meaning unresolved.
  float mMinHomingDist;
  float mHomingTurnRateScale; // Guessed name
  bool mActive : 1;
  bool mStartedUnderwater : 1;
  bool mWaterUpdate : 1;
  bool mInWater : 1;
  bool x410_4_ : 1;
  bool mAppliedDamage : 1;         // Guessed name
  bool mAppliedDamageToPlayer : 1; // Guessed name
  bool mMovingTowardTarget : 1;    // Guessed name
};
CHECK_SIZEOF(CGameProjectile, 0x418)

#endif // _CGAMEPROJECTILE
