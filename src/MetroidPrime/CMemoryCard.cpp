#include "MetroidPrime/CMemoryCard.hpp"

#include "MetroidPrime/CDummyWorld.hpp"

#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "rstl/algorithm.hpp"

CMemoryCard::CMemoryCard() : mHints(gpSimplePool->GetObj("HINT_Hints")) {
  mHints.Lock();
  mWorldInter = rs_new rstl::vector< CSaveWorldIntermediate >;
  mMemoryWorlds.reserve(40);
  mWorldInter->reserve(40);

  const rstl::vector< rstl::pair< rstl::string, SObjectTag > > resources =
      gpResourceFactory->GetResourceIdToNameList();
  for (rstl::vector< rstl::pair< rstl::string, SObjectTag > >::const_iterator it =
           resources.begin();
       it != resources.end(); ++it) {
    const CAssetId worldId = it->second.id;
    if (gpResourceFactory->GetResourceTypeById(worldId) != 'MLVL') {
      continue;
    }

    rstl::vector< MemoryWorld >::iterator existing =
        rstl::lower_bound(mMemoryWorlds.begin(), mMemoryWorlds.end(), worldId,
                          rstl::default_pair_sorter_finder< rstl::vector< MemoryWorld > >());
    if (existing == mMemoryWorlds.end() || existing->first != worldId) {
      mMemoryWorlds.insert(existing, MemoryWorld(worldId, CSaveWorldMemory()));
      mWorldInter->push_back(CSaveWorldIntermediate(worldId, kInvalidAssetId));
    }
  }
}

CMemoryCard::~CMemoryCard() {}

CSaveWorldIntermediate::CSaveWorldIntermediate(CAssetId mlvlId, CAssetId savwId) {
  if (savwId == kInvalidAssetId) {
    mDummyWorld = rs_new CDummyWorld(mlvlId, false);
  } else {
    mSaveWorld =
        rs_new TCachedToken< CWorldSaveGameInfo >(gpSimplePool->GetObj(SObjectTag('SAVW', savwId)));
    mSaveWorld->Lock();
  }

  mMlvlId = mlvlId;
  mWorldNameId = kInvalidAssetId;
  mDarkWorldNameId = kInvalidAssetId;
  mSaveWorldId = savwId;
}

bool CSaveWorldIntermediate::InitializePump() {
  if (!mDummyWorld.null()) {
    // TODO: finish the dummy-world load, collect area save IDs and shared layer metadata
    // from CWorldState, then start the SAVW token and release the dummy world.
    return false;
  }
  return mSaveWorld.null() || mSaveWorld->IsLoaded();
}

bool CMemoryCard::InitializePump() {
  if (mWorldInter.null()) {
    for (rstl::vector< MemoryWorld >::iterator it = mMemoryWorlds.begin();
         it != mMemoryWorlds.end(); ++it) {
      CSaveWorldMemory& memory = it->second;
      if (memory.mWorldName.valid() && !memory.mWorldName->IsLoaded()) {
        return false;
      }
      if (memory.mDarkWorldName.valid() && !memory.mDarkWorldName->IsLoaded()) {
        return false;
      }
    }
    return mHints.IsLoaded();
  }

  bool done = true;
  rstl::vector< CSaveWorldIntermediate >& worlds = *mWorldInter;
  for (rstl::vector< CSaveWorldIntermediate >::iterator it = worlds.begin(); it != worlds.end();
       ++it) {
    CSaveWorldIntermediate& world = *it;
    if (!world.InitializePump()) {
      done = false;
      continue;
    }
    if (world.mSaveWorld.null()) {
      continue;
    }

    CSaveWorldMemory& memory = const_cast< CSaveWorldMemory& >(GetSaveWorldMemory(world.mMlvlId));
    if (memory.mSaveWorldId == kInvalidAssetId) {
      memory.mSaveWorldId = world.mSaveWorldId;
    }
    if (memory.mWorldNameId == kInvalidAssetId) {
      memory.mWorldNameId = world.mWorldNameId;
    }
    if (memory.mDarkWorldNameId == kInvalidAssetId) {
      memory.mDarkWorldNameId = world.mDarkWorldNameId;
    }
    memory.mAreaIds = world.mAreaIds;
    memory.mDefaultLayerStates = world.mDefaultLayerStates;
    memory.mLayerNames = world.mLayerNames;
    memory.mAreaLayerNameOffsets = world.mAreaLayerNameOffsets;

    const CWorldSaveGameInfo& saveInfo = *world.mSaveWorld->GetObject();
    memory.mAreaCount = saveInfo.GetAreaCount();
    mScanStates.reserve(mScanStates.size() + saveInfo.GetScans().size());
    for (rstl::vector< ScanState >::const_iterator scan = saveInfo.GetScans().begin();
         scan != saveInfo.GetScans().end(); ++scan) {
      if (rstl::find(mScanStates.begin(), mScanStates.end(), *scan) == mScanStates.end()) {
        mScanStates.push_back(*scan);
      }
    }
    MergeEnvironmentVariables(saveInfo.GetSystemVariables(), mSystemVariables);
    MergeEnvironmentVariables(saveInfo.GetGameVariables(), mGameVariables);

    memory.mSaveWorld = *world.mSaveWorld;
    world.mSaveWorld = nullptr;

    const SObjectTag worldName('STRG', memory.mWorldNameId);
    if (gpResourceFactory->CanBuild(worldName)) {
      memory.mWorldName = TCachedToken< CStringTable >(gpSimplePool->GetObj(worldName));
      memory.mWorldName->Lock();
    }
    const SObjectTag darkWorldName('STRG', memory.mDarkWorldNameId);
    if (gpResourceFactory->CanBuild(darkWorldName)) {
      memory.mDarkWorldName = TCachedToken< CStringTable >(gpSimplePool->GetObj(darkWorldName));
      memory.mDarkWorldName->Lock();
    }
  }

  if (done) {
    mWorldInter = nullptr;
    rstl::sort_by_key(mScanStates);
  }
  return false;
}

bool CMemoryCard::HasSaveWorldMemory(CAssetId worldId) const {
  return rstl::find_by_key(mMemoryWorlds, worldId) != mMemoryWorlds.end();
}

const CSaveWorldMemory& CMemoryCard::GetSaveWorldMemory(CAssetId worldId) const {
  return rstl::find_by_key(mMemoryWorlds, worldId)->second;
}

const wchar_t* CSaveWorldMemory::GetFrontEndName() const {
  if (!mWorldName.valid() || mWorldName->GetObject() == nullptr) {
    return nullptr;
  }
  const CStringTable& names = *mWorldName->GetObject();
  return names.GetString(names.GetStringCount() < 4 ? 0 : 3);
}

const wchar_t* CSaveWorldMemory::GetDarkFrontEndName() const {
  if (!mDarkWorldName.valid() || mDarkWorldName->GetObject() == nullptr) {
    return nullptr;
  }
  const CStringTable& names = *mDarkWorldName->GetObject();
  return names.GetString(names.GetStringCount() < 4 ? 0 : 3);
}

rstl::pair< CAssetId, TAreaId > CMemoryCard::GetAreaAndWorldIdForSaveId(uint saveId) const {
  for (rstl::vector< MemoryWorld >::const_iterator it = mMemoryWorlds.begin();
       it != mMemoryWorlds.end(); ++it) {
    const rstl::vector< uint >& areas = it->second.mAreaIds;
    rstl::vector< uint >::const_iterator area = rstl::find(areas.begin(), areas.end(), saveId);
    if (area != areas.end()) {
      return rstl::pair< CAssetId, TAreaId >(it->first, TAreaId(area - areas.begin()));
    }
  }
  return rstl::pair< CAssetId, TAreaId >(kInvalidAssetId, kInvalidAreaId);
}

void CMemoryCard::MergeEnvironmentVariables(const rstl::vector< EnvironmentVariable >& source,
                                            rstl::vector< EnvironmentVariable >& destination) {
  destination.reserve(destination.size() + source.size());
  for (rstl::vector< EnvironmentVariable >::const_iterator it = source.begin(); it != source.end();
       ++it) {
    if (rstl::find(destination.begin(), destination.end(), *it) == destination.end()) {
      destination.push_back(*it);
    }
  }
}
