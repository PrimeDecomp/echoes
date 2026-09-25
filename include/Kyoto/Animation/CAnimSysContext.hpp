#ifndef _CANIMSYSCONTEXT
#define _CANIMSYSCONTEXT

#include "Kyoto/TToken.hpp"

#include "rstl/rc_ptr.hpp"

class CTransitionDatabaseGame;
class CRandom16;
class CSimplePool;

class CAnimSysContext {
public:
  CAnimSysContext(const TToken< CTransitionDatabaseGame >& transDb,
                  const rstl::ncrc_ptr< CRandom16 >& random, CSimplePool& store)
  : mTransDb(transDb), mRandom(random), mStore(store) {}

private:
  TToken< CTransitionDatabaseGame > mTransDb;
  rstl::rc_ptr< CRandom16 > mRandom;
  CSimplePool& mStore;
};

#endif // _CANIMSYSCONTEXT
