#ifndef _CANIMATIONMANAGER
#define _CANIMATIONMANAGER

#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/TToken.hpp"

class CAnimationDatabase;

class CAnimationManager {
public:
  CAnimationManager(TToken< CAnimationDatabase > animDB, const CAnimSysContext& sysCtx)
  : mAnimDB(animDB), mSysCtx(sysCtx) {}
  ~CAnimationManager();

private:
  TToken< CAnimationDatabase > mAnimDB;
  CAnimSysContext mSysCtx;
};
CHECK_SIZEOF(CAnimationManager, 0x20)

#endif // _CANIMATIONMANAGER
