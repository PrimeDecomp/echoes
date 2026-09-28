#ifndef _CANIMSYSCONTEXT
#define _CANIMSYSCONTEXT

#include "Kyoto/Animation/CAnimationSet.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/rc_ptr.hpp"

class CTransitionDatabase;
class CRandom16;
class CSimplePool;

class CAnimSysContext {
public:
  CAnimSysContext(const TToken< CTransitionDatabase >& transDb,
                  const rstl::ncrc_ptr< CRandom16 >& random, CSimplePool& store,
                  const CAnimationSet::EventSetList& eventSets)
  : mTransDb(transDb), mRandom(random), mStore(store), mEventSets(eventSets) {}

  CRandom16& GetRandomNumberGenerator() const { return *mRandom; }
  CSimplePool& GetSimplePool() const { return mStore; }
  const CAnimPOIData* GetEventData(int animIdx) const { return &mEventSets[animIdx]; } // Guessed name.

private:
  TToken< CTransitionDatabase > mTransDb;
  rstl::ncrc_ptr< CRandom16 > mRandom;
  CSimplePool& mStore;
  const CAnimationSet::EventSetList& mEventSets; // Guessed name.
};
CHECK_SIZEOF(CAnimSysContext, 0x18)

#endif // _CANIMSYSCONTEXT
