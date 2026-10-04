#ifndef _CGAMECOLLISION
#define _CGAMECOLLISION

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"

class CActor;
class CAreaCollisionCache;
class CColor;
class CEntity;
class CCollisionCache; // Guessed name; distinct from CAreaCollisionCache.
class CCollisionInfo;
class CCollisionInfoList;
class CCollisionPrimitive;
class CGameArea;
class CInternalCollisionStructure;
class CMaterialFilter;
class CMaterialList;
class CPhysicsActor;
class CRayCastResult;
class CStateManager;
class CTransform4f;
class CUnitVector3f;
class ICollisionFilter;

class CGameCollision {
public:
  static void InitCollision(CStateManager* mgr);
  // Guessed name.
  static void UninitializeCollision();
  static bool NullBooleanCollider(const CInternalCollisionStructure& collision);
  static bool NullMovingCollider(const CInternalCollisionStructure& collision,
                                 const CVector3f& direction, double& distance,
                                 CCollisionInfo& info);
  static bool NullCollisionCollider(const CInternalCollisionStructure& collision,
                                    CCollisionInfoList& collisions);

  static void PushActorAwayFromWalls(CStateManager& mgr, CPhysicsActor& actor, float dt,
                                     float height, float distance, float acceleration,
                                     int iterations, float radius);
  static CVector3f GetActorRelativeVelocities(const CPhysicsActor* actor,
                                              const CPhysicsActor* other);
  // Guessed name, correlated with Prime's search for a non-intersecting displacement.
  static rstl::optional_object< CVector3f >
  FindNonIntersectingVector(const CStateManager& mgr, CPhysicsActor& actor,
                            const CCollisionPrimitive& primitive,
                            const rstl::optional_object< CVector3f >& origin);
  static void CollisionFailsafe(const CStateManager& mgr, CAreaCollisionCache& cache,
                                CPhysicsActor& actor, const CCollisionPrimitive& primitive,
                                const rstl::reserved_vector< TUniqueId, 1024 >& nearList, float dt,
                                uint failsafeTicks, float impulseScale);
  static void MovePlayer(CStateManager& mgr, CPhysicsActor& actor, float dt,
                         const rstl::reserved_vector< TUniqueId, 1024 >* nearList);
  static void Move(CStateManager& mgr, CPhysicsActor& actor, float dt,
                   const rstl::reserved_vector< TUniqueId, 1024 >* nearList);
  static void CollideWithStaticBodyNoRot(CPhysicsActor& actor, const CMaterialList& material,
                                         const CMaterialList& otherMaterial,
                                         const CUnitVector3f& normal, float restitution,
                                         bool flattenNormal);
  static void CollideWithDynamicBodyNoRot(CPhysicsActor& actor, CPhysicsActor& other,
                                          const CCollisionInfo& collision, float restitution,
                                          bool flattenNormal);
  static void ResolveCollisions(CPhysicsActor& actor, CPhysicsActor* other,
                                const CCollisionInfoList& collisions);
  static float GetMinExtentForCollisionPrimitive(const CCollisionPrimitive& primitive);
  static bool IsFloor(const CMaterialList& material, const CVector3f& normal);
  static bool CanBlock(const CMaterialList& material, const CVector3f& normal);
  static float GetCoefficientOfRestitution(const CCollisionInfo& collision);
  static void ShowCollisionResults(CCollisionInfoList& collisions, const CColor& color);
  static void SendMaterialMessage(CStateManager& mgr, const CMaterialList& material, CActor& actor);
  static void SendScriptMessages(CStateManager& mgr, CActor& actor, CActor* other,
                                 const CCollisionInfoList& collisions);

  static bool DetectStaticCollisionBoolean(const CStateManager& mgr,
                                           const CCollisionPrimitive& primitive,
                                           const CTransform4f& transform,
                                           const CMaterialFilter& filter);
  static bool DetectStaticCollisionBoolean_Cached(const CStateManager& mgr,
                                                  CAreaCollisionCache& cache,
                                                  const CCollisionPrimitive& primitive,
                                                  const CTransform4f& transform,
                                                  const CMaterialFilter& filter);
  static bool DetectStaticCollision(const CStateManager& mgr, const CCollisionPrimitive& primitive,
                                    const CTransform4f& transform, const CMaterialFilter& filter,
                                    CCollisionInfoList& collisions);
  static bool DetectStaticCollision_Cached(const CStateManager& mgr, CAreaCollisionCache& cache,
                                           const CCollisionPrimitive& primitive,
                                           const CTransform4f& transform,
                                           const CMaterialFilter& filter,
                                           CCollisionInfoList& collisions);
  static bool DetectStaticCollision_Cached_Moving(
      const CStateManager& mgr, CAreaCollisionCache& cache, const CCollisionPrimitive& primitive,
      const CTransform4f& transform, const CMaterialFilter& filter, const CVector3f& direction,
      CCollisionInfo& collision, double& distance);
  static bool DetectDynamicCollision(const CCollisionPrimitive& primitive,
                                     const CTransform4f& transform,
                                     const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                     TUniqueId& idOut, CCollisionInfoList& collisions,
                                     const CStateManager& mgr);
  static bool
  DetectDynamicCollisionBoolean(const CCollisionPrimitive& primitive, const CTransform4f& transform,
                                const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                const CStateManager& mgr);
  // Guessed overload identities for Echoes's per-actor collision adapters.
  static bool DetectDynamicCollision(const CCollisionPrimitive& primitive,
                                     const CTransform4f& transform, const CPhysicsActor& actor,
                                     CCollisionInfoList& collisions);
  static bool DetectDynamicCollisionBoolean(const CCollisionPrimitive& primitive,
                                            const CTransform4f& transform,
                                            const CPhysicsActor& actor);
  static bool DetectDynamicCollisionMoving(const CCollisionPrimitive& primitive,
                                           const CTransform4f& transform,
                                           const CPhysicsActor& actor, const CVector3f& direction,
                                           CCollisionInfo& collision, double& distance);
  static bool DetectDynamicCollisionMoving(const CCollisionPrimitive& primitive,
                                           const CTransform4f& transform,
                                           const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                           const CVector3f& direction, TUniqueId& idOut,
                                           CCollisionInfo& collision, double& distance,
                                           const CStateManager& mgr);
  static bool DetectCollision_Cached_Moving(
      const CStateManager& mgr, CAreaCollisionCache& cache, const CCollisionPrimitive& primitive,
      const CTransform4f& transform, const CMaterialFilter& filter,
      const rstl::reserved_vector< TUniqueId, 1024 >& nearList, const CVector3f& direction,
      TUniqueId& idOut, CCollisionInfo& collision, double& distance);
  static bool DetectCollision_Cached(const CStateManager& mgr, CAreaCollisionCache& cache,
                                     const CCollisionPrimitive& primitive,
                                     const CTransform4f& transform, const CMaterialFilter& filter,
                                     const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                     TUniqueId& idOut, CCollisionInfoList& collisions);
  static bool
  DetectCollisionBoolean_Cached(const CStateManager& mgr, CAreaCollisionCache& cache,
                                const CCollisionPrimitive& primitive, const CTransform4f& transform,
                                const CMaterialFilter& filter,
                                const rstl::reserved_vector< TUniqueId, 1024 >& nearList);
  static void MakeCollisionCallbacks(CStateManager& mgr, CPhysicsActor& actor, const TUniqueId& id,
                                     const CCollisionInfoList& collisions);
  static bool DetectCollisionBoolean(const CStateManager& mgr, const CCollisionPrimitive& primitive,
                                     const CTransform4f& transform, const CMaterialFilter& filter,
                                     const rstl::reserved_vector< TUniqueId, 1024 >& nearList);
  static void BuildAreaCollisionCache(const CStateManager& mgr, CAreaCollisionCache& cache);

  static CRayCastResult RayStaticIntersection(const CStateManager& mgr, const CVector3f& position,
                                              const CVector3f& direction, float length,
                                              const CMaterialFilter& filter);
  static bool RayStaticLineOfSightTest(const CGameArea& area, const CVector3f& position,
                                       const CVector3f& direction, float length,
                                       const CMaterialFilter& filter);
  static bool RayStaticLineOfSightTest(const CStateManager& mgr, const CVector3f& position,
                                       const CVector3f& direction, float length,
                                       const CMaterialFilter& filter);
  static bool RayDynamicLineOfSightTest(const CStateManager& mgr, const CVector3f& position,
                                        const CVector3f& direction, float length,
                                        const CMaterialFilter& filter,
                                        const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                        const CActor* ignoreActor);
  static CRayCastResult
  RayDynamicIntersection(const CStateManager& mgr, TUniqueId& idOut, const CVector3f& position,
                         const CVector3f& direction, float length, const CMaterialFilter& filter,
                         const rstl::reserved_vector< TUniqueId, 1024 >& nearList);
  static CRayCastResult
  RayWorldIntersection(const CStateManager& mgr, TUniqueId& idOut, const CVector3f& position,
                       const CVector3f& direction, float length, const CMaterialFilter& filter,
                       const rstl::reserved_vector< TUniqueId, 1024 >& nearList);

  // Echoes's packed cache is a separate interface from the legacy area cache above.
  // Guessed names for the native keep/remove near-list policy.
  enum ECacheUpdatePolicy { kCUP_KeepNearListIds = 0, kCUP_RemoveCachedNearListIds = 1 };

  static void MoveAndCollide(CStateManager& mgr, CPhysicsActor& actor, float dt,
                             const ICollisionFilter& collisionFilter,
                             const rstl::reserved_vector< TUniqueId, 1024 >* nearList);
  static rstl::optional_object< CVector3f >
  FindNonIntersectingVector(const CStateManager& mgr, CPhysicsActor& actor,
                            const CCollisionPrimitive& primitive);
  static void CollisionFailsafe(const CStateManager& mgr, CCollisionCache& cache,
                                CPhysicsActor& actor, const CCollisionPrimitive& primitive,
                                const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                float dtFraction, uint failsafeTicks, float impulseScale);
  static bool DetectStaticCollision_Cached_Moving(const CStateManager& mgr, CCollisionCache& cache,
                                                  const CCollisionPrimitive& primitive,
                                                  const CTransform4f& transform,
                                                  const CMaterialFilter& filter,
                                                  const CVector3f& direction,
                                                  CCollisionInfo& collision, double& distance);
  static bool DetectStaticCollisionBoolean_Cached(const CStateManager& mgr, CCollisionCache& cache,
                                                  const CCollisionPrimitive& primitive,
                                                  const CTransform4f& transform,
                                                  const CMaterialFilter& filter);
  static bool DetectStaticCollision_Cached(const CStateManager& mgr, CCollisionCache& cache,
                                           const CCollisionPrimitive& primitive,
                                           const CTransform4f& transform,
                                           const CMaterialFilter& filter,
                                           CCollisionInfoList& collisions);
  static bool DetectCollision_Cached_Moving(
      const CStateManager& mgr, CCollisionCache& cache, const CCollisionPrimitive& primitive,
      const CTransform4f& transform, const CMaterialFilter& filter,
      const rstl::reserved_vector< TUniqueId, 1024 >& nearList, const CVector3f& direction,
      CCollisionInfo& collision, double& distance);
  static bool DetectCollision_Cached(const CStateManager& mgr, CCollisionCache& cache,
                                     const CCollisionPrimitive& primitive,
                                     const CTransform4f& transform, const CMaterialFilter& filter,
                                     const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                     TUniqueId& idOut, CCollisionInfoList& collisions);
  static bool
  DetectCollisionBoolean_Cached(const CStateManager& mgr, CCollisionCache& cache,
                                const CCollisionPrimitive& primitive, const CTransform4f& transform,
                                const CMaterialFilter& filter,
                                const rstl::reserved_vector< TUniqueId, 1024 >& nearList);
  static bool CacheActorGeometry(const CStateManager& mgr, CCollisionCache& cache,
                                 const CEntity* entity);
  static void UpdateCollisionCache(const CStateManager& mgr, CCollisionCache& cache,
                                   rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                   ECacheUpdatePolicy policy);
  static void BuildCollisionCache(const CStateManager& mgr, CCollisionCache& cache,
                                  rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                  ECacheUpdatePolicy policy);
  static void BuildCollisionCache(const CStateManager& mgr, CCollisionCache& cache,
                                  const CMaterialFilter& filter);
};

#endif // _CGAMECOLLISION
