#ifndef _CPORTALAREA
#define _CPORTALAREA

#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CActor;
class CStateManager;
class CPortalAreaData;

// Layout-derived names; the PTLA resource interface remains incomplete.
class CPortalArea {
public:
  explicit CPortalArea(const TLockedToken< CPortalAreaData >& data);
  void UpdateActor(CStateManager& mgr, CActor& actor);

private:
  struct SActorNode {
    CActor* mActor;
    SActorNode* mNext;
  };

  struct SActorPool {
    SActorNode* mFree;
    rstl::reserved_vector< SActorNode, 2048 > mNodes;
  };

  struct SActorList {
    SActorPool* mPool;
    SActorNode* mHead;
  };

  struct SPortalState {
    int x0_;
    SActorList mActors;
    const void* mPortalData;
  };

  SActorPool mActorPool;
  uint x4008_;
  uint x400c_;
  TLockedToken< CPortalAreaData > mData;
  rstl::vector< SPortalState > mPortals;
  SActorList mUnassignedActors;
  CObjectList mActors;
};
CHECK_SIZEOF(CPortalArea, 0x6044)

#endif // _CPORTALAREA
