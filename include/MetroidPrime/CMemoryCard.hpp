#ifndef _CMEMORYCARD
#define _CMEMORYCARD

#include "types.h"

#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CGameHintInfo.hpp"
#include "MetroidPrime/CSaveWorldMemory.hpp"
#include "MetroidPrime/CWorldSaveGameInfo.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/pair.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CDummyWorld;

class CSaveWorldIntermediate {
  friend class CMemoryCard;

public:
  CSaveWorldIntermediate(CAssetId mlvlId, CAssetId savwId);
  bool InitializePump();

private:
  CAssetId mMlvlId;
  CAssetId mWorldNameId;
  CAssetId mDarkWorldNameId;
  CAssetId mSaveWorldId;
  rstl::vector< uint > mAreaIds;
  rstl::vector< CWorldLayers::Area > mDefaultLayerStates;
  rstl::rc_ptr< rstl::vector< rstl::string > > mLayerNames;
  rstl::rc_ptr< rstl::vector< int > > mAreaLayerNameOffsets;
  rstl::auto_ptr< CDummyWorld > mDummyWorld;
  rstl::auto_ptr< TCachedToken< CWorldSaveGameInfo > > mSaveWorld;
};
CHECK_SIZEOF(CSaveWorldIntermediate, 0x50)

class CMemoryCard {
public:
  typedef rstl::pair< CAssetId, CSaveWorldMemory > MemoryWorld;
  typedef rstl::pair< CAssetId, uint > ScanState;
  typedef CWorldSaveGameInfo::SEnvironmentVariable EnvironmentVariable;

  CMemoryCard();
  ~CMemoryCard();

  bool InitializePump();
  bool HasSaveWorldMemory(CAssetId worldId) const;
  const CSaveWorldMemory& GetSaveWorldMemory(CAssetId worldId) const;
  rstl::pair< CAssetId, TAreaId > GetAreaAndWorldIdForSaveId(uint saveId) const;

  const rstl::vector< MemoryWorld >& GetMemoryWorlds() const { return mMemoryWorlds; }
  const rstl::vector< CGameHintInfo::CGameHint >& GetHints() const {
    return mHints.GetObject()->GetHints();
  }
  const rstl::vector< ScanState >& GetScanStates() const { return mScanStates; }
  const rstl::vector< EnvironmentVariable >& GetSystemVariables() const { return mSystemVariables; }
  const rstl::vector< EnvironmentVariable >& GetGameVariables() const { return mGameVariables; }

private:
  // Guessed name
  void MergeEnvironmentVariables(const rstl::vector< EnvironmentVariable >& source,
                                 rstl::vector< EnvironmentVariable >& destination);

  TCachedToken< CGameHintInfo > mHints;
  rstl::vector< MemoryWorld > mMemoryWorlds;
  rstl::single_ptr< rstl::vector< CSaveWorldIntermediate > > mWorldInter;
  rstl::vector< ScanState > mScanStates;
  rstl::vector< EnvironmentVariable > mSystemVariables;
  rstl::vector< EnvironmentVariable > mGameVariables;
};
CHECK_SIZEOF(CMemoryCard, 0x50)

extern CMemoryCard* gpMemoryCard;

#endif // _CMEMORYCARD
