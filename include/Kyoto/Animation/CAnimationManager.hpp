#ifndef _CANIMATIONMANAGER
#define _CANIMATIONMANAGER

#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/TToken.hpp"

class CAnimationDatabase;
class CAnimTreeNode;
class CMetaAnimTreeBuildOrders;
class IMetaAnim;

class CAnimationManager {
public:
  CAnimationManager(TToken< CAnimationDatabase > animDB, const CAnimSysContext& sysCtx)
  : mAnimDB(animDB), mSysCtx(sysCtx) {}
  ~CAnimationManager();

  rstl::ncrc_ptr< CAnimTreeNode > GetAnimationTree(uint animIdx,
                                                   const CMetaAnimTreeBuildOrders& orders) const;
  rstl::rc_ptr< IMetaAnim > GetMetaAnimation(uint animIdx) const;

private:
  TToken< CAnimationDatabase > mAnimDB;
  CAnimSysContext mSysCtx;
};
CHECK_SIZEOF(CAnimationManager, 0x20)

#endif // _CANIMATIONMANAGER
