#ifndef _CSURFACEALIGNMENTHELPER
#define _CSURFACEALIGNMENTHELPER

#include "Collision/CMaterialFilter.hpp"
#include "WorldFormat/CCollisionCache.hpp"
#include "WorldFormat/CCollisionSurface.hpp"
#include "rstl/single_ptr.hpp"

class CActor;
class CPhysicsActor;
class CStateManager;

// Guessed name
class CSurfaceAlignmentHelper {
public:
  enum EMode { // Guessed names
    kM_None,
    kM_NearbySurface,
    kM_WorldUp,
  };

  CSurfaceAlignmentHelper();
  explicit CSurfaceAlignmentHelper(const CMaterialFilter& filter);
  CVector3f ProjectPointToPlane(const CVector3f& point, const CVector3f& origin,
                                const CVector3f& normal) const; // Guessed names
  bool PointOnSurface(const CCollisionSurface& surface, const CVector3f& point) const;
  void OrientToSurfaceNormal(CActor& actor, const CVector3f& normal, float dt) const;
  void OrientToStoredSurface(CActor& actor, float dt) const;
  void AlignNearPosition(CActor& actor, CStateManager& mgr, const CVector3f& position, float dt);
  bool FindNearestSurface(CStateManager& mgr, const CVector3f& position, float radius,
                          CCollisionSurface& surface);
  void Update(CPhysicsActor& actor, CStateManager& mgr, float dt);
  void SetMode(EMode mode) { mMode = mode; } // Guessed name
  const CCollisionSurface& GetSurface() const { return mSurface; } // Guessed name

private:
  CCollisionSurface mSurface; // Guessed names
  CMaterialFilter mFilter;
  float mAngularRate;
  EMode mMode;
  rstl::single_ptr< CCollisionCache > mCache;
};
CHECK_SIZEOF(CSurfaceAlignmentHelper, 0x58)

#endif // _CSURFACEALIGNMENTHELPER
