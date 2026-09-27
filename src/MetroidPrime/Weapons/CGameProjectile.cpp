#include "MetroidPrime/Weapons/CGameProjectile.hpp"

#include "rstl/math.hpp"

static CTransform4f clear_transform(const CTransform4f& xf) {
  CTransform4f result(xf);
  result.SetTranslation(CVector3f::Zero());
  return result;
}

CGameProjectile::CGameProjectile(bool active, const TToken< CWeaponDescription >& description,
                                 const rstl::string& name, EWeaponType weaponType,
                                 const CTransform4f& xf, EMaterialTypes excludeMaterial,
                                 const CDamageInfo& damageInfo, TUniqueId uid, TAreaId areaId,
                                 TUniqueId owner, TUniqueId homingTarget, uint attribs,
                                 bool underwater, const CVector3f& scale,
                                 const CImpactVisorEffect& visorEffect)
: CWeapon(uid, areaId, active, owner, weaponType, name, xf,
          CMaterialFilter::MakeIncludeExclude(
              CMaterialList(kMT_Unknown59, kMT_NonSolidDamageable),
              CMaterialList(kMT_Projectile, kMT_NoPlatformCollision, excludeMaterial)),
          CMaterialList(kMT_Projectile), damageInfo, attribs | GetBeamAttribType(weaponType),
          CModelData())
, mInitialTransform(xf)
, mVisorEffect(visorEffect)
, mProjectile(description, xf.GetTranslation(), clear_transform(xf), scale,
              (attribs & kPA_ParticleOPTS) ? 1 : 0)
, mPreviousPos(xf.GetTranslation())
, mProjExtent(HasAttrib(kPA_BigProjectile) ? 0.25f : 0.1f)
, mHomingDt(0.03f)
, mTargetHomingTime(0.0)
, mCurHomingTime(mHomingDt)
, mHomingTargetId(homingTarget)
, mLastResolvedObj(kInvalidUniqueId)
, mHitProjectileOwner(kInvalidUniqueId)
, mPendingDamagee(kInvalidUniqueId)
, mProjectileLight(kInvalidUniqueId)
, mWpscId(description.GetTag().GetId())
, mTouchedDock(kInvalidUniqueId)
, x404_(0)
, mMinHomingDist(0.f)
, mHomingTurnRateScale(1.f)
, mActive(true)
, mStartedUnderwater(underwater)
, mWaterUpdate(underwater)
, mInWater(underwater)
, x410_4_(false)
, mAppliedDamage(false)
, mAppliedDamageToPlayer(false)
, mMovingTowardTarget(false) {}

void CGameProjectile::StopProjectile(CStateManager& mgr) {
  DeleteProjectileLight(mgr);
  // TODO: unregister the owner's weapon from the state manager.
  mActive = false;
  MaterialList() = CMaterialList();
  mgr.UpdateActorInSortedLists(this);
}

void CGameProjectile::Render(const CStateManager& mgr) const {
  mProjectile.Render();
  CWeapon::Render(mgr);
}

CAABox CGameProjectile::GetProjectileBounds() const {
  const CVector3f& position = GetTranslation();
  return CAABox(rstl::min_val(mPreviousPos.GetX(), position.GetX()) - mProjExtent,
                rstl::min_val(mPreviousPos.GetY(), position.GetY()) - mProjExtent,
                rstl::min_val(mPreviousPos.GetZ(), position.GetZ()) - mProjExtent,
                rstl::max_val(mPreviousPos.GetX(), position.GetX()) + mProjExtent,
                rstl::max_val(mPreviousPos.GetY(), position.GetY()) + mProjExtent,
                rstl::max_val(mPreviousPos.GetZ(), position.GetZ()) + mProjExtent);
}

void CGameProjectile::Touch(CActor& actor, CStateManager& mgr) {
  CActor::Touch(actor, mgr);
  // TODO: remember a touched dock in this projectile's area.
}

rstl::optional_object< CAABox > CGameProjectile::GetTouchBounds() const {
  if (!mActive) {
    return rstl::optional_object_null();
  }
  return GetProjectileBounds();
}

CProjectileTouchResult CGameProjectile::CanCollideWithTrigger(CActor& actor, CStateManager& mgr) {
  // TODO: test fluid entry/exit using the actor's current fluid state and EWTR/LWTR.
  return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
}

CProjectileTouchResult CGameProjectile::CanCollideWithGameObject(CActor& actor,
                                                                 CStateManager& mgr) {
  // TODO: damageability, ownership, material, patterned-actor and projectile filters.
  return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
}

CProjectileTouchResult CGameProjectile::CanCollideWithComplexCollision(CActor& actor,
                                                                       CStateManager& mgr) {
  // TODO: cast against the actor's primitive, including embedded sphere handling.
  return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
}

// Guessed name
CProjectileTouchResult CGameProjectile::CanCollideWithDoor(CActor& actor, CStateManager& mgr) {
  // TODO: cast the movement segment against the door's oriented box.
  return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
}

CProjectileTouchResult CGameProjectile::CanCollideWith(CActor& actor, CStateManager& mgr) {
  // TODO: vulnerability test and dispatch to the trigger, primitive, door or ordinary actor path.
  return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
}

CRayCastResult
CGameProjectile::RayCollisionCheckWithWorld(TUniqueId& idOut, const CVector3f& start,
                                            const CVector3f& end, float magnitude,
                                            rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                            CStateManager& mgr, EStaticGeometryTest staticTest) {
  idOut = kInvalidUniqueId;
  mPendingDamagee = kInvalidUniqueId;
  // TODO: static geometry selection and nearest actor hit, including overlapping bounds.
  return CRayCastResult();
}

void CGameProjectile::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  // TODO: XCRT state-manager snapshot, deletion cleanup and fluid-state messages.
}

void CGameProjectile::FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) {
  // TODO: call CWeapon::FluidFXThink only when the weapon description enables SWTR.
}

void CGameProjectile::ApplyDamageToOneActor(CStateManager& mgr, const CDamageInfo& damageInfo,
                                            TUniqueId id, const CVector3f& direction) {
  // TODO: direct/radius damage, hit flags and the player unfreeze attribute.
}

void CGameProjectile::ApplyDamageToActors(CStateManager& mgr, const CDamageInfo& damageInfo) {
  if (mPendingDamagee != kInvalidUniqueId) {
    ApplyDamageToOneActor(mgr, damageInfo, mPendingDamagee, GetTransform().GetForward());
    mPendingDamagee = kInvalidUniqueId;
  }
}

CRayCastResult CGameProjectile::DoCollisionCheck(TUniqueId& idOut, CStateManager& mgr) {
  // TODO: build the near list; test collision geometry in multiplayer, render geometry otherwise.
  return CRayCastResult();
}

void CGameProjectile::UpdateProjectileMovement(float dt, CStateManager& mgr) {
  const float useDt = mWaterUpdate ? 37.5f * (dt * dt) : dt;
  mPreviousPos = GetTranslation();
  mProjectile.Update(useDt);
  SetTransformAlt(mProjectile.GetTransform());
  SetTranslation(mProjectile.GetTranslation());
  UpdateHoming(dt, mgr);
  // TODO: cross touched docks and remove projectiles left in occluded areas.
}

void CGameProjectile::UpdateHoming(float dt, CStateManager& mgr) {
  if (mActive && mHomingTargetId != kInvalidUniqueId && mHomingDt > 0.f) {
    mTargetHomingTime += dt;
    while (mTargetHomingTime >= mCurHomingTime) {
      Chase(mHomingDt, mgr);
      mCurHomingTime += mHomingDt;
    }
  }
}

void CGameProjectile::Chase(float dt, CStateManager& mgr) {
  // TODO: target eligibility, aim-point selection and constrained homing rotation.
}

void CGameProjectile::CreateProjectileLight(const rstl::string& name, const CLight& light,
                                            CStateManager& mgr) {
  // TODO: create the owned light when fewer than three players are active.
}

void CGameProjectile::DeleteProjectileLight(CStateManager& mgr) {
  if (mProjectileLight != kInvalidUniqueId) {
    mgr.DeleteObjectRequest(mProjectileLight);
    mProjectileLight = kInvalidUniqueId;
  }
}

CWeapon::EProjectileAttrib CGameProjectile::GetBeamAttribType(EWeaponType type) {
  switch (type) {
  case kWT_Dark:
    return kPA_Dark;
  case kWT_Light:
    return kPA_Light;
  case kWT_Annihilator:
    return kPA_Annihilator;
  case kWT_Phazon:
    return kPA_Phazon;
  default:
    return kPA_None;
  }
}

void CGameProjectile::ResolveCollisionWithActor(const CRayCastResult& result, CActor& actor,
                                                CStateManager& mgr) {
  // TODO: apply the per-player blur, low-pass, forced-visor and billboard impact effects.
}

CGameProjectile::~CGameProjectile() {}
