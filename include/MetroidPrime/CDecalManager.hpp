#ifndef _CDECALMANAGER
#define _CDECALMANAGER

#include "MetroidPrime/TGameTypes.hpp"
#include "Weapons/CDecal.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"

class CAABox;
class CMaterialFilter;
class CStateManager;

class CDecalManager {
public:
  struct SDecal {
    SDecal(const rstl::optional_object< CDecal >& decal, TAreaId areaId, char nextFreeIndex)
    : mDecal(decal), mAreaId(areaId), mNextFreeIndex(nextFreeIndex) {}

    rstl::optional_object< CDecal > mDecal;
    TAreaId mAreaId;
    char mNextFreeIndex;
  };

  static void Initialize();
  static void ShutDown();
  static void Reinitialize();
  static void Update(float dt, CStateManager& mgr);
  static void AddToRenderer(const CStateManager& mgr);
  static void AddDecal(const TToken< CDecalDescription >& description,
                       const CTransform4f& transform, const CUnitVector3f& direction,
                       CStateManager& mgr);

private:
  static rstl::reserved_vector< int, 64 >::iterator
  RemoveFromActiveList(rstl::reserved_vector< int, 64 >::iterator it, int index);
  // Guessed name for the world-mesh triangle collection used during decal creation.
  static void GatherWorldSurfaces(const CStateManager& mgr, const CAABox& bounds,
                                  const CMaterialFilter& filter,
                                  rstl::vector< CCollisionSurface >& surfaces);

  static rstl::reserved_vector< SDecal, 64 > mDecalPool;
  static rstl::reserved_vector< int, 64 > mActiveIndexList;
  static int mFreeIndex;
  static bool mPoolInitialized;
  static float mDeltaTimeSinceLastDecalCreation;
  static int mLastDecalCreatedIndex;
  static CAssetId mLastDecalCreatedAssetId;
};
NESTED_CHECK_SIZEOF(CDecalManager, SDecal, 0xc8)

#endif // _CDECALMANAGER
