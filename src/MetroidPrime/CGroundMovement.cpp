#include "MetroidPrime/CGroundMovement.hpp"

#include "Collision/CCollidableAABoxSphere.hpp"
#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CCollisionPrimitive.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector3d.hpp"
#include "MetroidPrime/CAABoxFilter.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/UserNames.hpp"
#include "WorldFormat/CCollisionCache.hpp"
#include "WorldFormat/CMetroidAreaCollider.hpp"
#include "rstl/math.hpp"

#include <float.h>
#include <math.h>

void CGroundMovement::CheckFalling(CPhysicsActor& actor, CStateManager& mgr, float dt) {
  bool outOfBounds = true;
  const CAABox bounds = *actor.GetTouchBounds();
  for (CGameArea::CConstChainIterator it = mgr.GetWorld()->GetChainHead(CWorld::kC_Alive);
       it != CWorld::skGlobalEnd; ++it) {
    if (it->GetAABB().DoBoundsOverlap(*actor.GetTouchBounds())) {
      outOfBounds = false;
      break;
    }
  }
  if (!outOfBounds) {
    mgr.DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, kInvalidUniqueId, actor.GetUniqueId(),
                                    kSM_Falling, kSS_InvalidState));
  } else {
    mgr.DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, kInvalidUniqueId, actor.GetUniqueId(),
                                    kSM_Landed, kSS_InvalidState));
    actor.SetAngularVelocityWR(actor.GetAngularVelocityWR() * 0.98f);
    CVector3f velocity = actor.GetTransform().TransposeRotate(actor.GetVelocityWR());
    velocity.SetZ(0.f);
    actor.SetVelocityOR(velocity);
    actor.SetMomentumWR(CVector3f::Zero());
  }
}

void CGroundMovement::MoveGroundCollider(
    CStateManager& mgr, CPhysicsActor& actor, float dt,
    const rstl::reserved_vector< TUniqueId, 1024 >* colliderList) {
  CMotionState oldState = actor.GetMotionState();
  if (IsUser(0)) {
    actor.SetMotionState(oldState);
  }
  CMotionState newState = actor.PredictMotion_Internal(dt);
  if (actor.IsStandardCollider() && newState.IsZero()) {
    actor.ClearForcesAndTorques();
    actor.MoveCollisionPrimitive(CVector3f::Zero());
    return;
  }
  float deltaMag = newState.GetTranslation().Magnitude();
  CCollisionInfoList collisionList;
  TUniqueId idDetect = kInvalidUniqueId;
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  const CMaterialFilter& filter = actor.GetMaterialFilter();
  CAABox motionVolume = actor.GetMotionVolume(dt);
  actor.MoveCollisionPrimitive(newState.GetTranslation());
  if (colliderList != nullptr) {
    nearList = *colliderList;
  } else {
    mgr.BuildColliderList(nearList, actor, motionVolume);
  }
  CAreaCollisionCache cache(motionVolume);
  float collideDt = dt;
  if (actor.GetCollisionPrimitive()->GetPrimType() != 'OBTG') {
    CGameCollision::BuildAreaCollisionCache(mgr, cache);
    if (deltaMag >
        0.5f * CGameCollision::GetMinExtentForCollisionPrimitive(*actor.GetCollisionPrimitive())) {
      CAABox bounds = actor.GetCollisionPrimitive()->CalculateAABox(actor.GetPrimitiveTransform());
      CVector3f point = bounds.GetCenterPoint();
      CVector3f direction = newState.GetTranslation() / deltaMag;
      TUniqueId intersectId = kInvalidUniqueId;
      const CMaterialFilter& rayFilter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
      CRayCastResult result =
          mgr.RayWorldIntersection(intersectId, point, direction, deltaMag, rayFilter, nearList);
      if (result.IsValid()) {
        collideDt = dt * (result.GetTime() / deltaMag);
        newState = actor.PredictMotion_Internal(collideDt);
      }
    }
  }
  if (CGameCollision::DetectCollision_Cached(mgr, cache, *actor.GetCollisionPrimitive(),
                                             actor.GetPrimitiveTransform(), filter, nearList,
                                             idDetect, collisionList)) {
    float stepUp = actor.GetStepUpHeight();
    float resolved = 0.f;
    actor.AddMotionState(newState);
    if (ResolveUpDown(cache, mgr, actor, filter, nearList, stepUp, 0.f, resolved, collisionList)) {
      actor.SetMotionState(oldState);
      MoveGroundColliderXY(cache, mgr, actor, filter, nearList, collideDt);
    }
  } else {
    actor.AddMotionState(newState);
  }
  float resolved = 0.f;
  float stepDown = actor.GetStepDownHeight();
  collisionList.Clear();
  TUniqueId stepZId = kInvalidUniqueId;
  if (stepDown >= 0.f && MoveGroundColliderZ(cache, mgr, actor, filter, nearList, -stepDown,
                                             resolved, collisionList, stepZId)) {
    if (collisionList.GetCount() > 0) {
      CCollisionInfoList filteredList;
      CollisionUtil::FilterByClosestNormal(CVector3f(0.f, 0.f, 1.f), collisionList, filteredList);
      if (filteredList.GetCount() > 0) {
        CCollisionInfo info = filteredList[0];
        if (CGameCollision::IsFloor(info.GetMaterialLeft(), info.GetNormalLeft())) {
          CEntity* entity = mgr.ObjectById(stepZId);
          if (TCastToPtr< CScriptPlatform >(entity)) {
            mgr.DeliverScriptMsg(CScriptMsg(actor.GetUniqueId(), kInvalidUniqueId,
                                            entity->GetUniqueId(), kSM_AddPlatformRider,
                                            kSS_InvalidState));
          }
          CGameCollision::SendMaterialMessage(mgr, info.GetMaterialLeft(), actor);
          mgr.DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, kInvalidUniqueId, actor.GetUniqueId(),
                                          kSM_Landed, kSS_InvalidState));
          if (!TCastToPtr< CScriptPlatform >(entity)) {
            mgr.DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, kInvalidUniqueId, actor.GetUniqueId(),
                                            static_cast< EScriptObjectMessage >('XLSG'),
                                            kSS_InvalidState));
          }
        } else {
          CheckFalling(actor, mgr, dt);
        }
      }
    }
  } else {
    CheckFalling(actor, mgr, dt);
  }
  actor.ClearForcesAndTorques();
  actor.MoveCollisionPrimitive(CVector3f::Zero());
  if (actor.GetMaterialList().HasMaterial(kMT_Player)) {
    CGameCollision::CollisionFailsafe(mgr, cache, actor, *actor.GetCollisionPrimitive(), nearList,
                                      0.f, 1, 0.f);
  }
}

bool CGroundMovement::ResolveUpDown(CAreaCollisionCache& cache, CStateManager& mgr,
                                    CPhysicsActor& actor, const CMaterialFilter& filter,
                                    const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                    float stepUp, float stepDown, float& resolved,
                                    CCollisionInfoList& list) {
  if (list.GetCount() == 0) {
    return true;
  }

  CAABox bounds = CAABox::MakeMaxInvertedBox();
  CVector3f normal = CVector3f::Zero();
  for (int i = 0; i < list.GetCount(); ++i) {
    const CCollisionInfo& info = list[i];
    if (CGameCollision::IsFloor(info.GetMaterialLeft(), info.GetNormalLeft())) {
      bounds.AccumulateBounds(info.GetPoint());
      bounds.AccumulateBounds(info.GetExtreme());
      normal += info.GetNormalLeft();
    }
  }
  if (!normal.CanBeNormalized()) {
    return true;
  }
  normal = normal.AsNormalized();

  const CAABox actorBounds = actor.GetBoundingBox();
  float zExtent;
  if (normal.GetZ() >= 0.f) {
    zExtent = bounds.GetMaxPoint().GetZ() - actorBounds.GetMinPoint().GetZ() + 0.02f;
    if (zExtent > stepUp) {
      return true;
    }
  } else {
    zExtent = bounds.GetMinPoint().GetZ() - actorBounds.GetMaxPoint().GetZ() - 0.02f;
    if (zExtent < -stepDown) {
      return true;
    }
  }

  actor.MoveCollisionPrimitive(CVector3f(0.f, 0.f, zExtent));
  if (!CGameCollision::DetectCollisionBoolean_Cached(mgr, cache, *actor.GetCollisionPrimitive(),
                                                     actor.GetPrimitiveTransform(), filter,
                                                     nearList)) {
    resolved = zExtent;
    actor.SetTranslation(actor.GetTranslation() + CVector3f(0.f, 0.f, zExtent));
    actor.MoveCollisionPrimitive(CVector3f::Zero());

    bool floor = false;
    for (int i = 0; i < list.GetCount(); ++i) {
      if (CGameCollision::IsFloor(list[i].GetMaterialLeft(), list[i].GetNormalLeft())) {
        floor = true;
        break;
      }
    }
    if (!floor) {
      mgr.SendScriptMsg(CScriptMsg(kInvalidUniqueId, kInvalidUniqueId, actor.GetUniqueId(),
                                   kSM_LandOnNotFloor, kSS_InvalidState));
    }
    return false;
  }
  return true;
}

bool CGroundMovement::MoveGroundColliderZ(CAreaCollisionCache& cache, CStateManager& mgr,
                                          CPhysicsActor& actor, const CMaterialFilter& filter,
                                          const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                          float amount, float& resolved, CCollisionInfoList& list,
                                          TUniqueId& idOut) {
  actor.MoveCollisionPrimitive(CVector3f(0.f, 0.f, amount));
  idOut = kInvalidUniqueId;
  CAABox bounds = CAABox::MakeMaxInvertedBox();
  const bool collided = CGameCollision::DetectCollision_Cached(
      mgr, cache, *actor.GetCollisionPrimitive(), actor.GetPrimitiveTransform(), filter, nearList,
      idOut, list);
  if (!collided) {
    return false;
  }

  for (int i = 0; i < list.GetCount(); ++i) {
    bounds.AccumulateBounds(list[i].GetPoint());
    bounds.AccumulateBounds(list[i].GetExtreme());
  }
  const CAABox actorBounds = actor.GetBoundingBox();
  const float zExtent =
      amount > 0.f
          ? bounds.GetMinPoint().GetZ() - actorBounds.GetMaxPoint().GetZ() - 0.02f + amount
          : bounds.GetMaxPoint().GetZ() - actorBounds.GetMinPoint().GetZ() + 0.02f + amount;
  actor.MoveCollisionPrimitive(CVector3f(0.f, 0.f, zExtent));
  if (!CGameCollision::DetectCollisionBoolean_Cached(mgr, cache, *actor.GetCollisionPrimitive(),
                                                     actor.GetPrimitiveTransform(), filter,
                                                     nearList)) {
    resolved = zExtent;
    actor.SetTranslation(actor.GetTranslation() + CVector3f(0.f, 0.f, zExtent));
    actor.MoveCollisionPrimitive(CVector3f::Zero());
  }

  bool floor = false;
  for (int i = 0; i < list.GetCount(); ++i) {
    if (CGameCollision::IsFloor(list[i].GetMaterialLeft(), list[i].GetNormalLeft())) {
      floor = true;
      break;
    }
  }
  if (!floor) {
    mgr.SendScriptMsg(CScriptMsg(kInvalidUniqueId, kInvalidUniqueId, actor.GetUniqueId(),
                                 kSM_LandOnNotFloor, kSS_InvalidState));
  }

  CCollisionInfoList filteredList;
  CollisionUtil::FilterByClosestNormal(CVector3f(0.f, 0.f, amount > 0.f ? -1.f : 1.f), list,
                                       filteredList);
  if (filteredList.GetCount() > 0) {
    CGameCollision::MakeCollisionCallbacks(mgr, actor, idOut, filteredList);
  }
  return collided;
}

bool CGroundMovement::MoveGroundColliderXY(CAreaCollisionCache& cache, CStateManager& mgr,
                                           CPhysicsActor& actor, const CMaterialFilter& filter,
                                           const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                           float dt) {
  bool didCollide = false;
  bool isPlayer = actor.GetMaterialList().HasMaterial(kMT_Player);
  CMotionState oldState = actor.GetMotionState();
  if (IsUser(0)) {
    actor.MoveCollisionPrimitive(CVector3f::Zero());
    actor.SetMotionState(oldState);
  }
  static int numCollisions = 0;
  static int totalIterations = 0;
  static int peakIterationCount = 1;
  int iterationCount = 0;
  float remainingDt = dt;
  float originalDt = dt;
  CPhysicsActor* otherActor = nullptr;
  CCollisionInfoList collisionList;
  CMotionState motion = actor.PredictMotion_Internal(dt);
  float translationMag = motion.GetTranslation().Magnitude();
  float minimumTranslation = isPlayer ? rstl::max_val(translationMag / 5.f, 0.005f)
                                      : rstl::max_val(translationMag / 3.f, 0.02f);
  float minExtent =
      0.5f * CGameCollision::GetMinExtentForCollisionPrimitive(*actor.GetCollisionPrimitive());
  if (translationMag > minExtent) {
    originalDt = minExtent * (dt / translationMag);
    dt = originalDt;
    motion = actor.PredictMotion_Internal(dt);
    minimumTranslation = rstl::min_val(minExtent, minimumTranslation);
  }
  float nonCollideDt = dt;
  bool loopContinue = true;
  while (loopContinue) {
    actor.MoveCollisionPrimitive(motion.GetTranslation());
    collisionList.Clear();
    TUniqueId otherId = kInvalidUniqueId;
    ++iterationCount;
    bool collided = CGameCollision::DetectCollision_Cached(
        mgr, cache, *actor.GetCollisionPrimitive(), actor.GetPrimitiveTransform(), filter, nearList,
        otherId, collisionList);
    if (collided) {
      otherActor = TCastToPtr< CPhysicsActor >(mgr.ObjectById(otherId));
    }
    actor.MoveCollisionPrimitive(CVector3f::Zero());
    if (collided) {
      didCollide = true;
      if (motion.GetTranslation().Magnitude() < minimumTranslation) {
        CCollisionInfoList backfaceFilteredList;
        CCollisionInfoList floorFilteredList;
        CVector3f deltaVelocity = actor.GetVelocityWR();
        if (otherActor != nullptr) {
          deltaVelocity -= otherActor->GetVelocityWR();
        }
        CollisionUtil::FilterOutBackfaces(deltaVelocity, collisionList, backfaceFilteredList);
        CAABoxFilter::FilterBoxFloorCollisions(backfaceFilteredList, floorFilteredList);
        CGameCollision::MakeCollisionCallbacks(mgr, actor, otherId, floorFilteredList);
        if (floorFilteredList.GetCount() == 0 && isPlayer) {
          const CMotionState& lastState = actor.GetLastNonCollidingState();
          actor.SetMotionState(CMotionState(lastState.GetTranslation(), lastState.GetOrientation(),
                                            0.5f * lastState.GetVelocity(),
                                            lastState.GetAngularMomentum() * 0.5f));
        }
        if (IsUser(0)) {
          if (floorFilteredList.GetCount() > 0) {
            CGameCollision::ShowCollisionResults(floorFilteredList, CColor::White());
          } else {
            CGameCollision::ShowCollisionResults(collisionList, CColor::Yellow());
          }
        }
        for (int i = 0; i < floorFilteredList.GetCount(); ++i) {
          CCollisionInfo info = floorFilteredList[i];
          float restitution = CGameCollision::GetCoefficientOfRestitution(info) +
                              actor.GetCoefficientOfRestitutionModifier();
          CVector3f normal = info.GetNormalLeft();
          if (otherActor != nullptr) {
            CGameCollision::CollideWithDynamicBodyNoRot(actor, *otherActor, info, restitution,
                                                        true);
          } else {
            CGameCollision::CollideWithStaticBodyNoRot(actor, info.GetMaterialLeft(),
                                                       info.GetMaterialRight(),
                                                       CUnitVector3f(normal), restitution, true);
          }
        }
        remainingDt -= dt;
        nonCollideDt = rstl::min_val(remainingDt, originalDt);
        dt = nonCollideDt;
      } else {
        nonCollideDt *= 0.5f;
        dt *= 0.5f;
      }
    } else {
      actor.AddMotionState(motion);
      remainingDt -= dt;
      dt = nonCollideDt;
      actor.MoveCollisionPrimitive(CVector3f::Zero());
    }
    motion = actor.PredictMotion_Internal(dt);
    loopContinue = remainingDt > 0.f;
  }
  if (!didCollide && !actor.GetMaterialList().HasMaterial(kMT_GroundCollider)) {
    mgr.DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, kInvalidUniqueId, actor.GetUniqueId(),
                                    kSM_Falling, kSS_InvalidState));
  }
  actor.MoveCollisionPrimitive(CVector3f::Zero());
  if (IsUser(0)) {
    if (didCollide) {
      totalIterations += iterationCount;
      ++numCollisions;
    }
    if (iterationCount > peakIterationCount) {
      peakIterationCount = iterationCount;
    }
  }
  return didCollide;
}

CVector3f CGroundMovement::CollisionDamping(const CVector3f& velocity, const CVector3f& direction,
                                            const CVector3f& normal, float normalCoefficient,
                                            float deltaCoefficient) {
  const CVector3f reflected =
      (direction + (-2.f * normal) * CVector3f::Dot(normal, direction)).AsNormalized();
  const CVector3f normalPart = CVector3f::Dot(normal, reflected) * normal;
  return normalCoefficient * (velocity.Magnitude() * normalPart) +
         deltaCoefficient * (velocity.Magnitude() * (reflected - normalPart));
}

bool RemovePositiveZComponentFromNormal(CVector3f& normal) {
  if (normal.GetZ() > 0.f && normal.GetZ() < 0.99f) {
    normal = normal.DropZ().AsNormalized();
    return true;
  }
  return false;
}

bool CGroundMovement::RemoveNormalComponent(const CVector3f& normal, const CVector3f& direction,
                                            CVector3f& collisionNormal, float& normalDot) {
  const float dot = CVector3f::Dot(normal, collisionNormal);
  if (fabsf(dot) > 0.99f) {
    return false;
  }

  const float oldDot = CVector3f::Dot(direction, collisionNormal);
  const float newDot = CVector3f::Dot(direction, (collisionNormal - dot * normal).AsNormalized());
  if (oldDot > 0.f && newDot < 0.f) {
    return false;
  }
  if (fabsf(oldDot) > 0.01f && fabsf(newDot / oldDot) > 4.f) {
    return false;
  }

  collisionNormal -= dot * normal;
  normalDot = dot;
  return true;
}

bool CGroundMovement::RemoveNormalComponent(const CVector3f& normal, CVector3f& velocity) {
  const float dot = CVector3f::Dot(normal, velocity);
  if (fabsf(dot) > 0.99f) {
    return false;
  }
  velocity -= dot * normal;
  return true;
}

void CGroundMovement::MoveGroundCollider_New(
    CStateManager& mgr, CPhysicsActor& actor, float dt,
    const rstl::reserved_vector< TUniqueId, 1024 >* colliderList) {
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  CAABox motionVolume = actor.GetMotionVolume(dt);
  if (colliderList != nullptr) {
    nearList = *colliderList;
  } else {
    mgr.BuildColliderList(nearList, actor, motionVolume);
  }
  CCollisionCache* cachePtr = actor.GetCollisionCache();
  rstl::optional_object< CCollisionCache > localCache;
  if (cachePtr == nullptr || !motionVolume.Inside(cachePtr->GetBounds())) {
    const float padding = cachePtr != nullptr ? 0.5f : 0.f;
    const CVector3f paddingVector(padding, padding, padding);
    CVector3f minPoint = motionVolume.GetMinPoint();
    CVector3f maxPoint = motionVolume.GetMaxPoint();
    minPoint -= paddingVector;
    maxPoint += paddingVector;
    const CAABox cacheBounds(minPoint, maxPoint);
    cachePtr = new (localCache.prepare_emplace())
        CCollisionCache(cacheBounds, cachePtr != nullptr ? cachePtr->GetDynamicGeometryMode() : 1,
                        0, ushort(0xffff));
    CGameCollision::BuildCollisionCache(mgr, *cachePtr, nearList,
                                        CGameCollision::kCUP_RemoveCachedNearListIds);
  } else {
    CGameCollision::UpdateCollisionCache(mgr, *cachePtr, nearList,
                                         CGameCollision::kCUP_RemoveCachedNearListIds);
  }
  CCollisionCache& cache = *cachePtr;
  CPlayer& player = *TCastToPtr< CPlayer >(&actor);
  player.SetPlayerIsSlidingOnWall(false);
  bool applyJump = player.GetPlayerMovementState() == NPlayer::kMS_ApplyJump;
  bool dampUnderwater = false;
  if (player.GetFluidCount() != 0 &&
      !player.GetPlayerState()->HasPowerUp(CPlayerState::kIT_GravityBoost)) {
    dampUnderwater = true;
  }
  const bool noJump = player.GetPlayerMovementState() != NPlayer::kMS_ApplyJump &&
                      player.GetPlayerMovementState() != NPlayer::kMS_Jump;
  bool doStepDown = true;
  float stepDown = actor.GetStepDownHeight();
  float stepUp = actor.GetStepUpHeight();
  CMaterialList materials(kMT_NoStepLogic);
  SMoveObjectResult result;
  if (!applyJump) {
    SMovementOptions options;
    options.mSetWaterLandingForce = false;
    options.mWaterLandingForceCoefficient = 0.f;
    options.mMinimumWaterLandingForce = 0.f;
    options.mAnyZThreshold = 0.37f;
    options.mDownwardZThreshold = 0.25f;
    options.mWaterLandingVelocityReduction = 0.f;
    options.mDampForceAndMomentum = true;
    options.mAlwaysClip = false;
    options.mDisableClipForFloorOnly = noJump;
    options.mMaxCollisionCycles = 4;
    options.mMinimumTranslationDelta = 0.002f;
    options.mDampedNormalCoefficient = 0.f;
    options.mDampedDeltaCoefficient = 1.f;
    options.mFloorElasticForce = 0.f;
    options.mWallElasticConstant = 0.02f;
    options.mFloorPlaneNormal = actor.GetLastFloorPlaneNormal();
    options.mWallElasticLinear = 0.2f;
    options.mMaxPositiveVerticalVelocity = player.GetMaximumPlayerPositiveVerticalVelocity(mgr);
    if (noJump) {
      CVector3f velocity = actor.GetVelocityWR();
      velocity.SetZ(0.f);
      actor.SetVelocityWR(velocity);
      actor.ForceWR().SetZ(0.f);
      actor.MomentumWR().SetZ(0.f);
      actor.ImpulseWR().SetZ(0.f);
    }
    CPhysicsState before = actor.GetPhysicsState();
    CMaterialList moveMaterials =
        MoveObjectAnalytical(mgr, actor, dt, nearList, cache, options, result);
    CPhysicsState after = actor.GetPhysicsState();
    if (moveMaterials.GetValue() != 0ull) {
      SMovementOptions stepOptions = options;
      stepOptions.mAlwaysClip = noJump;
      stepOptions.mWallElasticConstant = 0.03f;
      CVector3f movement = before.GetTranslation();
      movement -= after.GetTranslation();
      float movementMag = movement.MagSquared();
      float quarterStepUp = 0.25f * stepUp;
      rstl::reserved_vector< CPhysicsState, 2 > states;
      rstl::reserved_vector< float, 2 > stepDeltas;
      rstl::reserved_vector< CCollisionInfo, 2 > collisions;
      rstl::reserved_vector< CMaterialList, 2 > stepMaterials;
      bool done = false;
      for (int i = 0; i < 2 && !done; ++i) {
        float stepAmount = i == 0 ? quarterStepUp : stepUp;
        actor.SetPhysicsState(before);
        CCollisionInfo upCollision;
        double useStepUp = stepAmount;
        CGameCollision::DetectCollision_Cached_Moving(
            mgr, cache, *actor.GetCollisionPrimitive(), actor.GetTransform(),
            actor.GetMaterialFilter(), nearList, CVector3f(0.f, 0.f, 1.f), upCollision, useStepUp);
        if (upCollision.IsValid()) {
          useStepUp = rstl::max_val(0.0, useStepUp - stepOptions.mMinimumTranslationDelta);
          done = true;
        }
        if (useStepUp > 0.0005f) {
          CVector3f translation = actor.GetTranslation();
          translation += CVector3f(0.f, 0.f, static_cast< float >(useStepUp));
          actor.SetTranslation(translation);
          SMoveObjectResult stepResult;
          CMaterialList stepMaterial =
              MoveObjectAnalytical(mgr, actor, dt, nearList, cache, stepOptions, stepResult);
          CCollisionInfo downCollision;
          double useStepDown = useStepUp + stepDown;
          if (useStepDown > 0.0) {
            CGameCollision::DetectCollision_Cached_Moving(
                mgr, cache, *actor.GetCollisionPrimitive(), actor.GetTransform(),
                actor.GetMaterialFilter(), nearList, CVector3f(0.f, 0.f, -1.f), downCollision,
                useStepDown);
          } else {
            useStepDown = 0.0;
          }
          float minStep = rstl::min_val(useStepDown, useStepUp);
          CVector3f step(0.f, 0.f, 1.f);
          step *= minStep;
          CVector3f endPosition = actor.GetTranslation();
          endPosition -= step;
          bool floor =
              downCollision.IsValid() && CGameCollision::CanBlock(downCollision.GetMaterialLeft(),
                                                                  downCollision.GetNormalLeft());
          CVector3f stepMovement = before.GetTranslation();
          stepMovement -= endPosition;
          float stepDelta = stepMovement.MagSquared();
          if (floor && movementMag < stepDelta) {
            useStepDown = rstl::max_val(useStepDown - 0.0005f, 0.0);
            CVector3f step(0.f, 0.f, 1.f);
            step *= static_cast< float >(useStepDown);
            CVector3f translation = actor.GetTranslation();
            translation -= step;
            actor.SetTranslation(translation);
            states.push_back(actor.GetPhysicsState());
            stepDeltas.push_back(stepDelta);
            collisions.push_back(downCollision);
            stepMaterials.push_back(stepMaterial);
          }
        }
      }
      if (states.empty()) {
        actor.SetPhysicsState(after);
        materials = moveMaterials;
      } else {
        float maxDelta = -1.e10f;
        int maxIndex = -1;
        for (int i = 0; i < states.size(); ++i) {
          if (maxDelta < stepDeltas[i]) {
            maxDelta = stepDeltas[i];
            maxIndex = i;
          }
        }
        actor.SetPhysicsState(states[maxIndex]);
        mgr.DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, kInvalidUniqueId, actor.GetUniqueId(),
                                        kSM_Landed, kSS_InvalidState));
        materials = stepMaterials[maxIndex];
        const TUniqueId id = collisions[maxIndex].GetObjectId();
        CEntity* entity = mgr.ObjectById(id);
        if (entity != nullptr) {
          result.mId = id;
          result.mCollision = collisions[maxIndex];
          if (TCastToPtr< CScriptPlatform >(entity)) {
            mgr.DeliverScriptMsg(CScriptMsg(actor.GetUniqueId(), kInvalidUniqueId,
                                            entity->GetUniqueId(), kSM_AddPlatformRider,
                                            kSS_InvalidState));
          }
        }
        CCollisionInfo& info = collisions[maxIndex];
        CGameCollision::SendMaterialMessage(mgr, info.GetMaterialLeft(), actor);
        doStepDown = false;
        actor.SetLastFloorPlaneNormal(info.GetNormalLeft());
      }
    }
  } else {
    SMovementOptions options;
    options.mSetWaterLandingForce = true;
    options.mWaterLandingForceCoefficient = dampUnderwater ? 35.f : 1.f;
    options.mMinimumWaterLandingForce = dampUnderwater ? 5.f : 0.f;
    options.mAnyZThreshold = dampUnderwater ? 0.05f : 0.37f;
    options.mDownwardZThreshold = dampUnderwater ? 0.01f : 0.25f;
    options.mWaterLandingVelocityReduction = dampUnderwater ? 0.2f : 0.f;
    options.mDampForceAndMomentum = false;
    options.mAlwaysClip = false;
    options.mDisableClipForFloorOnly = false;
    options.mMaxCollisionCycles = 4;
    options.mMinimumTranslationDelta = 0.002f;
    options.mDampedNormalCoefficient = 0.f;
    options.mDampedDeltaCoefficient = 1.f;
    options.mFloorElasticForce = 0.1f;
    options.mWallElasticConstant = 0.2f;
    options.mMaxPositiveVerticalVelocity = player.GetMaximumPlayerPositiveVerticalVelocity(mgr);
    materials = MoveObjectAnalytical(mgr, actor, dt, nearList, cache, options, result);
  }
  if (doStepDown) {
    CCollisionInfo info;
    double useStepDown = actor.GetStepDownHeight();
    float zOffset = 0.f;
    TUniqueId id = kInvalidUniqueId;
    if (useStepDown > static_cast< double >(FLT_EPSILON)) {
      CTransform4f transform = actor.GetTransform();
      CVector3f translation = transform.GetTranslation();
      translation += CVector3f(0.f, 0.f, 0.0005f);
      transform.SetTranslation(translation);
      if (!CGameCollision::DetectCollisionBoolean_Cached(mgr, cache, *actor.GetCollisionPrimitive(),
                                                         transform, actor.GetMaterialFilter(),
                                                         nearList)) {
        actor.SetTranslation(transform.GetTranslation());
        zOffset = 0.0005f;
        useStepDown += 0.0005f;
      }
      CGameCollision::DetectCollision_Cached_Moving(
          mgr, cache, *actor.GetCollisionPrimitive(), actor.GetTransform(),
          actor.GetMaterialFilter(), nearList, CVector3f(0.f, 0.f, -1.f), info, useStepDown);
      id = info.GetObjectId();
    }
    if (id != kInvalidUniqueId) {
      result.mId = id;
      result.mCollision = info;
    }
    if (!info.IsValid() ||
        !CGameCollision::CanBlock(info.GetMaterialLeft(), info.GetNormalLeft())) {
      if (zOffset > 0.f) {
        CTransform4f transform = actor.GetTransform();
        CVector3f translation = transform.GetTranslation();
        translation -= CVector3f(0.f, 0.f, zOffset);
        transform.SetTranslation(translation);
      }
      if (info.IsValid()) {
        player.SetPlayerIsSlidingOnWall(true);
        if (CVector3f::Dot(info.GetNormalLeft(), CVector3f::Up()) > 0.95f) {
          CVector3f flatVelocity = actor.GetVelocityWR();
          flatVelocity.SetZ(0.f);
          const float speed = flatVelocity.Magnitude();
          if (speed < 5.f) {
            if (speed > 0.2f) {
              flatVelocity *= 1.2f;
              actor.SetVelocityWR(flatVelocity);
            } else {
              CVector3f velocity = actor.GetTransform().GetForward();
              velocity *= 0.5f;
              actor.SetVelocityWR(velocity);
            }
          }
        }
      }
      CheckFalling(actor, mgr, dt);
      actor.SetLastFloorPlaneNormal(rstl::optional_object_null());
    } else {
      mgr.DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, kInvalidUniqueId, actor.GetUniqueId(),
                                      kSM_Landed, kSS_InvalidState));
      useStepDown = rstl::max_val(useStepDown - 0.0005f, 0.0);
      CVector3f step(0.f, 0.f, 1.f);
      step *= static_cast< float >(useStepDown);
      CVector3f translation = actor.GetTranslation();
      translation -= step;
      actor.SetTranslation(translation);
      CEntity* entity = mgr.ObjectById(id);
      if (TCastToPtr< CScriptPlatform >(entity)) {
        mgr.DeliverScriptMsg(CScriptMsg(actor.GetUniqueId(), kInvalidUniqueId,
                                        entity->GetUniqueId(), kSM_AddPlatformRider,
                                        kSS_InvalidState));
      } else {
        mgr.DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, kInvalidUniqueId, actor.GetUniqueId(),
                                        static_cast< EScriptObjectMessage >('XLSG'),
                                        kSS_InvalidState));
      }
      CGameCollision::SendMaterialMessage(mgr, info.GetMaterialLeft(), actor);
      actor.SetLastFloorPlaneNormal(info.GetNormalLeft());
    }
  }
  actor.ClearForcesAndTorques();
  if (materials.HasMaterial(kMT_Wall)) {
    player.SetPlayerHitWallDuringMove();
  }
  if (result.mId) {
    CCollisionInfoList collisionList;
    collisionList.Add(*result.mCollision);
    CGameCollision::MakeCollisionCallbacks(mgr, actor, *result.mId, collisionList);
  }
  CMotionState motion = actor.GetMotionState();
  const CMotionState lastNonColliding = actor.GetLastNonCollidingState();
  motion.SetTranslation(lastNonColliding.GetTranslation());
  motion.SetVelocity(lastNonColliding.GetVelocity());
  actor.SetLastNonCollidingState(motion);
  uchar primitiveStorage[64];
  const CCollisionPrimitive* primitive = actor.GetCollisionPrimitive();
  const CCollisionPrimitive* usePrimitive = primitive;
  void* storage = primitiveStorage;
  if (primitive->GetPrimType() == 'AABX') {
    const CCollidableAABox& box = static_cast< const CCollidableAABox& >(*primitive);
    usePrimitive = new (storage)
        CCollidableAABox(CAABox(box.GetBox().GetMinPoint() + CVector3f(0.0001f, 0.0001f, 0.0001f),
                                box.GetBox().GetMaxPoint() - CVector3f(0.0001f, 0.0001f, 0.0001f)),
                         primitive->GetMaterial());
  } else if (primitive->GetPrimType() == 'SPHR') {
    const CCollidableSphere& sphere = static_cast< const CCollidableSphere& >(*primitive);
    usePrimitive = new (storage) CCollidableSphere(
        CSphere(sphere.GetSphere().GetCenter(), sphere.GetSphere().GetRadius() - 0.0001f),
        primitive->GetMaterial());
  }
  CGameCollision::CollisionFailsafe(mgr, cache, actor, *usePrimitive, nearList, 0.f, 1, 0.f);
  if (CCollisionCache* currentCache = actor.GetCollisionCache()) {
    if (localCache) {
      *currentCache = *localCache;
    }
  }
}

CMaterialList
CGroundMovement::MoveObjectAnalytical(CStateManager& mgr, CPhysicsActor& actor, float dt,
                                      const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                      CCollisionCache& cache, const SMovementOptions& options,
                                      SMoveObjectResult& result) {
  CMaterialList materials;
  result.mProcessedCollisions = 0;
  float remainingDt = dt;
  uint cycle = 0;
  bool floorCollision = options.mFloorPlaneNormal;
  CVector3f floorNormal = floorCollision ? *options.mFloorPlaneNormal : CVector3f::Zero();

  while (remainingDt > 0.f) {
    float collideDt = remainingDt;
    CMotionState motion = actor.PredictMotion_Internal(remainingDt);
    const float translationMag = motion.GetTranslation().Magnitude();
    const CVector3f direction =
        translationMag > FLT_EPSILON
            ? CVector3d(motion.GetTranslation()).AsNormalized().AsCVector3f()
            : motion.GetTranslation();
    actor.GetCollisionPrimitive()->CalculateAABox(actor.GetPrimitiveTransform());
    double distance = translationMag;
    CCollisionInfo info;
    if (translationMag > options.mMinimumTranslationDelta) {
      CGameCollision::DetectCollision_Cached_Moving(
          mgr, cache, *actor.GetCollisionPrimitive(), actor.GetPrimitiveTransform(),
          actor.GetMaterialFilter(), nearList, direction, info, distance);
      if (info.IsValid() && info.GetObjectId() != kInvalidUniqueId) {
        result.mId = info.GetObjectId();
        result.mCollision = info;
      }
      collideDt = remainingDt * static_cast< float >(distance / translationMag);
    }

    const float moveDistance =
        rstl::max_val(static_cast< float >(distance - options.mMinimumTranslationDelta), 0.f);
    CVector3f collisionNormal = info.GetNormalLeft();
    const bool floor = CGameCollision::CanBlock(info.GetMaterialLeft(), collisionNormal);
    const bool clipCollision = options.mAlwaysClip || (options.mDisableClipForFloorOnly && !floor);
    float collisionFloorDot = 0.f;
    if (info.IsValid()) {
      ++result.mProcessedCollisions;
      if (floor) {
        materials.Add(kMT_Floor);
        floorNormal = info.GetNormalLeft();
        floorCollision = true;
      } else {
        materials.Add(kMT_Wall);
      }

      if (clipCollision) {
        if (floorCollision) {
          if (RemoveNormalComponent(floorNormal, direction, collisionNormal, collisionFloorDot)) {
            collisionNormal.Normalize();
          } else {
            RemovePositiveZComponentFromNormal(collisionNormal);
          }
        } else {
          RemovePositiveZComponentFromNormal(collisionNormal);
        }
      }
      motion = actor.PredictMotion_Internal(collideDt);
    }
    motion.SetTranslation(moveDistance * direction);
    actor.AddMotionState(motion);

    if (info.IsValid()) {
      const CVector3f oldVelocity = actor.GetVelocityWR();
      CVector3f velocity =
          oldVelocity.CanBeNormalized()
              ? CollisionDamping(oldVelocity, oldVelocity.AsNormalized(), collisionNormal,
                                 options.mDampedNormalCoefficient, options.mDampedDeltaCoefficient)
              : CVector3f::Zero();
      const float elasticForce =
          floor ? options.mFloorElasticForce
                : options.mWallElasticLinear * collisionFloorDot + options.mWallElasticConstant;
      const float dot = CVector3f::Dot(collisionNormal, velocity);
      if (dot < elasticForce) {
        velocity += (elasticForce - dot) * collisionNormal;
      }
      if (clipCollision && floorCollision && !RemoveNormalComponent(floorNormal, velocity)) {
        velocity.SetZ(0.f);
      }
      if (velocity.GetZ() > options.mMaxPositiveVerticalVelocity) {
        velocity *= options.mMaxPositiveVerticalVelocity / velocity.GetZ();
      }

      if (options.mDampForceAndMomentum) {
        const CVector3f force = actor.GetForceWR();
        if (force.CanBeNormalized()) {
          actor.SetForceWR(
              CollisionDamping(force, force.AsNormalized(), collisionNormal, 0.f, 1.f));
        }
        const CVector3f momentum = actor.GetMomentumWR();
        if (momentum.CanBeNormalized()) {
          actor.SetMomentumWR(
              CollisionDamping(momentum, momentum.AsNormalized(), collisionNormal, 0.f, 1.f));
        }
      }

      if (options.mSetWaterLandingForce && !floor) {
        if (info.GetNormalLeft().GetZ() < -0.1f && velocity.GetZ() > 0.f) {
          velocity.SetZ(0.5f * velocity.GetZ());
        }
        const float normalZ = fabsf(info.GetNormalLeft().GetZ());
        if ((normalZ > options.mDownwardZThreshold && velocity.GetZ() < 0.f) ||
            normalZ > options.mAnyZThreshold) {
          const float landingForce = rstl::max_val(options.mWaterLandingForceCoefficient * normalZ,
                                                   options.mMinimumWaterLandingForce);
          actor.SetForceWR(CVector3f(0.f, 0.f, -(1.f + landingForce) * actor.GetWeight()));
          velocity *= 1.f - options.mWaterLandingVelocityReduction;
        }
      }
      actor.SetVelocityWR(velocity);
    } else {
      CVector3f velocity = actor.GetVelocityWR();
      if (velocity.GetZ() > options.mMaxPositiveVerticalVelocity) {
        velocity *= options.mMaxPositiveVerticalVelocity / velocity.GetZ();
      }
      actor.SetVelocityWR(velocity);
    }

    actor.ClearImpulses();
    remainingDt -= collideDt;
    if (++cycle >= options.mMaxCollisionCycles) {
      break;
    }
  }

  result.mProcessedDt = dt - remainingDt;
  return materials;
}
