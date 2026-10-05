#include "MetroidPrime/CGameCollision.hpp"

#include "Collision/CCollidableAABoxSphere.hpp"
#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ICollisionFilter.hpp"
#include "MetroidPrime/UserNames.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "WorldFormat/CCollidableOBBTree.hpp"
#include "WorldFormat/CCollidableOBBTreeGroup.hpp"
#include "WorldFormat/CCollisionCache.hpp"
#include "WorldFormat/CMetroidAreaCollider.hpp"
#include "WorldFormat/COBBTree.hpp"
#include "rstl/math.hpp"

namespace {
const CMaterialList kImplicitGeometryMaterials(kMT_Unknown59, kMT_Unknown60);

// Guessed helper names and source-local placement.
TUniqueId FirstCollisionObjectId(const CCollisionInfoList& collisions) {
  for (int i = 0; i < collisions.GetCount(); ++i) {
    const TUniqueId id = collisions[i].GetObjectId();
    if (id != kInvalidUniqueId) {
      return id;
    }
  }
  return kInvalidUniqueId;
}

CTransform4f MakeAABoxCacheTransform(const CPhysicsActor& actor, const CCollidableAABox& box) {
  const CAABox bounds = box.CalculateAABox(actor.GetPrimitiveTransform());
  CTransform4f transform = CTransform4f::Scale(bounds.GetMaxPoint() - bounds.GetMinPoint());
  transform.SetTranslation(bounds.GetCenterPoint());
  return transform;
}

CTransform4f MakeSphereCacheTransform(const CPhysicsActor& actor, const CCollidableSphere& sphere) {
  const CVector3f center = actor.GetPrimitiveTransform() * sphere.GetSphere().GetCenter();
  CTransform4f transform = CTransform4f::Scale(sphere.GetSphere().GetRadius());
  transform.SetTranslation(center);
  return transform;
}
} // namespace

void CGameCollision::BuildCollisionCache(const CStateManager& mgr, CCollisionCache& cache,
                                         const CMaterialFilter& filter) {
  cache.Reset();
  const CWorld& world = *mgr.GetWorld();
  for (CGameArea::CConstChainIterator area = world.GetChainHead(CWorld::kC_Alive);
       area != CWorld::skGlobalEnd; ++area) {
    CMetroidAreaCollider::BuildCollisionCache(area->GetPostConstructed()->mCollision->GetRootNode(),
                                              cache);
  }

  if (cache.GetDynamicGeometryMode() != 0) {
    rstl::reserved_vector< TUniqueId, 1024 > nearList;
    const CActor* owner = TCastToConstPtr< CActor >(mgr.GetObjectById(cache.GetOwnerId()));
    if (owner) {
      mgr.BuildColliderList(nearList, *owner, cache.GetBounds());
    } else {
      mgr.BuildNearList(nearList, cache.GetBounds(), filter, nullptr);
    }
    for (int i = 0; i < nearList.size(); ++i) {
      CacheActorGeometry(mgr, cache, mgr.GetObjectById(nearList[i]));
    }
  }
}

void CGameCollision::BuildCollisionCache(const CStateManager& mgr, CCollisionCache& cache,
                                         rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                         ECacheUpdatePolicy policy) {
  const CWorld& world = *mgr.GetWorld();
  for (CGameArea::CConstChainIterator area = world.GetChainHead(CWorld::kC_Alive);
       area != CWorld::skGlobalEnd; ++area) {
    CMetroidAreaCollider::BuildCollisionCache(area->GetPostConstructed()->mCollision->GetRootNode(),
                                              cache);
  }

  if (cache.GetDynamicGeometryMode() != 0) {
    for (rstl::reserved_vector< TUniqueId, 1024 >::iterator id = nearList.begin();
         id != nearList.end();) {
      if (CacheActorGeometry(mgr, cache, mgr.GetObjectById(*id)) &&
          policy == kCUP_RemoveCachedNearListIds) {
        id = nearList.erase(id);
      } else {
        ++id;
      }
    }
  }
}

void CGameCollision::UpdateCollisionCache(const CStateManager& mgr, CCollisionCache& cache,
                                          rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                          ECacheUpdatePolicy policy) {
  uchar status[1024] = {};
  CCollisionCacheIterator iterator;

  for (;;) {
    const uint result = cache.SkipGeometry(iterator);
    if (result == uint(-1)) {
      for (int i = 0; i < nearList.size(); ++i) {
        const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(nearList[i]));
        if (actor && status[i] != 1 && CacheActorGeometry(mgr, cache, actor)) {
          status[i] = 1;
        }
      }

      if (policy == kCUP_RemoveCachedNearListIds) {
        int statusIndex = 0;
        for (rstl::reserved_vector< TUniqueId, 1024 >::iterator id = nearList.begin();
             id != nearList.end(); ++statusIndex) {
          if (status[statusIndex] == 1) {
            id = nearList.erase(id);
          } else {
            ++id;
          }
        }
      }
      return;
    }

    const short objectId = iterator.GetObjectId();
    if (objectId == kInvalidUniqueId.value) {
      continue;
    }

    bool removeGeometry = true;
    for (int i = 0; i < nearList.size(); ++i) {
      if (nearList[i].value != objectId) {
        continue;
      }

      const CPhysicsActor* actor =
          TCastToConstPtr< CPhysicsActor >(mgr.GetObjectById(nearList[i]));
      if (!actor) {
        continue;
      }

      const CCollisionPrimitive& primitive = *actor->GetCollisionPrimitive();
      const CCollisionPrimitiveData* geometry = nullptr;
      CTransform4f transform = CTransform4f::Identity();
      switch (primitive.GetPrimType()) {
      case 'OBTG':
        geometry = static_cast< const CCollidableOBBTreeGroup& >(primitive).GetOBBTree(0);
        transform = actor->GetPrimitiveTransform();
        break;
      case 'AABX':
        if (cache.GetDynamicGeometryMode() != 2) {
          geometry = &iterator.GetGeometry();
          transform = MakeAABoxCacheTransform(
              *actor, static_cast< const CCollidableAABox& >(primitive));
        }
        break;
      case 'SPHR':
        if (cache.GetDynamicGeometryMode() != 2) {
          transform = MakeSphereCacheTransform(
              *actor, static_cast< const CCollidableSphere& >(primitive));
          geometry = &iterator.GetGeometry();
        }
        break;
      }

      if (geometry) {
        const bool matches = iterator.MatchesGeometry(
            objectId, geometry, transform, actor->GetMaterialList().GetValue());
        status[i] = matches ? 1 : 2;
        if (matches) {
          removeGeometry = false;
        }
        break;
      }
    }

    if (removeGeometry) {
      cache.RemoveGeometry(iterator);
    }
  }
}

bool CGameCollision::CacheActorGeometry(const CStateManager& mgr, CCollisionCache& cache,
                                        const CEntity* entity) {
  if (cache.GetDynamicGeometryMode() == 0) {
    return false;
  }

  const CPhysicsActor* actor = TCastToConstPtr< CPhysicsActor >(entity);
  if (!actor) {
    return true;
  }

  const CCollisionPrimitive& primitive = *actor->GetCollisionPrimitive();
  if (primitive.GetPrimType() == 'OBTG') {
    static_cast< const CCollidableOBBTreeGroup& >(primitive).CacheTree(
        cache, actor->GetPrimitiveTransform(), entity->GetUniqueId().value,
        actor->GetMaterialList().GetValue());
    return true;
  }
  if (cache.GetDynamicGeometryMode() != 2) {
    return false;
  }

  if (primitive.GetPrimType() == 'AABX') {
    const CTransform4f transform =
        MakeAABoxCacheTransform(*actor, static_cast< const CCollidableAABox& >(primitive));
    CCollidableOBBTree::CacheAABox(cache, transform, entity->GetUniqueId().value,
                                   actor->GetMaterialList().GetValue());
    return true;
  }
  if (primitive.GetPrimType() == 'SPHR') {
    const CTransform4f transform =
        MakeSphereCacheTransform(*actor, static_cast< const CCollidableSphere& >(primitive));
    CCollidableOBBTree::CacheSphere(cache, transform, entity->GetUniqueId().value,
                                    actor->GetMaterialList().GetValue());
    return true;
  }
  return false;
}

bool CGameCollision::DetectCollisionBoolean_Cached(
    const CStateManager& mgr, CCollisionCache& cache, const CCollisionPrimitive& primitive,
    const CTransform4f& transform, const CMaterialFilter& filter,
    const rstl::reserved_vector< TUniqueId, 1024 >& nearList) {
  if (!filter.GetExcludeList().HasMaterial(kMT_NoStaticCollision) &&
      DetectStaticCollisionBoolean_Cached(mgr, cache, primitive, transform, filter)) {
    return true;
  }
  return DetectDynamicCollisionBoolean(primitive, transform, nearList, mgr);
}

bool CGameCollision::DetectCollision_Cached(
    const CStateManager& mgr, CCollisionCache& cache, const CCollisionPrimitive& primitive,
    const CTransform4f& transform, const CMaterialFilter& filter,
    const rstl::reserved_vector< TUniqueId, 1024 >& nearList, TUniqueId& idOut,
    CCollisionInfoList& collisions) {
  idOut = kInvalidUniqueId;
  TUniqueId dynamicId = kInvalidUniqueId;
  bool hit = false;
  if (!filter.GetExcludeList().HasMaterial(kMT_NoStaticCollision) &&
      DetectStaticCollision_Cached(mgr, cache, primitive, transform, filter, collisions)) {
    hit = true;
    idOut = FirstCollisionObjectId(collisions);
  }
  if (DetectDynamicCollision(primitive, transform, nearList, dynamicId, collisions, mgr)) {
    hit = true;
    idOut = dynamicId;
  }
  return hit;
}

bool CGameCollision::DetectCollision_Cached_Moving(
    const CStateManager& mgr, CCollisionCache& cache, const CCollisionPrimitive& primitive,
    const CTransform4f& transform, const CMaterialFilter& filter,
    const rstl::reserved_vector< TUniqueId, 1024 >& nearList, const CVector3f& direction,
    CCollisionInfo& collision, double& distance) {
  bool hit = false;
  if (!filter.GetExcludeList().HasMaterial(kMT_NoStaticCollision)) {
    hit = DetectStaticCollision_Cached_Moving(mgr, cache, primitive, transform, filter, direction,
                                              collision, distance);
  }
  TUniqueId dynamicId = kInvalidUniqueId;
  if (DetectDynamicCollisionMoving(primitive, transform, nearList, direction, dynamicId, collision,
                                   distance, mgr)) {
    hit = true;
    collision.SetObjectId(dynamicId);
  }
  return hit;
}

bool CGameCollision::DetectStaticCollision_Cached(const CStateManager& mgr, CCollisionCache& cache,
                                                  const CCollisionPrimitive& primitive,
                                                  const CTransform4f& transform,
                                                  const CMaterialFilter& filter,
                                                  CCollisionInfoList& collisions) {
  const CMaterialFilter geometryFilter = filter.WithImplicitMaterials(kImplicitGeometryMaterials);
  if (geometryFilter.GetType() == CMaterialFilter::kFT_Never || primitive.GetPrimType() == 'OBTG') {
    return false;
  }

  const CAABox bounds = primitive.CalculateAABox(transform);
  if (!bounds.Inside(cache.GetBounds())) {
    const CVector3f margin(0.2f, 0.2f, 0.2f);
    CAABox grownBounds(bounds.GetMinPoint() - margin, bounds.GetMaxPoint() + margin);
    grownBounds.Include(cache.GetBounds());
    cache.SetBounds(grownBounds);
    BuildCollisionCache(mgr, cache, CMaterialFilter());
  }

  switch (primitive.GetPrimType()) {
  case 'AABX':
    return CMetroidAreaCollider::AABoxCollisionCheck_Cached(cache, bounds, geometryFilter,
                                                            primitive.GetMaterial(), collisions);
  case 'SPHR':
    return CMetroidAreaCollider::SphereCollisionCheck_Cached(
        cache, bounds, static_cast< const CCollidableSphere& >(primitive).Transform(transform),
        primitive.GetMaterial(), geometryFilter, collisions);
  case 'ABSH': {
    const CCollidableAABoxSphere& compound =
        static_cast< const CCollidableAABoxSphere& >(primitive);
    bool hit = DetectStaticCollision_Cached(mgr, cache, compound.GetCollidableAABox(), transform,
                                            geometryFilter, collisions);
    if (DetectStaticCollision_Cached(mgr, cache, compound.GetCollidableSphere(), transform,
                                     geometryFilter, collisions)) {
      hit = true;
    }
    return hit;
  }
  default:
    return false;
  }
}

bool CGameCollision::DetectStaticCollisionBoolean_Cached(const CStateManager& mgr,
                                                         CCollisionCache& cache,
                                                         const CCollisionPrimitive& primitive,
                                                         const CTransform4f& transform,
                                                         const CMaterialFilter& filter) {
  const CMaterialFilter geometryFilter = filter.WithImplicitMaterials(kImplicitGeometryMaterials);
  if (geometryFilter.GetType() == CMaterialFilter::kFT_Never || primitive.GetPrimType() == 'OBTG') {
    return false;
  }

  const CAABox bounds = primitive.CalculateAABox(transform);
  if (!bounds.Inside(cache.GetBounds())) {
    const CVector3f margin(0.2f, 0.2f, 0.2f);
    CAABox grownBounds(bounds.GetMinPoint() - margin, bounds.GetMaxPoint() + margin);
    grownBounds.Include(cache.GetBounds());
    cache.SetBounds(grownBounds);
    BuildCollisionCache(mgr, cache, CMaterialFilter());
  }

  switch (primitive.GetPrimType()) {
  case 'AABX':
    return CMetroidAreaCollider::AABoxCollisionCheckBoolean_Cached(cache, bounds, geometryFilter);
  case 'SPHR':
    return CMetroidAreaCollider::SphereCollisionCheckBoolean_Cached(
        cache, bounds, static_cast< const CCollidableSphere& >(primitive).Transform(transform),
        geometryFilter);
  case 'ABSH': {
    const CCollidableAABoxSphere& compound =
        static_cast< const CCollidableAABoxSphere& >(primitive);
    return DetectStaticCollisionBoolean_Cached(mgr, cache, compound.GetCollidableAABox(), transform,
                                               geometryFilter) ||
           DetectStaticCollisionBoolean_Cached(mgr, cache, compound.GetCollidableSphere(),
                                               transform, geometryFilter);
  }
  default:
    return false;
  }
}

bool CGameCollision::DetectStaticCollision_Cached_Moving(
    const CStateManager& mgr, CCollisionCache& cache, const CCollisionPrimitive& primitive,
    const CTransform4f& transform, const CMaterialFilter& filter, const CVector3f& direction,
    CCollisionInfo& collision, double& distance) {
  const CMaterialFilter geometryFilter = filter.WithImplicitMaterials(kImplicitGeometryMaterials);
  if (geometryFilter.GetType() == CMaterialFilter::kFT_Never || primitive.GetPrimType() == 'OBTG') {
    return false;
  }

  const CVector3f displacement = static_cast< float >(distance) * direction;
  const CAABox bounds = primitive.CalculateAABox(transform);
  CAABox sweptBounds = bounds;
  sweptBounds.AccumulateBounds(bounds.GetMinPoint() + displacement);
  sweptBounds.AccumulateBounds(bounds.GetMaxPoint() + displacement);
  if (!sweptBounds.Inside(cache.GetBounds())) {
    const CVector3f margin(0.2f, 0.2f, 0.2f);
    CAABox grownBounds(sweptBounds.GetMinPoint() - margin, sweptBounds.GetMaxPoint() + margin);
    grownBounds.Include(cache.GetBounds());
    cache.SetBounds(grownBounds);
    BuildCollisionCache(mgr, cache, CMaterialFilter());
  }

  if (primitive.GetPrimType() == 'AABX') {
    CCollisionInfo result;
    double resultDistance = distance;
    if (CMetroidAreaCollider::MovingAABoxCollisionCheck_Cached(
            cache, bounds, geometryFilter, CMaterialList(kMT_Unknown59), direction,
            static_cast< float >(distance), result, resultDistance) &&
        resultDistance < distance) {
      collision = result;
      distance = static_cast< float >(resultDistance);
    }
  }
  return collision.IsValid();
}

void CGameCollision::CollisionFailsafe(const CStateManager& mgr, CCollisionCache& cache,
                                       CPhysicsActor& actor, const CCollisionPrimitive& primitive,
                                       const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                       float dtFraction, uint failsafeTicks, float impulseScale) {
  actor.MoveCollisionPrimitive(CVector3f::Zero());
  if (dtFraction > 0.5f) {
    actor.SetNumTicksPartialUpdate(actor.GetNumTicksPartialUpdate() + 1);
  }

  if (actor.GetNumTicksPartialUpdate() <= 1 &&
      !DetectCollisionBoolean_Cached(mgr, cache, primitive, actor.GetPrimitiveTransform(),
                                     actor.GetMaterialFilter(), nearList)) {
    actor.SetLastNonCollidingState(actor.GetMotionState());
    actor.SetNumTicksStuck(0);
    return;
  }

  actor.SetNumTicksPartialUpdate(0);
  actor.SetNumTicksStuck(actor.GetNumTicksStuck() + 1);
  if (actor.GetNumTicksStuck() < failsafeTicks) {
    return;
  }

  const CMotionState oldState = actor.GetMotionState();
  const CMotionState lastState = actor.GetLastNonCollidingState();
  actor.SetMotionState(lastState);
  if (!DetectCollisionBoolean_Cached(mgr, cache, primitive, actor.GetPrimitiveTransform(),
                                     actor.GetMaterialFilter(), nearList)) {
    actor.SetLastNonCollidingState(
        CMotionState(lastState.GetTranslation(), lastState.GetOrientation(),
                     0.5f * lastState.GetVelocity(),
                     0.5f * lastState.GetAngularMomentum()));
    actor.SetNumTicksStuck(0);
    return;
  }

  CVector3f recoveryImpulse = CVector3f::Zero();
  if (impulseScale != 0.f) {
    CCollisionInfoList collisions;
    TUniqueId id = kInvalidUniqueId;
    DetectCollision_Cached(mgr, cache, primitive, actor.GetPrimitiveTransform(),
                           actor.GetMaterialFilter(), nearList, id, collisions);
    if (collisions.GetCount() != 0) {
      CVector3f normal = CVector3f::Zero();
      for (int i = 0; i < collisions.GetCount(); ++i) {
        normal += collisions[i].GetNormalLeft();
      }
      if (normal.IsNonZero()) {
        normal = normal.AsNormalized();
      }
      recoveryImpulse = actor.GetMass() * impulseScale * normal;
    }
  }

  actor.SetMotionState(oldState);
  const rstl::optional_object< CVector3f > displacement =
      FindNonIntersectingVector(mgr, actor, primitive);
  if (displacement.valid()) {
    actor.SetMotionState(
        CMotionState(oldState.GetTranslation() + *displacement, oldState.GetOrientation(),
                     oldState.GetVelocity() + recoveryImpulse, oldState.GetAngularMomentum()));
    actor.SetLastNonCollidingState(actor.GetMotionState());
  } else {
    actor.SetLastNonCollidingState(
        CMotionState(lastState.GetTranslation(), lastState.GetOrientation(),
                     0.5f * lastState.GetVelocity() + recoveryImpulse,
                     0.5f * lastState.GetAngularMomentum()));
  }
}

rstl::optional_object< CVector3f >
CGameCollision::FindNonIntersectingVector(const CStateManager& mgr, CPhysicsActor& actor,
                                          const CCollisionPrimitive& primitive) {
  CTransform4f transform = actor.GetPrimitiveTransform();
  const CVector3f origin = transform.GetTranslation();
  const CAABox bounds = primitive.CalculateAABox(transform);
  const CVector3f center = primitive.CalculateAABox(transform).GetCenterPoint();
  const CVector3f margin(5.f, 5.f, 5.f);
  const CAABox searchBounds(bounds.GetMinPoint() - margin, bounds.GetMaxPoint() + margin);
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildColliderList(nearList, actor, searchBounds);

  for (int i = 2; i < 1000; i += i / 2) {
    const float step = 0.005f * static_cast< float >(i);
    for (int j = 0; j < 26; ++j) {
      CVector3f displacement = CVector3f::Zero();
      switch (j) {
      case 0:
        displacement = CVector3f(0.f, step, 0.f);
        break;
      case 1:
        displacement = CVector3f(0.f, -step, 0.f);
        break;
      case 2:
        displacement = CVector3f(step, 0.f, 0.f);
        break;
      case 3:
        displacement = CVector3f(-step, 0.f, 0.f);
        break;
      case 4:
        displacement = CVector3f(0.f, 0.f, step);
        break;
      case 5:
        displacement = CVector3f(0.f, 0.f, -step);
        break;
      case 6:
        displacement = CVector3f(0.f, step, step);
        break;
      case 7:
        displacement = CVector3f(0.f, -step, -step);
        break;
      case 8:
        displacement = CVector3f(0.f, -step, step);
        break;
      case 9:
        displacement = CVector3f(0.f, step, -step);
        break;
      case 10:
        displacement = CVector3f(step, 0.f, step);
        break;
      case 11:
        displacement = CVector3f(-step, 0.f, -step);
        break;
      case 12:
        displacement = CVector3f(-step, 0.f, step);
        break;
      case 13:
        displacement = CVector3f(step, 0.f, -step);
        break;
      case 14:
        displacement = CVector3f(step, step, 0.f);
        break;
      case 15:
        displacement = CVector3f(-step, -step, 0.f);
        break;
      case 16:
        displacement = CVector3f(-step, step, 0.f);
        break;
      case 17:
        displacement = CVector3f(step, -step, 0.f);
        break;
      case 18:
        displacement = CVector3f(step, step, step);
        break;
      case 19:
        displacement = CVector3f(-step, step, step);
        break;
      case 20:
        displacement = CVector3f(step, -step, step);
        break;
      case 21:
        displacement = CVector3f(-step, -step, step);
        break;
      case 22:
        displacement = CVector3f(step, step, -step);
        break;
      case 23:
        displacement = CVector3f(-step, step, -step);
        break;
      case 24:
        displacement = CVector3f(step, -step, -step);
        break;
      case 25:
        displacement = CVector3f(-step, -step, -step);
        break;
      }

      const CVector3f position = origin + displacement;
      if (mgr.GetWorld()->GetAreaAlways(mgr.GetNextAreaId()).GetAABB().PointInside(position) &&
          mgr.RayCollideWorld(center, center + displacement, nearList,
                              CMaterialFilter::GetPassEverything(), &actor)) {
        transform.SetTranslation(position);
        if (!DetectCollisionBoolean(mgr, primitive, transform, actor.GetMaterialFilter(),
                                    nearList)) {
          return rstl::optional_object< CVector3f >(displacement);
        }
      }
    }
  }
  return rstl::optional_object< CVector3f >();
}

void CGameCollision::MoveAndCollide(CStateManager& mgr, CPhysicsActor& actor, float dt,
                                    const ICollisionFilter& collisionFilter,
                                    const rstl::reserved_vector< TUniqueId, 1024 >* nearList) {
  bool hadCollision = false;
  bool resolvedCollision = false;
  const bool isPlayer = actor.GetMaterialList().HasMaterial(kMT_Player);
  uint iteration = 0;

  float remainingDt = dt;
  float maxStepDt = dt;
  float stepDt = dt;
  CCollisionInfoList collisions;
  CMotionState motion = actor.PredictMotion_Internal(dt);
  const float translationMag = motion.GetTranslation().Magnitude();
  float minMoveMag = rstl::max_val(translationMag / (5.f * actor.GetCollisionAccuracyModifier()),
                                   0.0005f / actor.GetCollisionAccuracyModifier());
  const float collisionMinMag = 0.001f / actor.GetCollisionAccuracyModifier();
  const CMaterialFilter& materialFilter = actor.GetMaterialFilter();

  rstl::reserved_vector< TUniqueId, 1024 > nearbyActors;
  const CAABox motionVolume = actor.GetMotionVolume(dt);
  if (nearList) {
    nearbyActors = *nearList;
  } else {
    mgr.BuildColliderList(nearbyActors, actor, motionVolume);
  }

  rstl::optional_object< CCollisionCache > localCache;
  CCollisionCache* cached = actor.GetCollisionCache();
  CCollisionCache* cachePtr = cached;
  const bool skipStaticCache = actor.GetCollisionPrimitive()->GetPrimType() == 'OBTG' ||
                               materialFilter.GetExcludeList().HasMaterial(kMT_NoStaticCollision);
  if (!skipStaticCache) {
    const CAABox primitiveBounds =
        actor.GetCollisionPrimitive()->CalculateAABox(actor.GetPrimitiveTransform());
    const CVector3f center = primitiveBounds.GetCenterPoint();
    const float minExtent =
        0.5f * GetMinExtentForCollisionPrimitive(*actor.GetCollisionPrimitive());
    if (translationMag > minExtent) {
      TUniqueId id = kInvalidUniqueId;
      const CVector3f direction = motion.GetTranslation() / translationMag;
      const CRayCastResult hit =
          mgr.RayWorldIntersection(id, center, direction, translationMag, materialFilter,
                                   nearbyActors);
      if (hit.IsValid()) {
        stepDt = dt * (hit.GetTime() / translationMag);
        motion = actor.PredictMotion_Internal(stepDt);
        maxStepDt = minExtent * (dt / translationMag);
        minMoveMag = rstl::min_val(minExtent, minMoveMag);
      }
    }

    if (cached == nullptr || !motionVolume.Inside(cached->GetBounds())) {
      const float padding = cached != nullptr ? 0.5f : 0.f;
      const CVector3f margin(padding, padding, padding);
      const CAABox cacheBounds(motionVolume.GetMinPoint() - margin,
                               motionVolume.GetMaxPoint() + margin);
      cachePtr = &localCache.emplace(cacheBounds,
                                     cached != nullptr ? cached->GetDynamicGeometryMode() : 1,
                                     0, ushort(0xffff));
      BuildCollisionCache(mgr, *cachePtr, nearbyActors, kCUP_RemoveCachedNearListIds);
    } else {
      UpdateCollisionCache(mgr, *cachePtr, nearbyActors, kCUP_RemoveCachedNearListIds);
    }
  } else if (cached == nullptr) {
    const CVector3f lower(-1.e9f, -1.e9f, -1.e9f);
    const CVector3f upper(1.e9f, 1.e9f, 1.e9f);
    cachePtr = &localCache.emplace(CAABox(lower, upper), 0, 2, ushort(0xffff));
  }
  CCollisionCache& cache = *cachePtr;

  float currentDt = stepDt;
  bool continueLoop = true;
  while (continueLoop) {
    actor.MoveCollisionPrimitive(motion.GetTranslation());
    if (DetectCollisionBoolean_Cached(mgr, cache, *actor.GetCollisionPrimitive(),
                                      actor.GetPrimitiveTransform(), materialFilter,
                                      nearbyActors)) {
      hadCollision = true;
      if (motion.GetTranslation().Magnitude() < minMoveMag) {
        resolvedCollision = true;
        collisions.Clear();
        TUniqueId id = kInvalidUniqueId;
        DetectCollision_Cached(mgr, cache, *actor.GetCollisionPrimitive(),
                               actor.GetPrimitiveTransform(), materialFilter, nearbyActors, id,
                               collisions);
        CPhysicsActor* otherActor = TCastToPtr< CPhysicsActor >(mgr.ObjectById(id));
        actor.MoveCollisionPrimitive(CVector3f::Zero());

        CCollisionInfoList filtered;
        CCollisionInfoList backfaced;
        CollisionUtil::FilterOutBackfaces(GetActorRelativeVelocities(&actor, otherActor),
                                          collisions, backfaced);
        if (backfaced.GetCount() != 0) {
          collisionFilter.Filter(backfaced, filtered);
          if (filtered.GetCount() == 0 && isPlayer) {
            const CMotionState lastState = actor.GetLastNonCollidingState();
            actor.SetMotionState(
                CMotionState(lastState.GetTranslation(), lastState.GetOrientation(),
                             lastState.GetVelocity() * 0.5f,
                             lastState.GetAngularMomentum() * 0.5f));
          }
        }

        MakeCollisionCallbacks(mgr, actor, id, filtered);
        if (IsUser(0)) {
          ShowCollisionResults(filtered, CColor::Grey());
        }
        SendScriptMessages(mgr, actor, otherActor, filtered);
        ResolveCollisions(actor, otherActor, filtered);

        remainingDt -= stepDt;
        currentDt = rstl::min_val(remainingDt, maxStepDt);
        stepDt = currentDt;
      } else {
        currentDt *= 0.5f;
        stepDt *= 0.5f;
      }
    } else {
      actor.AddMotionState(motion);
      remainingDt -= stepDt;
      stepDt = currentDt;
      actor.ClearImpulses();
      actor.MoveCollisionPrimitive(CVector3f::Zero());
    }

    ++iteration;
    continueLoop = remainingDt > 0.f &&
                   (motion.GetTranslation().Magnitude() > collisionMinMag || !resolvedCollision) &&
                   iteration <= 1000;
    if (continueLoop) {
      motion = actor.PredictMotion_Internal(stepDt);
    }
  }

  const float remainingFraction = remainingDt / dt;
  if (!hadCollision && !actor.GetMaterialList().HasMaterial(kMT_GroundCollider)) {
    mgr.DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, kInvalidUniqueId, actor.GetUniqueId(),
                                    kSM_Falling, kSS_InvalidState));
  }
  if (isPlayer) {
    CollisionFailsafe(mgr, cache, actor, *actor.GetCollisionPrimitive(), nearbyActors,
                      remainingFraction, 2, 4.f);
  }
  actor.ClearForcesAndTorques();
  actor.MoveCollisionPrimitive(CVector3f::Zero());
  if (cached != nullptr && localCache && !skipStaticCache) {
    *cached = *localCache;
  }
}
