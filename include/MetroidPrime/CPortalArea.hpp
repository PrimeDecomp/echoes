#ifndef _CPORTALAREA
#define _CPORTALAREA

#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CPortalAreaData.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CActor;
class CStateManager;
class CGameCamera;
class CFrustumPlanes;
class CTransform4f;

// Runtime names are reconstructed from the portal-visibility consumers.
class CPortalArea {
public:
  explicit CPortalArea(const TLockedToken< CPortalAreaData >& data);
  void AddActor(CStateManager& mgr, CActor& actor);
  bool RemoveActor(CStateManager& mgr, const TUniqueId& uid);
  void UpdateActor(CStateManager& mgr, CActor& actor);
  void PreRender(CStateManager& mgr, const CGameCamera& camera, const CTransform4f& xf);

  const CObjectList& GetVisibleActors() const { return mVisibleActors; }

private:
  struct SActorNode {
    SActorNode() : mActor(nullptr), mNext(nullptr) {}

    CActor* mActor;
    SActorNode* mNext;
  };

  struct SActorPool {
    SActorPool();
    SActorNode* AllocateNode();
    void FreeNode(SActorNode* node);

    SActorNode* mFree;
    rstl::reserved_vector< SActorNode, 2048 > mNodes;
  };

  struct SActorList {
    explicit SActorList(SActorPool& pool) : mPool(&pool), mHead(nullptr) {}
    void AddActor(CActor& actor);
    bool RemoveActor(const TUniqueId& uid);

    SActorPool* mPool;
    SActorNode* mHead;
  };

  struct SPortalState {
    SPortalState(SActorPool& pool, const CPortalAreaData::SVolume& volume)
    : x0_(-1), mActors(pool), mVolumeData(&volume) {}

    void AddActor(CActor& actor);
    bool RemoveActor(const TUniqueId& uid);

    int x0_;
    SActorList mActors;
    const CPortalAreaData::SVolume* mVolumeData;
  };

  void BuildVisibleActorList(CStateManager& mgr, const CGameCamera& camera, short volumeIndex,
                             const CFrustumPlanes& frustum, bool skipPortals,
                             const CVector3f& entryPoint);

  SActorPool mActorPool;
  uint mVisibilityGeneration;
  uint mActorCount;
  TLockedToken< CPortalAreaData > mData;
  // The inherited SPortalState spelling denotes per-volume actor lists.
  rstl::vector< SPortalState > mVolumes;
  SActorList mUnassignedActors;
  CObjectList mVisibleActors;
};
CHECK_SIZEOF(CPortalArea, 0x6044)

#endif // _CPORTALAREA
