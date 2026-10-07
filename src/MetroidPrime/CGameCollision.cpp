#include "MetroidPrime/UserNames.hpp"
#include "MetroidPrime/CGameCollision.hpp"

#include "Collision/CCollidableAABoxSphere.hpp"
#include "Collision/CCollidableSphere.hpp"
#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CCollisionPrimitive.hpp"
#include "Collision/CInternalRayCastStructure.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Math/CLine.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/CAABoxFilter.hpp"
#include "MetroidPrime/CBallFilter.hpp"
#include "MetroidPrime/CGroundMovement.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "WorldFormat/CAreaOctTree.hpp"
#include "WorldFormat/CMetroidAreaCollider.hpp"
#include "WorldFormat/COBBTreeGroup.hpp"
#include "WorldFormat/COBBTree.hpp"
#include "WorldFormat/CCollidableOBBTreeGroup.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/TToken.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/math.hpp"

#include <float.h>

// The meanings of the two implicit static-geometry materials are not yet known.
static EMaterialTypes sStaticGeometryMaterial0 = kMT_Unknown59;                     // Guessed name.
static EMaterialTypes sStaticGeometryMaterial1 = static_cast< EMaterialTypes >(60); // Guessed name.
static const CMaterialList skStaticGeometryMaterials(sStaticGeometryMaterial0,
                                                     sStaticGeometryMaterial1);

// Guessed names: prebuilt OBB trees for the unit primitives.
static rstl::optional_object< TLockedToken< COBBTreeGroup > > sUnitCube;
static rstl::optional_object< TLockedToken< COBBTreeGroup > > sUnitSphereLow;
static rstl::optional_object< TLockedToken< COBBTreeGroup > > sUnitSphereMedium;
static rstl::optional_object< TLockedToken< COBBTreeGroup > > sUnitSphereHigh;
static uchar* sDuplicatePrimitiveBuffer; // Guessed name.

static float CollisionImpulseFiniteVsInfinite(float, float, float);
static float CollisionImpulseFiniteVsFinite(float, float, float, float);
static bool CollideCachedAABox(const CAreaCollisionCache&, const CAABox&, const CMaterialFilter&,
                               CCollisionInfoList&, const CCollisionPrimitive&);

CMotionState CPhysicsActor::GetLastNonCollidingState() const { return mLastNonCollidingState; }

void CGameCollision::InitCollision(CStateManager* mgr) {
  CCollisionPrimitive::InitBeginTypes();
  CCollisionPrimitive::InitAddType(CCollidableOBBTreeGroup::GetType());
  CCollisionPrimitive::InitEndTypes();

  CCollisionPrimitive::InitBeginColliders();
  CCollisionPrimitive::InitAddCollider(CCollidableOBBTreeGroup::SphereCollide, "CCollidableSphere",
                                       "CCollidableOBBTreeGroup");
  CCollisionPrimitive::InitAddCollider(CCollidableOBBTreeGroup::AABoxCollide, "CCollidableAABox",
                                       "CCollidableOBBTreeGroup");
  CCollisionPrimitive::InitAddBooleanCollider(CCollidableOBBTreeGroup::SphereCollideBoolean,
                                              "CCollidableSphere", "CCollidableOBBTreeGroup");
  CCollisionPrimitive::InitAddBooleanCollider(CCollidableOBBTreeGroup::AABoxCollideBoolean,
                                              "CCollidableAABox", "CCollidableOBBTreeGroup");
  CCollisionPrimitive::InitAddMovingCollider(CCollidableOBBTreeGroup::CollideMovingAABox,
                                             "CCollidableAABox", "CCollidableOBBTreeGroup");
  CCollisionPrimitive::InitAddMovingCollider(CCollidableOBBTreeGroup::CollideMovingSphere,
                                             "CCollidableSphere", "CCollidableOBBTreeGroup");
  CCollisionPrimitive::InitAddCollider(NullCollisionCollider, "CCollidableOBBTreeGroup",
                                       "CCollidableOBBTreeGroup");
  CCollisionPrimitive::InitAddBooleanCollider(NullBooleanCollider, "CCollidableOBBTreeGroup",
                                              "CCollidableOBBTreeGroup");
  CCollisionPrimitive::InitAddMovingCollider(NullMovingCollider, "CCollidableOBBTreeGroup",
                                             "CCollidableOBBTreeGroup");
  CCollisionPrimitive::InitEndColliders();

  if (mgr != nullptr ? mgr->IsMultiplayer() : true) {
    sDuplicatePrimitiveBuffer = static_cast< uchar* >(CMemory::Alloc(0xc800));
    CMetroidAreaCollider::SetDuplicatePrimitiveBuffers(
        sDuplicatePrimitiveBuffer, 0x2800, sDuplicatePrimitiveBuffer + 0x2800, 0x6000,
        sDuplicatePrimitiveBuffer + 0x6000, 0x4000);
  } else {
    sDuplicatePrimitiveBuffer = static_cast< uchar* >(CMemory::Alloc(0x42a0));
    CMetroidAreaCollider::SetDuplicatePrimitiveBuffers(
        sDuplicatePrimitiveBuffer, 0xdc0, sDuplicatePrimitiveBuffer + 0xdc0, 0x20e0,
        sDuplicatePrimitiveBuffer + 0x20e0, 0x1400);
  }

  sUnitCube = TLockedToken< COBBTreeGroup >(gpSimplePool->GetObj("UnitCube"));
  sUnitSphereLow = TLockedToken< COBBTreeGroup >(gpSimplePool->GetObj("UnitSphere_Low"));
  sUnitSphereMedium = TLockedToken< COBBTreeGroup >(gpSimplePool->GetObj("UnitSphere_Med"));
  sUnitSphereHigh = TLockedToken< COBBTreeGroup >(gpSimplePool->GetObj("UnitSphere_High"));
  COBBTree::SetPrebuiltTree((*sUnitCube.data())->GetTree(0), COBBTree::kPBT_UnitCube);
  COBBTree::SetPrebuiltTree((*sUnitSphereLow.data())->GetTree(0),
                            COBBTree::kPBT_UnitSphereLow);
  COBBTree::SetPrebuiltTree((*sUnitSphereMedium.data())->GetTree(0),
                            COBBTree::kPBT_UnitSphereMedium);
  COBBTree::SetPrebuiltTree((*sUnitSphereHigh.data())->GetTree(0),
                            COBBTree::kPBT_UnitSphereHigh);
}

void CGameCollision::UninitializeCollision() {
  sUnitCube = rstl::optional_object< TLockedToken< COBBTreeGroup > >();
  sUnitSphereLow = rstl::optional_object< TLockedToken< COBBTreeGroup > >();
  sUnitSphereMedium = rstl::optional_object< TLockedToken< COBBTreeGroup > >();
  sUnitSphereHigh = rstl::optional_object< TLockedToken< COBBTreeGroup > >();
  COBBTree::SetPrebuiltTree(nullptr, COBBTree::kPBT_UnitCube);
  COBBTree::SetPrebuiltTree(nullptr, COBBTree::kPBT_UnitSphereLow);
  COBBTree::SetPrebuiltTree(nullptr, COBBTree::kPBT_UnitSphereMedium);
  COBBTree::SetPrebuiltTree(nullptr, COBBTree::kPBT_UnitSphereHigh);
  CMetroidAreaCollider::SetDuplicatePrimitiveBuffers(nullptr, 0, nullptr, 0, nullptr, 0);
  CMemory::Free(sDuplicatePrimitiveBuffer);
  sDuplicatePrimitiveBuffer = nullptr;
  CCollisionPrimitive::Uninitialize();
}

bool CGameCollision::NullCollisionCollider(const CInternalCollisionStructure&,
                                           CCollisionInfoList&) {
  return false;
}

bool CGameCollision::NullBooleanCollider(const CInternalCollisionStructure&) { return false; }

bool CGameCollision::NullMovingCollider(const CInternalCollisionStructure&, const CVector3f&,
                                        double&, CCollisionInfo&) {
  return false;
}

CRayCastResult
CGameCollision::RayWorldIntersection(const CStateManager& mgr, TUniqueId& idOut,
                                     const CVector3f& position, const CVector3f& direction,
                                     float length, const CMaterialFilter& filter,
                                     const rstl::reserved_vector< TUniqueId, 1024 >& nearList) {
  const CRayCastResult staticResult =
      RayStaticIntersection(mgr, position, direction, length, filter);
  const CRayCastResult dynamicResult =
      RayDynamicIntersection(mgr, idOut, position, direction, length, filter, nearList);
  if (dynamicResult.IsValid()) {
    if (!staticResult.IsValid()) {
      return dynamicResult;
    }
    if (staticResult.GetTime() >= dynamicResult.GetTime()) {
      return dynamicResult;
    }
  }
  idOut = kInvalidUniqueId;
  return staticResult;
}

CRayCastResult
CGameCollision::RayDynamicIntersection(const CStateManager& mgr, TUniqueId& idOut,
                                       const CVector3f& position, const CVector3f& direction,
                                       float length, const CMaterialFilter& filter,
                                       const rstl::reserved_vector< TUniqueId, 1024 >& nearList) {
  float closest = length > 0.f ? length : 100000.f;
  CRayCastResult result;
  for (const TUniqueId* id = nearList.begin(); id != nearList.end(); ++id) {
    if (const CPhysicsActor* actor = TCastToConstPtr< CPhysicsActor >(mgr.GetObjectById(*id))) {
      const CTransform4f& xf = actor->GetPrimitiveTransform();
      const CCollisionPrimitive* prim = actor->GetCollisionPrimitive();
      const CInternalRayCastStructure ray(position, direction, closest, xf, filter);
      const CRayCastResult candidate = prim->CastRayInternal(ray);
      if (candidate.IsValid() && candidate.GetTime() < closest) {
        result = candidate;
        closest = candidate.GetTime();
        idOut = actor->GetUniqueId();
        if (closest <= FLT_EPSILON) {
          break;
        }
      }
    }
  }
  return result;
}

bool CGameCollision::RayDynamicLineOfSightTest(
    const CStateManager& mgr, const CVector3f& position, const CVector3f& direction, float length,
    const CMaterialFilter& filter, const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
    const CActor* ignoreActor) {
  const float maxDistance = length > 0.f ? length : 100000.f;
  for (const TUniqueId* id = nearList.begin(); id != nearList.end(); ++id) {
    if (const CPhysicsActor* actor = TCastToConstPtr< CPhysicsActor >(mgr.GetObjectById(*id))) {
      if (ignoreActor == nullptr || actor->GetUniqueId() != ignoreActor->GetUniqueId()) {
        const CTransform4f& xf = actor->GetPrimitiveTransform();
        const CCollisionPrimitive* prim = actor->GetCollisionPrimitive();
        const CInternalRayCastStructure ray(position, direction, maxDistance, xf, filter);
        const CRayCastResult result = prim->CastRayInternal(ray);
        if (result.IsValid()) {
          return false;
        }
      }
    }
  }
  return true;
}

bool CGameCollision::RayStaticLineOfSightTest(const CStateManager& mgr, const CVector3f& position,
                                              const CVector3f& direction, float length,
                                              const CMaterialFilter& filter) {
  const CMaterialFilter staticFilter = filter.WithImplicitMaterials(skStaticGeometryMaterials);
  if (staticFilter.GetType() == CMaterialFilter::kFT_Never) {
    return false;
  }
  const CWorld* world = mgr.GetWorld();
  const CUnitVector3f unitDir = CUnitVector3f(direction.GetX(), direction.GetY(), direction.GetZ());
  const CLine line(position, unitDir);
  const float maxDistance = length > 0.f ? length : 100000.f;
  for (CGameArea::CConstChainIterator area = world->GetChainHead(CWorld::kC_Alive);
       area != CWorld::skGlobalEnd; ++area) {
    const CAreaOctTree& tree = *area->GetPostConstructed()->mCollision;
    if (!tree.GetRootNode().LineTest(line, staticFilter, maxDistance)) {
      return false;
    }
  }
  return true;
}

bool CGameCollision::RayStaticLineOfSightTest(const CGameArea& area, const CVector3f& position,
                                              const CVector3f& direction, float length,
                                              const CMaterialFilter& filter) {
  const CMaterialFilter staticFilter = filter.WithImplicitMaterials(skStaticGeometryMaterials);
  if (staticFilter.GetType() == CMaterialFilter::kFT_Never) {
    return false;
  }
  const CUnitVector3f unitDir = CUnitVector3f(direction.GetX(), direction.GetY(), direction.GetZ());
  const CLine line(position, unitDir);
  const float maxDistance = length > 0.f ? length : 100000.f;
  if (!area.GetPostConstructed()->mCollision->GetRootNode().LineTest(line, staticFilter,
                                                                     maxDistance)) {
    return false;
  }
  return true;
}

CRayCastResult CGameCollision::RayStaticIntersection(const CStateManager& mgr,
                                                     const CVector3f& position,
                                                     const CVector3f& direction, float length,
                                                     const CMaterialFilter& filter) {
  CRayCastResult result;
  const CMaterialFilter staticFilter = filter.WithImplicitMaterials(skStaticGeometryMaterials);
  if (staticFilter.GetType() == CMaterialFilter::kFT_Never) {
    return result;
  }
  const CUnitVector3f unitDir = CUnitVector3f(direction.GetX(), direction.GetY(), direction.GetZ());
  const CLine line(position, unitDir);
  const CWorld* world = mgr.GetWorld();
  float closest = length > 0.f ? length : 100000.f;
  for (CGameArea::CConstChainIterator area = world->GetChainHead(CWorld::kC_Alive);
       area != CWorld::skGlobalEnd; ++area) {
    CAreaOctTree::SRayResult candidate;
    const CAreaOctTree& tree = *area->GetPostConstructed()->mCollision;
    tree.GetRootNode().LineTestEx(line, staticFilter, candidate, length);
    if (candidate.mSurface && (length == 0.f || !(length < candidate.mT)) && candidate.mT < closest) {
      closest = candidate.mT;
      result = CRayCastResult(candidate.mT, position + candidate.mT * direction, candidate.mPlane,
                              CMaterialList(candidate.mSurface->GetSurfaceFlags()));
    }
  }
  return result;
}

void CGameCollision::BuildAreaCollisionCache(const CStateManager& mgr, CAreaCollisionCache& cache) {
  cache.ClearCache();
  for (CGameArea::CConstChainIterator area = mgr.GetWorld()->GetChainHead(CWorld::kC_Alive);
       area != CWorld::skGlobalEnd; ++area) {
    CMetroidAreaCollider::COctreeLeafCache leaves(*area->GetPostConstructed()->mCollision,
                                                  area->GetId().Value());
    CMetroidAreaCollider::BuildOctreeLeafCache(
        area->GetPostConstructed()->mCollision->GetRootNode(), cache.GetCacheBounds(), leaves);
    cache.AddOctreeLeafCache(leaves);
  }
  IsUser(0);
}

bool CGameCollision::DetectCollisionBoolean(
    const CStateManager& mgr, const CCollisionPrimitive& primitive, const CTransform4f& transform,
    const CMaterialFilter& filter, const rstl::reserved_vector< TUniqueId, 1024 >& nearList) {
  if (filter.GetExcludeList().HasMaterial(kMT_NoStaticCollision) == false &&
      DetectStaticCollisionBoolean(mgr, primitive, transform, filter)) {
    return true;
  }
  if (DetectDynamicCollisionBoolean(primitive, transform, nearList, mgr)) {
    return true;
  }
  return false;
}

bool CGameCollision::DetectCollisionBoolean_Cached(
    const CStateManager& mgr, CAreaCollisionCache& cache, const CCollisionPrimitive& primitive,
    const CTransform4f& transform, const CMaterialFilter& filter,
    const rstl::reserved_vector< TUniqueId, 1024 >& nearList) {
  if (filter.GetExcludeList().HasMaterial(kMT_NoStaticCollision) == false &&
      DetectStaticCollisionBoolean_Cached(mgr, cache, primitive, transform, filter)) {
    return true;
  }
  if (DetectDynamicCollisionBoolean(primitive, transform, nearList, mgr)) {
    return true;
  }
  return false;
}

bool CGameCollision::DetectCollision_Cached(
    const CStateManager& mgr, CAreaCollisionCache& cache, const CCollisionPrimitive& primitive,
    const CTransform4f& transform, const CMaterialFilter& filter,
    const rstl::reserved_vector< TUniqueId, 1024 >& nearList, TUniqueId& idOut,
    CCollisionInfoList& collisions) {
  idOut = kInvalidUniqueId;
  bool hit = false;
  if (filter.GetExcludeList().HasMaterial(kMT_NoStaticCollision) == false) {
    if (DetectStaticCollision_Cached(mgr, cache, primitive, transform, filter, collisions)) {
      hit = true;
    }
  }
  TUniqueId dynamicId = kInvalidUniqueId;
  if (DetectDynamicCollision(primitive, transform, nearList, dynamicId, collisions, mgr)) {
    hit = true;
    idOut = dynamicId;
  }
  return hit;
}

bool CGameCollision::DetectCollision_Cached_Moving(
    const CStateManager& mgr, CAreaCollisionCache& cache, const CCollisionPrimitive& primitive,
    const CTransform4f& transform, const CMaterialFilter& filter,
    const rstl::reserved_vector< TUniqueId, 1024 >& nearList, const CVector3f& direction,
    TUniqueId& idOut, CCollisionInfo& collision, double& distance) {
  idOut = kInvalidUniqueId;
  bool hit = false;
  if (filter.GetExcludeList().HasMaterial(kMT_NoStaticCollision) == false) {
    if (DetectStaticCollision_Cached_Moving(mgr, cache, primitive, transform, filter, direction,
                                            collision, distance)) {
      hit = true;
    }
  }
  if (DetectDynamicCollisionMoving(primitive, transform, nearList, direction, idOut, collision,
                                   distance, mgr)) {
    hit = true;
  }
  return hit;
}

bool CGameCollision::DetectDynamicCollisionMoving(const CCollisionPrimitive& primitive,
                                                  const CTransform4f& transform,
                                                  const CPhysicsActor& actor,
                                                  const CVector3f& direction,
                                                  CCollisionInfo& collision, double& distance) {
  const CMaterialFilter& filter = CMaterialFilter::GetPassEverything();
  if (CCollisionPrimitive::CollideMoving(
          CInternalCollisionStructure::CPrimDesc(primitive, filter, transform),
          CInternalCollisionStructure::CPrimDesc(*actor.GetCollisionPrimitive(), filter,
                                                 actor.GetPrimitiveTransform()),
          direction, distance, collision)) {
    return true;
  }
  return false;
}

bool CGameCollision::DetectDynamicCollisionMoving(
    const CCollisionPrimitive& primitive, const CTransform4f& transform,
    const rstl::reserved_vector< TUniqueId, 1024 >& nearList, const CVector3f& direction,
    TUniqueId& idOut, CCollisionInfo& collision, double& distance, const CStateManager& mgr) {
  bool hit = false;
  for (const TUniqueId* id = nearList.begin(); id != nearList.end(); ++id) {
    const CPhysicsActor* actor = TCastToConstPtr< CPhysicsActor >(mgr.GetObjectById(*id));
    double candidateDistance = distance;
    CCollisionInfo candidate;
    if (actor != nullptr) {
      if (DetectDynamicCollisionMoving(primitive, transform, *actor, direction, candidate,
                                       candidateDistance) &&
          candidateDistance < distance) {
        hit = true;
        collision = candidate;
        distance = candidateDistance;
        idOut = actor->GetUniqueId();
      }
    }
  }
  return hit;
}

bool CGameCollision::DetectDynamicCollisionBoolean(const CCollisionPrimitive& primitive,
                                                   const CTransform4f& transform,
                                                   const CPhysicsActor& actor) {
  const CMaterialFilter& filter = CMaterialFilter::GetPassEverything();
  if (CCollisionPrimitive::CollideBoolean(
          CInternalCollisionStructure::CPrimDesc(primitive, filter, transform),
          CInternalCollisionStructure::CPrimDesc(*actor.GetCollisionPrimitive(), filter,
                                                 actor.GetPrimitiveTransform()))) {
    return true;
  }
  return false;
}

bool CGameCollision::DetectDynamicCollisionBoolean(
    const CCollisionPrimitive& primitive, const CTransform4f& transform,
    const rstl::reserved_vector< TUniqueId, 1024 >& nearList, const CStateManager& mgr) {
  for (const TUniqueId* id = nearList.begin(); id != nearList.end(); ++id) {
    if (const CPhysicsActor* actor = TCastToConstPtr< CPhysicsActor >(mgr.GetObjectById(*id))) {
      if (DetectDynamicCollisionBoolean(primitive, transform, *actor)) {
        return true;
      }
    }
  }
  return false;
}

bool CGameCollision::DetectDynamicCollision(const CCollisionPrimitive& primitive,
                                            const CTransform4f& transform,
                                            const CPhysicsActor& actor,
                                            CCollisionInfoList& collisions) {
  const CMaterialFilter& filter = CMaterialFilter::GetPassEverything();
  if (CCollisionPrimitive::Collide(
          CInternalCollisionStructure::CPrimDesc(primitive, filter, transform),
          CInternalCollisionStructure::CPrimDesc(*actor.GetCollisionPrimitive(), filter,
                                                 actor.GetPrimitiveTransform()),
          collisions)) {
    return true;
  }
  return false;
}

bool CGameCollision::DetectDynamicCollision(
    const CCollisionPrimitive& primitive, const CTransform4f& transform,
    const rstl::reserved_vector< TUniqueId, 1024 >& nearList, TUniqueId& idOut,
    CCollisionInfoList& collisions, const CStateManager& mgr) {
  for (const TUniqueId* id = nearList.begin(); id != nearList.end(); ++id) {
    if (const CPhysicsActor* actor = TCastToConstPtr< CPhysicsActor >(mgr.GetObjectById(*id))) {
      if (DetectDynamicCollision(primitive, transform, *actor, collisions)) {
        idOut = actor->GetUniqueId();
        return true;
      }
    }
  }
  idOut = kInvalidUniqueId;
  return false;
}

// Guessed name for the legacy cache's AABox contact-collection helper.
static bool CollideCachedAABox(const CAreaCollisionCache& cache, const CAABox& bounds,
                               const CMaterialFilter& filter, CCollisionInfoList& collisions,
                               const CCollisionPrimitive& primitive) {
  bool hit = false;
  for (int i = 0; i < cache.GetNumCaches(); ++i) {
    if (CMetroidAreaCollider::AABoxCollisionCheck_Cached(
            cache.GetOctreeLeafCache(i), bounds, filter, primitive.GetMaterial(), collisions) ==
        true) {
      hit = true;
    }
  }
  return hit;
}

bool CGameCollision::DetectStaticCollision_Cached(
    const CStateManager& mgr, CAreaCollisionCache& cache, const CCollisionPrimitive& primitive,
    const CTransform4f& transform, const CMaterialFilter& filter, CCollisionInfoList& collisions) {
  const CMaterialFilter staticFilter = filter.WithImplicitMaterials(skStaticGeometryMaterials);
  if (staticFilter.GetType() == CMaterialFilter::kFT_Never) {
    return false;
  }
  if (primitive.GetPrimType() == 'OBTG') {
    return false;
  }
  bool hit = false;
  const CAABox bounds = primitive.CalculateAABox(transform);
  if (!bounds.Inside(cache.GetCacheBounds())) {
    const CVector3f margin(0.2f, 0.2f, 0.2f);
    CAABox expanded(bounds.GetMinPoint() - margin, bounds.GetMaxPoint() + margin);
    expanded.AccumulateBounds(cache.GetCacheBounds().GetMinPoint());
    expanded.AccumulateBounds(cache.GetCacheBounds().GetMaxPoint());
    cache.SetCacheBounds(expanded);
    BuildAreaCollisionCache(mgr, cache);
    IsUser(0);
  }
  if (cache.HasCacheOverflowed()) {
    return DetectStaticCollision(mgr, primitive, transform, staticFilter, collisions);
  }
  if (primitive.GetPrimType() == 'AABX') {
    hit = CollideCachedAABox(cache, bounds, staticFilter, collisions, primitive);
  } else if (primitive.GetPrimType() == 'SPHR') {
    const CSphere sphere = static_cast< const CCollidableSphere& >(primitive).Transform(transform);
    for (int i = 0; i < static_cast< int >(cache.GetNumCaches()); ++i) {
      if (CMetroidAreaCollider::SphereCollisionCheck_Cached(cache.GetOctreeLeafCache(i), bounds,
                                                            sphere, primitive.GetMaterial(),
                                                            staticFilter, collisions)) {
        hit = true;
      }
    }
  } else if (primitive.GetPrimType() == 'ABSH') {
    const CCollidableAABoxSphere& compound =
        static_cast< const CCollidableAABoxSphere& >(primitive);
    if (DetectStaticCollision_Cached(mgr, cache, compound.GetCollidableAABox(), transform,
                                     staticFilter, collisions)) {
      hit = true;
    }
    if (DetectStaticCollision_Cached(mgr, cache, compound.GetCollidableSphere(), transform,
                                     staticFilter, collisions)) {
      hit = true;
    }
  }
  return hit;
}

bool CGameCollision::DetectStaticCollision(const CStateManager& mgr,
                                           const CCollisionPrimitive& primitive,
                                           const CTransform4f& transform,
                                           const CMaterialFilter& filter,
                                           CCollisionInfoList& collisions) {
  const CMaterialFilter staticFilter = filter.WithImplicitMaterials(skStaticGeometryMaterials);
  if (staticFilter.GetType() == CMaterialFilter::kFT_Never) {
    return false;
  }
  if (primitive.GetPrimType() == 'OBTG') {
    return false;
  }
  bool hit = false;
  const CWorld* world = mgr.GetWorld();
  if (primitive.GetPrimType() == 'AABX') {
    const CAABox bounds = primitive.CalculateAABox(transform);
    for (CGameArea::CConstChainIterator area = world->GetChainHead(CWorld::kC_Alive);
         area != CWorld::skGlobalEnd; ++area) {
      if (CMetroidAreaCollider::AABoxCollisionCheck(*area->GetPostConstructed()->mCollision, bounds,
                                                    staticFilter, primitive.GetMaterial(),
                                                    collisions)) {
        hit = true;
      }
    }
  } else if (primitive.GetPrimType() == 'SPHR') {
    const CAABox bounds = primitive.CalculateAABox(transform);
    const CSphere sphere = static_cast< const CCollidableSphere& >(primitive).Transform(transform);
    for (CGameArea::CConstChainIterator area = world->GetChainHead(CWorld::kC_Alive);
         area != CWorld::skGlobalEnd; ++area) {
      if (CMetroidAreaCollider::SphereCollisionCheck(*area->GetPostConstructed()->mCollision,
                                                     bounds, sphere, primitive.GetMaterial(),
                                                     staticFilter, collisions)) {
        hit = true;
      }
    }
  } else if (primitive.GetPrimType() == 'ABSH') {
    const CCollidableAABoxSphere& compound =
        static_cast< const CCollidableAABoxSphere& >(primitive);
    if (DetectStaticCollision(mgr, compound.GetCollidableAABox(), transform, staticFilter,
                              collisions)) {
      hit = true;
    }
    if (DetectStaticCollision(mgr, compound.GetCollidableSphere(), transform, staticFilter,
                              collisions)) {
      hit = true;
    }
  }
  return hit;
}

bool CGameCollision::DetectStaticCollisionBoolean_Cached(const CStateManager& mgr,
                                                         CAreaCollisionCache& cache,
                                                         const CCollisionPrimitive& primitive,
                                                         const CTransform4f& transform,
                                                         const CMaterialFilter& filter) {
  const CMaterialFilter staticFilter = filter.WithImplicitMaterials(skStaticGeometryMaterials);
  if (staticFilter.GetType() == CMaterialFilter::kFT_Never) {
    return false;
  }
  if (primitive.GetPrimType() == 'OBTG') {
    return false;
  }
  bool hit = false;
  const CAABox bounds = primitive.CalculateAABox(transform);
  if (!bounds.Inside(cache.GetCacheBounds())) {
    const CVector3f margin(0.2f, 0.2f, 0.2f);
    CAABox expanded(bounds.GetMinPoint() - margin, bounds.GetMaxPoint() + margin);
    expanded.AccumulateBounds(cache.GetCacheBounds().GetMinPoint());
    expanded.AccumulateBounds(cache.GetCacheBounds().GetMaxPoint());
    cache.SetCacheBounds(expanded);
    BuildAreaCollisionCache(mgr, cache);
    IsUser(0);
  }
  if (cache.HasCacheOverflowed()) {
    return DetectStaticCollisionBoolean(mgr, primitive, transform, staticFilter);
  }
  if (primitive.GetPrimType() == 'AABX') {
    for (int i = 0; i < static_cast< int >(cache.GetNumCaches()); ++i) {
      if (CMetroidAreaCollider::AABoxCollisionCheckBoolean_Cached(cache.GetOctreeLeafCache(i),
                                                                  bounds, staticFilter)) {
        hit = true;
        break;
      }
    }
  } else if (primitive.GetPrimType() == 'SPHR') {
    const CSphere sphere = static_cast< const CCollidableSphere& >(primitive).Transform(transform);
    for (int i = 0; i < static_cast< int >(cache.GetNumCaches()); ++i) {
      if (CMetroidAreaCollider::SphereCollisionCheckBoolean_Cached(cache.GetOctreeLeafCache(i),
                                                                   bounds, sphere, staticFilter)) {
        hit = true;
        break;
      }
    }
  } else if (primitive.GetPrimType() == 'ABSH') {
    const CCollidableAABoxSphere& compound =
        static_cast< const CCollidableAABoxSphere& >(primitive);
    if (DetectStaticCollisionBoolean_Cached(mgr, cache, compound.GetCollidableAABox(), transform,
                                            staticFilter)) {
      hit = true;
    } else if (DetectStaticCollisionBoolean_Cached(mgr, cache, compound.GetCollidableSphere(),
                                                   transform, staticFilter)) {
      hit = true;
    }
  }
  return hit;
}

bool CGameCollision::DetectStaticCollisionBoolean(const CStateManager& mgr,
                                                  const CCollisionPrimitive& primitive,
                                                  const CTransform4f& transform,
                                                  const CMaterialFilter& filter) {
  const CMaterialFilter staticFilter = filter.WithImplicitMaterials(skStaticGeometryMaterials);
  if (staticFilter.GetType() == CMaterialFilter::kFT_Never) {
    return false;
  }
  if (primitive.GetPrimType() == 'OBTG') {
    return false;
  }
  const CWorld* world = mgr.GetWorld();
  if (primitive.GetPrimType() == 'AABX') {
    const CAABox bounds = primitive.CalculateAABox(transform);
    for (CGameArea::CConstChainIterator area = world->GetChainHead(CWorld::kC_Alive);
         area != CWorld::skGlobalEnd; ++area) {
      if (CMetroidAreaCollider::AABoxCollisionCheckBoolean(*area->GetPostConstructed()->mCollision,
                                                           bounds, staticFilter)) {
        return true;
      }
    }
  } else if (primitive.GetPrimType() == 'SPHR') {
    const CAABox bounds = primitive.CalculateAABox(transform);
    const CSphere sphere = static_cast< const CCollidableSphere& >(primitive).Transform(transform);
    for (CGameArea::CConstChainIterator area = world->GetChainHead(CWorld::kC_Alive);
         area != CWorld::skGlobalEnd; ++area) {
      if (CMetroidAreaCollider::SphereCollisionCheckBoolean(*area->GetPostConstructed()->mCollision,
                                                            bounds, sphere, staticFilter)) {
        return true;
      }
    }
  } else if (primitive.GetPrimType() == 'ABSH') {
    const CCollidableAABoxSphere& compound =
        static_cast< const CCollidableAABoxSphere& >(primitive);
    if (DetectStaticCollisionBoolean(mgr, compound.GetCollidableAABox(), transform,
                                     staticFilter)) {
      return true;
    }
    if (DetectStaticCollisionBoolean(mgr, compound.GetCollidableSphere(), transform,
                                     staticFilter)) {
      return true;
    }
  }
  return false;
}

bool CGameCollision::DetectStaticCollision_Cached_Moving(
    const CStateManager& mgr, CAreaCollisionCache& cache, const CCollisionPrimitive& primitive,
    const CTransform4f& transform, const CMaterialFilter& filter, const CVector3f& direction,
    CCollisionInfo& collision, double& distance) {
  const CMaterialFilter staticFilter = filter.WithImplicitMaterials(skStaticGeometryMaterials);
  if (staticFilter.GetType() == CMaterialFilter::kFT_Never) {
    return false;
  }
  if (primitive.GetPrimType() == 'OBTG') {
    return false;
  }
  const CVector3f displacement = float(distance) * direction;
  const CAABox bounds = primitive.CalculateAABox(transform);
  CAABox sweptBounds(bounds);
  sweptBounds.AccumulateBounds(bounds.GetMinPoint() + displacement);
  sweptBounds.AccumulateBounds(bounds.GetMaxPoint() + displacement);
  if (!sweptBounds.Inside(cache.GetCacheBounds())) {
    const CVector3f margin(0.2f, 0.2f, 0.2f);
    CAABox expanded(sweptBounds.GetMinPoint() - margin, sweptBounds.GetMaxPoint() + margin);
    expanded.AccumulateBounds(cache.GetCacheBounds().GetMinPoint());
    expanded.AccumulateBounds(cache.GetCacheBounds().GetMaxPoint());
    cache.SetCacheBounds(expanded);
    BuildAreaCollisionCache(mgr, cache);
    IsUser(0);
  }

  if (primitive.GetPrimType() == 'AABX') {
    for (int i = 0; i < static_cast< int >(cache.GetNumCaches()); ++i) {
      CCollisionInfo candidate;
      double candidateDistance = distance;
      const float maxDistance = distance;
      if (CMetroidAreaCollider::MovingAABoxCollisionCheck_Cached(
              cache.GetOctreeLeafCache(i), bounds, staticFilter, CMaterialList(kMT_Unknown59),
              direction, maxDistance, candidate, candidateDistance) &&
          candidateDistance < distance) {
        collision = candidate;
        distance = float(candidateDistance);
      }
    }
  } else if (primitive.GetPrimType() == 'SPHR') {
    const CSphere& sphere = static_cast< const CCollidableSphere& >(primitive).GetSphere();
    for (int i = 0; i < static_cast< int >(cache.GetNumCaches()); ++i) {
      CCollisionInfo candidate;
      double candidateDistance = distance;
      const float maxDistance = distance;
      if (CMetroidAreaCollider::MovingSphereCollisionCheck_Cached(
              cache.GetOctreeLeafCache(i), bounds,
              CSphere(transform * sphere.GetCenter(), sphere.GetRadius()), staticFilter,
              CMaterialList(kMT_Unknown59), direction, maxDistance, candidate,
              candidateDistance) &&
          candidateDistance < distance) {
        collision = candidate;
        distance = float(candidateDistance);
      }
    }
  }
  return collision.IsValid();
}

void CGameCollision::MakeCollisionCallbacks(CStateManager& mgr, CPhysicsActor& actor,
                                            const TUniqueId& id,
                                            const CCollisionInfoList& collisions) {
  actor.CollidedWith(id, collisions, mgr);
  if (id != kInvalidUniqueId) {
    if (CPhysicsActor* other = TCastToPtr< CPhysicsActor >(mgr.ObjectById(id))) {
      CCollisionInfoList swapped(collisions);
      swapped.Swap(0);
      // The original passes the unswapped list despite constructing the swapped copy.
      other->CollidedWith(actor.GetUniqueId(), collisions, mgr);
    }
  }
}

void CGameCollision::SendScriptMessages(CStateManager& mgr, CActor& actor, CActor* other,
                                        const CCollisionInfoList& collisions) {
  CMaterialList materials;
  bool hasFloor = false;
  bool hasPlatform = false;
  for (const CCollisionInfo* it = collisions.Begin(); it != collisions.End(); ++it) {
    materials.Add(it->GetMaterialLeft());
  }
  for (int i = 0; i < collisions.GetCount(); ++i) {
    const CCollisionInfo& collision = collisions[i];
    if (IsFloor(collision.GetMaterialLeft(), collision.GetNormalLeft())) {
      hasFloor = true;
      if (collision.GetMaterialLeft().HasMaterial(kMT_Platform)) {
        hasPlatform = true;
      }
    }
  }
  SendMaterialMessage(mgr, materials, actor);
  if (hasFloor) {
    mgr.DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, actor.GetUniqueId(), kSM_Landed));
    if (hasPlatform) {
      if (CScriptPlatform* platform = TCastToPtr< CScriptPlatform >(other)) {
        mgr.DeliverScriptMsg(CScriptMsg(actor.GetUniqueId(), platform->GetUniqueId(),
                                        static_cast< EScriptObjectMessage >('XONP')));
      }
    } else {
      mgr.DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, actor.GetUniqueId(),
                                      static_cast< EScriptObjectMessage >('XLSG')));
    }
  } else if (other != nullptr) {
    if (CScriptPlatform* platform = TCastToPtr< CScriptPlatform >(&actor)) {
      for (int i = 0; i < collisions.GetCount(); ++i) {
        const CCollisionInfo& collision = collisions[i];
        if (IsFloor(collision.GetMaterialRight(), collision.GetNormalRight()) &&
            collision.GetMaterialRight().HasMaterial(kMT_Platform)) {
          hasPlatform = true;
          break;
        }
      }
      if (hasPlatform) {
        mgr.DeliverScriptMsg(CScriptMsg(other->GetUniqueId(), platform->GetUniqueId(),
                                        static_cast< EScriptObjectMessage >('XONP')));
      }
    }
  }
}

void CGameCollision::SendMaterialMessage(CStateManager& mgr, const CMaterialList&, CActor& actor) {
  // Echoes always sends the normal-surface message here.
  mgr.DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, actor.GetUniqueId(),
                                  static_cast< EScriptObjectMessage >('XOND')));
}

void CGameCollision::ShowCollisionResults(CCollisionInfoList&, const CColor&) {}

float CGameCollision::GetCoefficientOfRestitution(const CCollisionInfo&) { return 0.f; }

static float CollisionImpulseFiniteVsFinite(float mass, float otherMass, float velocity,
                                            float restitution) {
  return -(1.f + restitution) * velocity / (1.f / mass + 1.f / otherMass);
}

static float CollisionImpulseFiniteVsInfinite(float mass, float velocity, float restitution) {
  return mass * (-(1.f + restitution) * velocity);
}

bool CGameCollision::IsFloor(const CMaterialList& material, const CVector3f& normal) {
  if (material.HasMaterial(kMT_Floor)) {
    return true;
  }
  return normal.GetZ() > 0.85f;
}

bool CGameCollision::CanBlock(const CMaterialList& material, const CVector3f& normal) {
  if (material.HasMaterial(kMT_Character) && !material.HasMaterial(kMT_SolidCharacter)) {
    return false;
  }
  if (material.HasMaterial(kMT_NoPlayerCollision)) {
    return false;
  }
  if (material.HasMaterial(kMT_Floor)) {
    return true;
  }
  return normal.GetZ() > 0.85f;
}

float CGameCollision::GetMinExtentForCollisionPrimitive(const CCollisionPrimitive& primitive) {
  if (primitive.GetPrimType() == 'SPHR') {
    return 2.f * static_cast< const CCollidableSphere& >(primitive).GetSphere().GetRadius();
  }
  if (primitive.GetPrimType() == 'AABX') {
    const CAABox& bounds = static_cast< const CCollidableAABox& >(primitive).GetBox();
    const CVector3f& extent = bounds.GetMaxPoint() - bounds.GetMinPoint();
    return rstl::min_val(rstl::min_val(extent.GetX(), extent.GetY()), extent.GetZ());
  }
  if (primitive.GetPrimType() == 'ABSH') {
    const CCollidableAABoxSphere& compound =
        static_cast< const CCollidableAABoxSphere& >(primitive);
    return rstl::min_val(GetMinExtentForCollisionPrimitive(compound.GetCollidableAABox()),
                         GetMinExtentForCollisionPrimitive(compound.GetCollidableSphere()));
  }
  return 1.f;
}

void CGameCollision::ResolveCollisions(CPhysicsActor& actor, CPhysicsActor* other,
                                       const CCollisionInfoList& collisions) {
  for (int i = 0; i < collisions.GetCount(); ++i) {
    const CCollisionInfo collision = collisions[i];
    const float restitution =
        GetCoefficientOfRestitution(collision) + actor.GetCoefficientOfRestitutionModifier();
    if (other != nullptr) {
      CollideWithDynamicBodyNoRot(actor, *other, collision, restitution, false);
    } else {
      CollideWithStaticBodyNoRot(actor, collision.GetMaterialLeft(), collision.GetMaterialRight(),
                                 CUnitVector3f(collision.GetNormalLeft()), restitution, false);
    }
  }
}

void CGameCollision::CollideWithDynamicBodyNoRot(CPhysicsActor& actor, CPhysicsActor& other,
                                                 const CCollisionInfo& collision, float restitution,
                                                 bool flattenNormal) {
  CVector3f normal = collision.GetNormalLeft();
  if (flattenNormal) {
    normal.SetZ(0.f);
  }
  const float mass = actor.GetMass();
  const float otherMass = other.GetMass();
  const float normalVelocity = CVector3f::Dot(GetActorRelativeVelocities(&actor, &other), normal);
  const float maxSpeed =
      rstl::max_val(actor.GetMaximumCollisionVelocity(), actor.GetVelocityWR().Magnitude());
  const float otherMaxSpeed =
      rstl::max_val(other.GetMaximumCollisionVelocity(), other.GetVelocityWR().Magnitude());
  const bool immovable = actor.GetMaterialList().HasMaterial(kMT_Immovable) || mass == 0.f;
  const bool otherImmovable =
      other.GetMaterialList().HasMaterial(kMT_Immovable) || otherMass == 0.f;

  if (normalVelocity < -0.0001f) {
    if (immovable && otherImmovable) {
      actor.SetVelocityWR(CVector3f::Zero());
      other.SetVelocityWR(CVector3f::Zero());
    } else if (immovable) {
      const float impulse =
          CollisionImpulseFiniteVsInfinite(otherMass, normalVelocity, restitution);
      other.ApplyImpulseWR(-impulse * normal, CAxisAngle::Identity());
    } else if (otherImmovable) {
      const float impulse = CollisionImpulseFiniteVsInfinite(mass, normalVelocity, restitution);
      actor.ApplyImpulseWR(impulse * normal, CAxisAngle::Identity());
    } else {
      const float impulse =
          CollisionImpulseFiniteVsFinite(mass, otherMass, normalVelocity, restitution);
      actor.ApplyImpulseWR(impulse * normal, CAxisAngle::Identity());
      other.ApplyImpulseWR(-impulse * normal, CAxisAngle::Identity());
    }
    actor.UseCollisionImpulses();
    other.UseCollisionImpulses();
  } else if (normalVelocity < 0.1f) {
    if (!immovable) {
      actor.ApplyImpulseWR((0.05f * mass) * normal, CAxisAngle::Identity());
      actor.UseCollisionImpulses();
    }
    if (!otherImmovable) {
      other.ApplyImpulseWR((-0.05f * otherMass) * normal, CAxisAngle::Identity());
      other.UseCollisionImpulses();
    }
  }
  const float speed = actor.GetVelocityWR().Magnitude();
  if (speed > maxSpeed) {
    actor.SetVelocityWR(maxSpeed * (actor.GetVelocityWR() / speed));
  }
  const float otherSpeed = other.GetVelocityWR().Magnitude();
  if (otherSpeed > otherMaxSpeed) {
    other.SetVelocityWR(otherMaxSpeed * (other.GetVelocityWR() / otherSpeed));
  }
}

void CGameCollision::CollideWithStaticBodyNoRot(CPhysicsActor& actor, const CMaterialList& material,
                                                const CMaterialList& otherMaterial,
                                                const CUnitVector3f& normal, float restitution,
                                                bool flattenNormal) {
  CVector3f collisionNormal(normal);
  if (flattenNormal && material.HasMaterial(kMT_Player) && !otherMaterial.HasMaterial(kMT_Floor)) {
    collisionNormal.SetZ(0.f);
  }
  if (!collisionNormal.CanBeNormalized()) {
    return;
  }
  collisionNormal.Normalize();
  const float normalVelocity = CVector3f::Dot(actor.GetVelocityWR(), collisionNormal);
  if (normalVelocity < -0.0001f) {
    const float impulse =
        CollisionImpulseFiniteVsInfinite(actor.GetMass(), normalVelocity, restitution);
    actor.ApplyImpulseWR(impulse * collisionNormal, CAxisAngle::Identity());
    actor.UseCollisionImpulses();
  } else {
    const float speed = CVector3f(actor.GetVelocityWR()).Magnitude();
    const float cosAngle = speed > 0.001f ? normalVelocity / speed : 0.f;
    if (normalVelocity < 0.001f || cosAngle < 0.0008f) {
      actor.ApplyImpulseWR((0.05f * actor.GetMass()) * collisionNormal, CAxisAngle::Identity());
      actor.UseCollisionImpulses();
    }
  }
}

void CGameCollision::Move(CStateManager& mgr, CPhysicsActor& actor, float dt,
                          const rstl::reserved_vector< TUniqueId, 1024 >* colliderList) {
  if (!actor.GetMovable()) {
    return;
  }
  if (actor.GetMaterialList().HasMaterial(kMT_GroundCollider) || actor.WillMove(mgr)) {
    if (actor.GetAngularEnabled()) {
      actor.AddMotionState(actor.PredictAngularMotion(dt));
    }
    actor.UseCollisionImpulses();
    if (actor.GetMaterialList().HasMaterial(kMT_Unknown59)) {
      if (actor.GetMaterialList().HasMaterial(kMT_Player)) {
        if (!gpMain->IsMaxSpeed() || mgr.GetPlayer(0)->ShouldSampleFailsafe(mgr)) {
          MovePlayer(mgr, actor, dt, colliderList);
        }
      } else if (actor.GetMaterialList().HasMaterial(kMT_GroundCollider)) {
        CGroundMovement::MoveGroundCollider(mgr, actor, dt, colliderList);
      } else {
        MoveAndCollide(mgr, actor, dt, CAABoxFilter(actor), colliderList);
      }
    } else {
      CMotionState state = actor.PredictMotion_Internal(dt);
      actor.AddMotionState(state);
      actor.ClearForcesAndTorques();
    }
    mgr.UpdateActorInSortedLists(&actor);
  }
}

void CGameCollision::MovePlayer(CStateManager& mgr, CPhysicsActor& actor, float dt,
                                const rstl::reserved_vector< TUniqueId, 1024 >* colliderList) {
  actor.SetAngularEnabled(true);
  actor.AddMotionState(actor.PredictAngularMotion(dt));
  if (actor.IsStandardCollider()) {
    MoveAndCollide(mgr, actor, dt, CBallFilter(actor), colliderList);
  } else if (actor.GetMaterialList().HasMaterial(kMT_GroundCollider)) {
    CGroundMovement::MoveGroundCollider_New(mgr, actor, dt, colliderList);
  } else {
    MoveAndCollide(mgr, actor, dt, CBallFilter(actor), colliderList);
  }
  actor.SetAngularEnabled(false);
}

void CGameCollision::CollisionFailsafe(const CStateManager& mgr, CAreaCollisionCache& cache,
                                       CPhysicsActor& actor, const CCollisionPrimitive& primitive,
                                       const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                       float dtFraction, uint failsafeTicks, float impulseScale) {
  actor.MoveCollisionPrimitive(CVector3f::Zero());
  if (dtFraction > 0.5f) {
    actor.SetNumTicksPartialUpdate(actor.GetNumTicksPartialUpdate() + 1);
  }

  if (actor.GetNumTicksPartialUpdate() > 1 ||
      DetectCollisionBoolean_Cached(mgr, cache, primitive, actor.GetPrimitiveTransform(),
                                    actor.GetMaterialFilter(), nearList)) {
    actor.SetNumTicksPartialUpdate(0);
    actor.SetNumTicksStuck(actor.GetNumTicksStuck() + 1);
    if (actor.GetNumTicksStuck() < failsafeTicks) {
      return;
    }

    const CMotionState& oldState = actor.GetMotionState();
    const CMotionState& lastState = actor.GetLastNonCollidingState();
    actor.SetMotionState(CMotionState(lastState.GetTranslation(), lastState.GetOrientation(),
                                      lastState.GetVelocity(), lastState.GetAngularMomentum()));
    if (!DetectCollisionBoolean_Cached(mgr, cache, primitive, actor.GetPrimitiveTransform(),
                                       actor.GetMaterialFilter(), nearList)) {
      actor.SetLastNonCollidingState(
          CMotionState(lastState.GetTranslation(), lastState.GetOrientation(),
                       0.5f * lastState.GetVelocity(), lastState.GetAngularMomentum() * 0.5f));
      actor.SetNumTicksStuck(0);
    } else {
      CVector3f recoveryImpulse = CVector3f::Zero();
      if (impulseScale) {
        TUniqueId id = kInvalidUniqueId;
        CCollisionInfoList collisions;
        DetectCollision_Cached(mgr, cache, primitive, actor.GetPrimitiveTransform(),
                               actor.GetMaterialFilter(), nearList, id, collisions);
        if (collisions.GetCount() != 0) {
          const CVector3f normal = collisions.GetCombinedNormalLeft();
          recoveryImpulse = actor.GetMass() * (impulseScale * normal);
        }
      }

      actor.SetMotionState(CMotionState(oldState.GetTranslation(), oldState.GetOrientation(),
                                        oldState.GetVelocity(), oldState.GetAngularMomentum()));
      const rstl::optional_object< CVector3f > displacement =
          FindNonIntersectingVector(mgr, actor, primitive, rstl::optional_object< CVector3f >());
      if (displacement.valid()) {
        actor.SetMotionState(
            CMotionState(oldState.GetTranslation() + *displacement, oldState.GetOrientation(),
                         oldState.GetVelocity() + recoveryImpulse, oldState.GetAngularMomentum()));
        actor.SetLastNonCollidingState(actor.GetMotionState());
      } else {
        actor.SetLastNonCollidingState(
            CMotionState(lastState.GetTranslation(), lastState.GetOrientation(),
                         0.5f * lastState.GetVelocity() + recoveryImpulse,
                         lastState.GetAngularMomentum() * 0.5f));
      }
    }
  } else {
    actor.SetLastNonCollidingState(actor.GetMotionState());
    actor.SetNumTicksStuck(0);
  }
}

rstl::optional_object< CVector3f >
CGameCollision::FindNonIntersectingVector(const CStateManager& mgr, CPhysicsActor& actor,
                                          const CCollisionPrimitive& prim,
                                          const rstl::optional_object< CVector3f >& center) {
  CTransform4f xf = actor.GetPrimitiveTransform();
  const CVector3f origOrigin = xf.GetTranslation();
  CAABox aabb = prim.CalculateAABox(xf);
  const CVector3f centerPoint = center ? center.data() : aabb.GetCenterPoint();
  aabb = CAABox(aabb.GetMinPoint() - CVector3f(5.f, 5.f, 5.f),
                aabb.GetMaxPoint() + CVector3f(5.f, 5.f, 5.f));
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildColliderList(nearList, actor, aabb);
  for (int i = 2; i < 1000.f; i += i / 2) {
    const float pos = 0.005f * i;
    const float neg = -pos;
    for (int j = 0; j < 26; ++j) {
      CVector3f vec(CVector3f::Zero());
      switch (j) {
      case 0:
        vec = CVector3f(0.f, pos, 0.f);
        break;
      case 1:
        vec = CVector3f(0.f, neg, 0.f);
        break;
      case 2:
        vec = CVector3f(pos, 0.f, 0.f);
        break;
      case 3:
        vec = CVector3f(neg, 0.f, 0.f);
        break;
      case 4:
        vec = CVector3f(0.f, 0.f, pos);
        break;
      case 5:
        vec = CVector3f(0.f, 0.f, neg);
        break;
      case 6:
        vec = CVector3f(0.f, pos, pos);
        break;
      case 7:
        vec = CVector3f(0.f, neg, neg);
        break;
      case 8:
        vec = CVector3f(0.f, neg, pos);
        break;
      case 9:
        vec = CVector3f(0.f, pos, neg);
        break;
      case 10:
        vec = CVector3f(pos, 0.f, pos);
        break;
      case 11:
        vec = CVector3f(neg, 0.f, neg);
        break;
      case 12:
        vec = CVector3f(neg, 0.f, pos);
        break;
      case 13:
        vec = CVector3f(pos, 0.f, neg);
        break;
      case 14:
        vec = CVector3f(pos, pos, 0.f);
        break;
      case 15:
        vec = CVector3f(neg, neg, 0.f);
        break;
      case 16:
        vec = CVector3f(neg, pos, 0.f);
        break;
      case 17:
        vec = CVector3f(pos, neg, 0.f);
        break;
      case 18:
        vec = CVector3f(pos, pos, pos);
        break;
      case 19:
        vec = CVector3f(neg, pos, pos);
        break;
      case 20:
        vec = CVector3f(pos, neg, pos);
        break;
      case 21:
        vec = CVector3f(neg, neg, pos);
        break;
      case 22:
        vec = CVector3f(pos, pos, neg);
        break;
      case 23:
        vec = CVector3f(neg, pos, neg);
        break;
      case 24:
        vec = CVector3f(pos, neg, neg);
        break;
      case 25:
        vec = CVector3f(neg, neg, neg);
        break;
      }
      if (mgr.GetWorld()->GetArea(mgr.GetNextAreaId())->GetAABB().PointInside(origOrigin + vec)) {
        if (mgr.RayCollideWorld(centerPoint, centerPoint + vec, nearList,
                                CMaterialFilter::skPassEverything, &actor)) {
          xf.SetTranslation(origOrigin + vec);
          if (!DetectCollisionBoolean(mgr, prim, xf, actor.GetMaterialFilter(), nearList)) {
            return vec;
          }
        }
      }
    }
  }
  return rstl::optional_object_null();
}

CVector3f CGameCollision::GetActorRelativeVelocities(const CPhysicsActor* actor,
                                                     const CPhysicsActor* other) {
  CVector3f velocity = actor->GetVelocityWR();
  if (other != nullptr) {
    const CScriptPlatform* platform = TCastToConstPtr< CScriptPlatform >(other);
    bool rider = false;
    if (platform != nullptr) {
      rider = platform->IsRider(actor->GetUniqueId());
    }
    if (!rider) {
      velocity -= other->GetVelocityWR();
    }
  }
  return velocity;
}

void CGameCollision::PushActorAwayFromWalls(CStateManager& mgr, CPhysicsActor& actor, float dt,
                                            float height, float distance, float acceleration,
                                            int iterations, float radius) {
  const CVector3f actorPosition = actor.GetTranslation();
  const CVector3f center = actorPosition + CVector3f(0.f, 0.f, height);
  const float cacheRadius = 1.2f * radius;
  const CVector3f extent(distance + cacheRadius, distance + cacheRadius, cacheRadius);
  CAreaCollisionCache cache(CAABox(center - extent, center + extent));
  BuildAreaCollisionCache(mgr, cache);
  const CSphere sphere(center, radius);
  const CMaterialFilter filter = CMaterialFilter::MakeExclude(CMaterialList(kMT_Floor));
  if (DetectStaticCollisionBoolean_Cached(mgr, cache,
                                          CCollidableSphere(sphere, CMaterialList(kMT_Unknown59)),
                                          CTransform4f::Identity(), filter)) {
    return;
  }

  CVector3f correction = CVector3f::Zero();
  const float angleStep = M_2PIF / float(iterations);
  for (int i = 0; i < iterations; ++i) {
    const float angle = angleStep * float(i);
    const CVector3f direction(CMath::SlowSineR(angle), CMath::SlowCosineR(angle), 0.f);
    double collisionDistance = distance;
    CCollisionInfo collision;
    if (cache.HasCacheOverflowed()) {
      cache.ClearCache();
      CAABox bounds(center, center);
      bounds.AccumulateBounds(actorPosition + distance * direction);
      const CVector3f radiusVector(radius, radius, radius);
      // Both corners use the minimum point in the original.
      cache.SetCacheBounds(
          CAABox(bounds.GetMinPoint() - radiusVector, bounds.GetMinPoint() + radiusVector));
      BuildAreaCollisionCache(mgr, cache);
    }
    if (DetectStaticCollision_Cached_Moving(
            mgr, cache, CCollidableSphere(sphere, CMaterialList(kMT_Unknown59)),
            CTransform4f::Identity(), filter, direction, collision, collisionDistance)) {
      const float fraction = float(distance - collisionDistance) / distance / float(iterations);
      correction -= fraction * direction;
    }
  }
  actor.SetVelocityWR(actor.GetVelocityWR() + dt * (acceleration * correction));
}

// Guessed name; shared source helper for the legacy cache's expanding query bounds.
