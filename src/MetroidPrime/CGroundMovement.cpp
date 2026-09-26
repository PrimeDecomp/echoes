#include "MetroidPrime/CGroundMovement.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CCollisionPrimitive.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Math/CVector3d.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "rstl/math.hpp"

#include <float.h>
#include <math.h>

void CGroundMovement::CheckFalling(CPhysicsActor& actor, CStateManager& mgr, float dt) {
  // TODO: Check the alive-area chain and send Echoes's falling/on-floor messages.
}

void CGroundMovement::MoveGroundCollider(
    CStateManager& mgr, CPhysicsActor& actor, float dt,
    const rstl::reserved_vector< TUniqueId, 1024 >* colliderList) {
  // TODO: Restore the legacy area-cache path, including the stationary-ground early exit.
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
      mgr.SendScriptMsg_fn_80037100(CScriptMsg(kInvalidUniqueId, kInvalidUniqueId,
                                               actor.GetUniqueId(), kSM_LandOnNotFloor,
                                               kSS_InvalidState));
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
    mgr.SendScriptMsg_fn_80037100(CScriptMsg(kInvalidUniqueId, kInvalidUniqueId,
                                             actor.GetUniqueId(), kSM_LandOnNotFloor,
                                             kSS_InvalidState));
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
  // TODO: Restore iterative movement, floor filtering and static/dynamic collision response.
  return false;
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
  // TODO: Restore player stepping, packed-cache reuse, Gravity Boost and wall-slide handling.
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
