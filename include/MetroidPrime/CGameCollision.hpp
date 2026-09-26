#ifndef _CGAMECOLLISION
#define _CGAMECOLLISION

#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/reserved_vector.hpp"

class CAreaCollisionCache;
class CCollisionCache; // Guessed name; distinct from CAreaCollisionCache.
class CCollisionInfo;
class CCollisionInfoList;
class CCollisionPrimitive;
class CMaterialFilter;
class CMaterialList;
class CPhysicsActor;
class CStateManager;
class CTransform4f;
class CVector3f;

class CGameCollision {
public:
  static void Move(CStateManager& mgr, CPhysicsActor& actor, float dt,
                   const rstl::reserved_vector< TUniqueId, 1024 >* nearList);
  static bool IsFloor(const CMaterialList& material, const CVector3f& normal);
  static bool CanBlock(const CMaterialList& material, const CVector3f& normal);
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
  static bool DetectCollision_Cached_Moving(
      const CStateManager& mgr, CCollisionCache& cache, const CCollisionPrimitive& primitive,
      const CTransform4f& transform, const CMaterialFilter& filter,
      const rstl::reserved_vector< TUniqueId, 1024 >& nearList, const CVector3f& direction,
      CCollisionInfo& collision, double& distance);
};

#endif // _CGAMECOLLISION
