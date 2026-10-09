#include "MetroidPrime/Weapons/CGameProjectile.hpp"

#include "Collision/CCollidableAABox.hpp"
#include "Collision/CCollisionPrimitive.hpp"
#include "Collision/CInternalRayCastStructure.hpp"
#include "Collision/CMRay.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CControlHintManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CHintManager.hpp"
#include "MetroidPrime/CRagDoll.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CSwarmBasics.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/RenderGeometryRayCast.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDamageableTriggerOrientated.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDock.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlayerHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "Weapons/CWeaponDescription.hpp"
#include <math.h>

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
              CMaterialList(kMT_Solid, kMT_NonSolidDamageable),
              CMaterialList(kMT_Projectile, kMT_ProjectilePassthrough, excludeMaterial)),
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
, mCreationRenderFrameIndex(0)
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
  mgr.RemoveWeaponId(GetOwnerId(), GetType());
  mActive = false;
  MaterialList() = CMaterialList();
  mgr.UpdateActorInSortedLists(this);
}

void CGameProjectile::Render(const CStateManager& mgr) const {
  mProjectile.Render();
  CWeapon::Render(mgr);
}

CAABox CGameProjectile::GetProjectileBounds() const {
  const CVector3f position = GetTranslation();
  return CAABox(rstl::min_val(mPreviousPos.GetX(), position.GetX()) - mProjExtent,
                rstl::min_val(mPreviousPos.GetY(), position.GetY()) - mProjExtent,
                rstl::min_val(mPreviousPos.GetZ(), position.GetZ()) - mProjExtent,
                rstl::max_val(mPreviousPos.GetX(), position.GetX()) + mProjExtent,
                rstl::max_val(mPreviousPos.GetY(), position.GetY()) + mProjExtent,
                rstl::max_val(mPreviousPos.GetZ(), position.GetZ()) + mProjExtent);
}

void CGameProjectile::Touch(CActor& actor, CStateManager& mgr) {
  CActor::Touch(actor, mgr);
  if (CScriptDock* dock = TCastToPtr< CScriptDock >(actor)) {
    if (dock->GetCurrentAreaId() == GetCurrentAreaId()) {
      mTouchedDock = actor.GetUniqueId();
    }
  }
}

rstl::optional_object< CAABox > CGameProjectile::GetTouchBounds() const {
  if (!mActive) {
    return rstl::optional_object_null();
  }
  return GetProjectileBounds();
}

CProjectileTouchResult CGameProjectile::CanCollideWithTrigger(CActor& actor, CStateManager& mgr) {
  const bool isWater = TCastToPtr< CScriptWater >(actor) != nullptr;
  if (isWater) {
    const bool enteredWater =
        isWater && GetFluidCount() == 0 && !mProjectile.GetWeaponDescription()->mEWTR;
    const bool leftWater =
        !isWater && GetFluidCount() != 0 && !mProjectile.GetWeaponDescription()->mLWTR;
    const bool collide = enteredWater || leftWater;
    return CProjectileTouchResult(collide ? actor.GetUniqueId() : kInvalidUniqueId,
                                  rstl::optional_object_null());
  }
  return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
}

CProjectileTouchResult CGameProjectile::CanCollideWithGameObject(CActor& actor,
                                                                 CStateManager& mgr) {
  CGameProjectile* projectile = TCastToPtr< CGameProjectile >(actor);
  if (!projectile) {
    CPatterned* patterned = TCastToPtr< CPatterned >(actor);
    if (patterned && patterned->GetRagDoll()) {
      return patterned->GetRagDoll()->ProjectileCollision(*this, actor.GetUniqueId());
    }
    if (CSwarmBasics* swarm = TCastToPtr< CSwarmBasics >(actor)) {
      if (!swarm->GetMaterialList().HasMaterial(kMT_Solid)) {
        return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
      }
    }
    if (!actor.GetMaterialList().HasMaterial(kMT_Solid) && !actor.GetHealthInfo()) {
      return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
    }
    if (actor.GetUniqueId() == GetOwnerId()) {
      return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
    }
    if (actor.GetUniqueId() == mLastResolvedObj) {
      return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
    }
    if (actor.GetMaterialList().SharesMaterials(GetFilter().GetExcludeList())) {
      return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
    }
    if (patterned && !patterned->CanBeShot(mgr, GetAttribField())) {
      return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
    }
  } else if (HasAttrib(kPA_PartialCharge) || projectile->HasAttrib(kPA_PartialCharge)) {
    return CProjectileTouchResult(actor.GetUniqueId(), rstl::optional_object_null());
  } else if (!HasAttrib(kPA_PartialCharge) && !projectile->HasAttrib(kPA_PartialCharge)) {
    return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
  }
  return CProjectileTouchResult(actor.GetUniqueId(), rstl::optional_object_null());
}

CProjectileTouchResult CGameProjectile::CanCollideWithComplexCollision(CActor& actor,
                                                                       CStateManager& mgr) {
  CPhysicsActor* physicsActor = TCastToPtr< CPhysicsActor >(&actor);
  CPhysicsActor* useActor = nullptr;
  if (CCollisionActor* collisionActor = TCastToPtr< CCollisionActor >(actor)) {
    if (collisionActor->GetOwnerId() == GetOwnerId()) {
      return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
    }
    useActor = collisionActor;
  } else if (physicsActor->GetCollisionPrimitive()->GetPrimType() == 'OBTG') {
    useActor = physicsActor;
  }
  if (!useActor) {
    return CProjectileTouchResult(actor.GetUniqueId(), rstl::optional_object_null());
  }
  const CCollisionPrimitive* primitive = useActor->GetCollisionPrimitive();
  const CTransform4f xf = useActor->GetPrimitiveTransform();
  const CVector3f delta = GetTranslation() - mPreviousPos;
  if (delta.CanBeNormalized()) {
    const CVector3f direction = delta.AsNormalized();
    const float magnitude = delta.Magnitude();
    const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
        CMaterialList(kMT_Solid), CMaterialList(kMT_ProjectilePassthrough));
    const CRayCastResult result =
        primitive->CastRay(mPreviousPos, direction, magnitude, filter, xf);
    if (result.IsValid()) {
      return CProjectileTouchResult(actor.GetUniqueId(), result);
    }
    if (primitive->GetPrimType() != 'SPHR') {
      const CMaterialFilter secondFilter = CMaterialFilter::MakeIncludeExclude(
          CMaterialList(kMT_Solid), CMaterialList(kMT_ProjectilePassthrough));
      const CRayCastResult second =
          primitive->CastRay(mPreviousPos - 1.f * (magnitude * direction), direction,
                             2.f * magnitude, secondFilter, xf);
      if (second.IsValid()) {
        return CProjectileTouchResult(actor.GetUniqueId(), second);
      }
    } else if (CCollisionActor* collisionActor = TCastToPtr< CCollisionActor >(actor)) {
      const float radius = collisionActor->GetSphereRadius();
      const CVector3f offset = mPreviousPos - collisionActor->GetTranslation();
      if (CVector3f::Dot(offset, offset) < radius * radius) {
        const CVector3f point = mPreviousPos - 1.125f * (radius * direction);
        const CPlane plane(point, CUnitVector3f(-direction));
        return CProjectileTouchResult(actor.GetUniqueId(),
                                      CRayCastResult(0.f, point, plane, actor.GetMaterialList()));
      }
    }
  }
  return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
}

// Reconstructed name: the cast and box accessor identify the oriented damageable trigger.
CProjectileTouchResult CGameProjectile::CanCollideWithOrientatedTrigger(CActor& actor,
                                                                        CStateManager& mgr) {
  if (CScriptDamageableTriggerOrientated* trigger =
          TCastToPtr< CScriptDamageableTriggerOrientated >(actor)) {
    CVector3f point = CVector3f::Zero();
    CVector3f normal = CVector3f::Zero();
    float time = 0.f;
    if (trigger->GetOBBox().LineIntersectsBox(CMRay(mPreviousPos, GetTranslation()), point, time,
                                              &normal)) {
      const CPlane plane(point, CUnitVector3f(normal));
      return CProjectileTouchResult(actor.GetUniqueId(),
                                    CRayCastResult(time, point, plane, trigger->GetMaterialList()));
    }
  }
  return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
}

CProjectileTouchResult CGameProjectile::CanCollideWith(CActor& actor, CStateManager& mgr) {
  const CDamageVulnerability& vuln = *actor.GetDamageVulnerability();
  if (vuln.GetVulnerability(mCurDamageInfo.GetWeaponMode()).mEffect ==
      CWeaponTypeVulnerability::kE_PassThrough) {
    return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
  }
  if (TCastToPtr< CScriptTrigger >(actor)) {
    return CanCollideWithTrigger(actor, mgr);
  }
  CPhysicsActor* physicsActor =
      const_cast< CPhysicsActor* >(TCastToConstPtr< CPhysicsActor >(&actor));
  if (TCastToPtr< CCollisionActor >(physicsActor) ||
      (physicsActor && physicsActor->GetCollisionPrimitive()->GetPrimType() == 'OBTG')) {
    return CanCollideWithComplexCollision(actor, mgr);
  }
  if (TCastToPtr< CScriptDamageableTriggerOrientated >(actor)) {
    return CanCollideWithOrientatedTrigger(actor, mgr);
  }
  return CanCollideWithGameObject(actor, mgr);
}

CRayCastResult
CGameProjectile::RayCollisionCheckWithWorld(TUniqueId& idOut, const CVector3f& start,
                                            const CVector3f& end, float magnitude,
                                            rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                            CStateManager& mgr, EStaticGeometryTest staticTest) {
  idOut = kInvalidUniqueId;
  mPendingDamagee = kInvalidUniqueId;
  CRayCastResult result = CRayCastResult::MakeInvalid();
  const CVector3f delta = end - start;
  if (!delta.CanBeNormalized()) {
    return result;
  }
  const CVector3f direction = delta.AsNormalized();
  float bestMagnitude = magnitude;
  CRayCastResult worldResult = CRayCastResult::MakeInvalid();
  switch (staticTest) {
  case kSGT_None:
    break;
  case kSGT_CollisionGeometry:
    worldResult =
        CGameCollision::RayStaticIntersection(mgr, start, direction, magnitude, GetFilter());
    break;
  case kSGT_RenderGeometry:
    worldResult = RenderGeometryRayCast::RayWorldIntersection(mgr, start, direction, magnitude,
                                                              GetFilter(), nullptr);
    break;
  }
  if (worldResult.IsValid()) {
    bestMagnitude = worldResult.GetTime();
    result = worldResult;
  }
  for (rstl::reserved_vector< TUniqueId, 1024 >::iterator it = nearList.begin();
       it != nearList.end(); ++it) {
    if (CActor* actor = static_cast< CActor* >(mgr.ObjectById(*it))) {
      const CProjectileTouchResult touch = CanCollideWith(*actor, mgr);
      if (touch.GetActorId() == kInvalidUniqueId) {
        continue;
      }
      if (touch.HasRayCastResult()) {
        if (touch.GetRayCastResult().GetTime() < bestMagnitude) {
          actor->Touch(*this, mgr);
          result = touch.GetRayCastResult();
          bestMagnitude = result.GetTime();
          mPendingDamagee = idOut = touch.GetActorId();
        }
      } else {
        rstl::optional_object< CAABox > bounds = actor->GetTouchBounds();
        const CGameProjectile* projectile = TCastToConstPtr< CGameProjectile >(actor);
        if (projectile) {
          bounds = projectile->GetProjectileBounds();
        }
        if (!bounds) {
          continue;
        }
        const CCollidableAABox primitive = CCollidableAABox(*bounds, actor->GetMaterialList());
        const CRayCastResult ray =
            primitive.CastRay(start, direction, magnitude, CMaterialFilter::GetPassEverything(),
                              CTransform4f::Identity());
        if (ray.IsValid()) {
          if (ray.GetTime() < bestMagnitude) {
            bestMagnitude = ray.GetTime();
            result = ray;
            mPendingDamagee = idOut = touch.GetActorId();
          }
        } else if (bounds->PointInside(start) ||
                   (projectile && GetProjectileBounds().DoBoundsOverlap(*bounds))) {
          result = CRayCastResult(0.f, start, CPlane(start, CUnitVector3f(-direction)),
                                  actor->GetMaterialList());
          mPendingDamagee = idOut = actor->GetUniqueId();
          break;
        }
      }
    }
  }
  if (mInWater && idOut == kInvalidUniqueId) {
    mInWater = false;
  }
  return result;
}

void CGameProjectile::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Create:
    mCreationRenderFrameIndex = mgr.GetRenderFrameIndex();
    break;
  case kSM_Delete:
    DeleteProjectileLight(mgr);
    break;
  case kSM_EnteredFluid:
    if (mInWater != true) {
      mInWater = true;
      mWaterUpdate = true;
    }
    break;
  case kSM_InsideFluid:
    if (!mWaterUpdate) {
      mWaterUpdate = true;
    }
    break;
  case kSM_ExitedFluid:
    if (mWaterUpdate) {
      mWaterUpdate = false;
      mInWater = false;
    }
    break;
  default:
    break;
  }
}

void CGameProjectile::FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) {
  const bool swoosh = mProjectile.GetWeaponDescription()->mSWTR;
  if (swoosh) {
    CWeapon::FluidFXThink(state, water, mgr);
  }
}

void CGameProjectile::ApplyDamageToOneActor(CStateManager& mgr, const CDamageInfo& damageInfo,
                                            TUniqueId id, const CVector3f& direction) {
  if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(id))) {
    if (damageInfo.ShouldApplyRadiusDamage()) {
      mgr.ApplyRadiusDamage(*this, GetTranslation(), *actor, GetOwnerId(), damageInfo);
    } else {
      mgr.ApplyDamage(GetUniqueId(), actor->GetUniqueId(), GetOwnerId(), damageInfo, GetFilter(),
                      direction);
    }
    mAppliedDamage = true;
    if (CPlayer* player = TCastToPtr< CPlayer >(actor)) {
      mAppliedDamageToPlayer = true;
      if (HasAttrib(kPA_PlayerUnFreeze) && player->GetFrozenState()) {
        player->BreakFrozenState(mgr, CPlayer::kBFS_BreakWithEffects, false);
      }
    }
  }
}

void CGameProjectile::ApplyDamageToActors(CStateManager& mgr, const CDamageInfo& damageInfo) {
  const CVector3f forward = GetTransform().GetForward();
  if (mPendingDamagee != kInvalidUniqueId) {
    ApplyDamageToOneActor(mgr, damageInfo, mPendingDamagee, forward);
    mPendingDamagee = kInvalidUniqueId;
  }
}

CRayCastResult CGameProjectile::DoCollisionCheck(TUniqueId& idOut, CStateManager& mgr) {
  CRayCastResult result = CRayCastResult::MakeInvalid();
  if (mActive) {
    const CVector3f delta = GetTranslation() - mPreviousPos;
    rstl::reserved_vector< TUniqueId, 1024 > nearList;
    mgr.BuildNearList(nearList, GetProjectileBounds(),
                      CMaterialFilter::MakeExclude(CMaterialList(kMT_ProjectilePassthrough)), this);
    const EStaticGeometryTest staticTest =
        mgr.IsMultiplayer() ? kSGT_CollisionGeometry : kSGT_RenderGeometry;
    result = RayCollisionCheckWithWorld(idOut, mPreviousPos, GetTranslation(), delta.Magnitude(),
                                        nearList, mgr, staticTest);
  }
  return result;
}

void CGameProjectile::UpdateProjectileMovement(float dt, CStateManager& mgr) {
  const float useDt = mWaterUpdate ? 37.5f * (dt * dt) : dt;
  mPreviousPos = GetTranslation();
  mProjectile.Update(useDt);
  SetTransform(mProjectile.GetTransform());
  SetTranslation(mProjectile.GetTranslation());
  UpdateHoming(dt, mgr);
  if (mTouchedDock != kInvalidUniqueId) {
    if (CScriptDock* dock = static_cast< CScriptDock* >(mgr.ObjectById(mTouchedDock))) {
      const rstl::optional_object< CAABox > dockBounds = dock->GetTouchBounds();
      if (dockBounds) {
        const rstl::optional_object< CAABox > projectileBounds = GetTouchBounds();
        const bool leftDock = projectileBounds
                                  ? !dockBounds->DoBoundsOverlap(*projectileBounds)
                                  : !dockBounds->PointInside(mProjectile.GetTranslation());
        if (leftDock) {
          mTouchedDock = kInvalidUniqueId;
          if (dock->HasPointCrossedDock(mgr, mProjectile.GetTranslation())) {
            const CGameArea::Dock& areaDock =
                mgr.GetWorld()->GetAreaAlways(dock->GetCurrentAreaId()).GetDock(dock->GetDockId());
            const TAreaId connectedArea = areaDock.GetConnectedAreaId(areaDock.GetReferenceCount());
            if (connectedArea != kInvalidAreaId &&
                mgr.GetWorld()->GetAreaAlways(connectedArea).IsLoaded()) {
              mgr.SetActorAreaId(*this, connectedArea);
            }
          }
        }
      }
    }
  }
  if (mgr.GetWorld()->IsAreaValid(GetCurrentAreaId()) &&
      mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetOcclusionState() ==
          CGameArea::kOS_Occluded) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
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
  if (!mProjectile.IsProjectileActive()) {
    return;
  }
  if (mHomingTargetId == kInvalidUniqueId) {
    return;
  }
  const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mHomingTargetId));
  if (!actor) {
    return;
  }
  if (!actor->GetMaterialList().HasMaterial(kMT_Target) &&
      !actor->GetMaterialList().HasMaterial(kMT_SeekerTarget) &&
      !actor->GetMaterialList().HasMaterial(kMT_Player)) {
    mHomingTargetId = kInvalidUniqueId;
    return;
  }
  const CPlayer* owner = TCastToConstPtr< CPlayer >(mgr.GetObjectById(GetOwnerId()));
  const CWeaponMode& mode = mCurDamageInfo.GetWeaponMode();
  if (owner && owner->GetOrbitTargetId() != mHomingTargetId &&
      ((mode.GetType() == kWT_Missile && (GetAttribField() & 0x400000) != 0x400000) ||
       (mode.GetType() == kWT_Power && mode.IsComboed()))) {
    mHomingTargetId = kInvalidUniqueId;
    return;
  }
  CVector3f homingPosition = actor->GetHomingPosition(mgr, 0.f);
  if (GetType() == kWT_AI && mode.GetType() != kWT_Phazon) {
    if (const CPlayer* player = TCastToConstPtr< CPlayer >(actor)) {
      if (player->GetPlayerState()->GetChargeBeamFactor() == 1.f) {
        if (const CScriptPlayerHint* hint = TCastToConstPtr< CScriptPlayerHint >(
                player->GetPlayerHintManager()->GetCurrentHint(mgr))) {
          if (hint->GetOverrideFlags() & 0x40000) {
            homingPosition = actor->GetAimPosition(mgr, 0.f);
          }
        }
      }
    }
  }
  const CSwarmBasics* const swarm = TCastToConstPtr< CSwarmBasics >(actor);
  if (swarm) {
    const int lockOnId = swarm->GetCurrentLockOnId();
    if (swarm->GetLockOnLocationValid(lockOnId)) {
      homingPosition = swarm->GetLockOnLocation(lockOnId);
    } else {
      mHomingTargetId = kInvalidUniqueId;
      return;
    }
  }
  CVector3f delta = homingPosition - mProjectile.GetTranslation();
  const bool breakHoming = mProjectile.GetWeaponDescription()->mBHBT;
  if (breakHoming) {
    const CVector3f& movement = GetTranslation() - mPreviousPos;
    const bool movingToward = CVector3f::Dot(movement, delta) > 0.f;
    if (mMovingTowardTarget && !movingToward) {
      mHomingTargetId = kInvalidUniqueId;
      return;
    }
    mMovingTowardTarget = movingToward;
  }
  if (mMinHomingDist > 0.f && delta.Magnitude() < mMinHomingDist) {
    mHomingTargetId = kInvalidUniqueId;
    return;
  }
  const CPhysicsActor* physicsActor = TCastToConstPtr< CPhysicsActor >(actor);
  if (!physicsActor && !swarm) {
    const rstl::optional_object< CAABox > bounds = actor->GetTouchBounds();
    if (bounds) {
      delta.SetZ(delta.GetZ() +
                 0.5f * (bounds->GetMaxPoint().GetZ() - bounds->GetMinPoint().GetZ()));
    }
  }
  const CVector3f forward = mProjectile.GetTransform().GetForward();
  if ((GetAttribField() & 0x10) == 0x10) {
    delta.SetZ(mInitialTransform.GetForward().GetZ());
    if (delta.CanBeNormalized()) {
      delta.Normalize();
    }
  }
  CQuaternion rotation = CQuaternion::ShortestRotationArc(forward, delta);
  const float threshold = 2.f * rotation.GetScalar() * rotation.GetScalar() - 1.f;
  if (!(threshold > 0.99f)) {
    float turnRate = mHomingTurnRateScale * mProjectile.GetMaxTurnRate();
    if (mWaterUpdate) {
      turnRate *= 0.5f;
    }
    const CRelAngle maxTurn = CRelAngle::FromDegrees(dt * turnRate);
    const CRelAngle turn = CRelAngle::FromRadians(acosf(threshold));
    if (maxTurn.AsRadians() < turn.AsRadians()) {
      const float halfTurnSin = sinf(turn.AsRadians() * 0.5f);
      rotation = CQuaternion::ScalarVector(cosf(maxTurn.AsRadians() * 0.5f),
                                           (sinf(maxTurn.AsRadians() * 0.5f) / halfTurnSin) *
                                               rotation.GetVector());
    }
    CTransform4f xf = rotation.BuildTransform4f() * mProjectile.GetTransform();
    xf.Orthonormalize();
    mProjectile.SetWorldSpaceOrientation(xf);
  }
}

void CGameProjectile::CreateProjectileLight(const rstl::string& name, const CLight& light,
                                            CStateManager& mgr) {
  if (mgr.GetNumPlayers() < 3u) {
    DeleteProjectileLight(mgr);
    mProjectileLight = mgr.AllocateUniqueId();
    const uint sourceId = mWpscId;
    CGameLight* gameLight =
        rs_new CGameLight(mProjectileLight, GetAreaIdForPersistence(), GetActive(), name,
                          GetTransform(), GetUniqueId(), light, sourceId, 0, 0.f);
    mgr.AddObject(gameLight);
  }
}

void CGameProjectile::DeleteProjectileLight(CStateManager& mgr) {
  if (mProjectileLight != kInvalidUniqueId) {
    mgr.DeleteObjectRequest(mProjectileLight);
    mProjectileLight = kInvalidUniqueId;
  }
}

CWeapon::EProjectileAttrib CGameProjectile::GetBeamAttribType(EWeaponType type) {
  switch (type) {
  case kWT_Phazon:
    return kPA_Phazon;
  case kWT_Dark:
    return kPA_Dark;
  case kWT_Light:
    return kPA_Light;
  case kWT_Annihilator:
    return kPA_Annihilator;
  default:
    return kPA_None;
  }
}

void CGameProjectile::ResolveCollisionWithActor(const CRayCastResult& result, CActor& actor,
                                                CStateManager& mgr) {
  const CVector3f reverseDirection = -GetTransform().GetForward().AsNormalized();
  if (CPlayer* player = TCastToPtr< CPlayer >(&actor)) {
    const uint playerIndex = mgr.MaskUIdNumPlayers(player->GetUniqueId());
    if (mVisorEffect.GetBlurEffect()) {
      const CImpactVisorEffect::SBlurEffect& blur = *mVisorEffect.GetBlurEffect();
      CCameraBlurPass& pass = mgr.CameraBlurPass(playerIndex, 3);
      pass.SetBlur(CCameraBlurPass::EBlurType(blur.mType), blur.mAmount, 0.f, false);
      pass.DisableBlur(blur.mFadeOutTime);
    }
    const rstl::optional_object< rstl::pair< int, float > > lowPass =
        mVisorEffect.GetLowPassFilter();
    if (lowPass && lowPass->second > 0.f) {
      CSfxManager::AddLowPassAreaFilter(lowPass->first, lowPass->second);
    }
    if (mVisorEffect.GetForcedVisor() != CPlayerState::EPlayerVisor(-1)) {
      const CScriptControlHint::TCommandStates commands;
      static_cast< CControlHintManager* >(player->GetControlHintManager())
          ->CreateHint(mgr, rstl::string("Impact Visor Effect"), 0,
                       mVisorEffect.GetForcedVisorDuration(), 0x10, commands, GetUniqueId(),
                       CGameHint::kBHT_None, 0, 0.f, CGameHint::SCallback(), CGameHint::SCallback(),
                       0.f, 0);
      if (player->GetPlayerState()->HasVisor(mVisorEffect.GetForcedVisor())) {
        player->GetPlayerState()->StartTransitionToVisor(mVisorEffect.GetForcedVisor());
      }
    }
    const rstl::optional_object< CImpactVisorEffect::SParticleEffect >& particle =
        mVisorEffect.GetParticleEffect();
    if (particle && particle->mParticle && player->GetCameraState() == CPlayer::kCS_FirstPerson) {
      const CVector3f cameraForward = mgr.GetCameraManager(playerIndex)
                                          ->GetCurrentCameraTransform(mgr, true)
                                          .GetForward()
                                          .AsNormalized();
      const float angle =
          360.f *
          CMath::Rad2Rev(CMath::FastArcCosR(CVector3f::Dot(reverseDirection, cameraForward)));
      if (angle <= 45.f) {
        mgr.AddObject(rs_new CHUDBillboardEffect(
            rstl::optional_object< TToken< CGenDescription > >(*particle->mParticle),
            rstl::optional_object_null(), mgr.AllocateUniqueId(), true, rstl::string_l("VisorAcid"),
            CHUDBillboardEffect::GetNearClipDistance(mgr, playerIndex),
            CHUDBillboardEffect::GetScaleForPOV(mgr), playerIndex, CColor(1.f, 1.f, 1.f, 1.f),
            CVector3f(1.f, 1.f, 1.f), CVector3f::Zero(), false));
        CSfxManager::SfxStart(particle->mSound, 0x7f, player->GetSoundPan(CPlayer::kMSP_4));
        if (particle->mSendCollideMessage) {
          mgr.SendScriptMsg(player, GetUniqueId(), kSM_AcidOnVisor);
        }
      }
    }
  }
}

CGameProjectile::~CGameProjectile() {}
