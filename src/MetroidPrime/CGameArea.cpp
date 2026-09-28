#include "MetroidPrime/CGameArea.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CARAMManager.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/CLZOSupport.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "Kyoto/Streams/CMemoryStreamOut.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CRELFileManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/CWorldLayerState.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAreaProperties.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "WorldFormat/CPVSAreaSet.hpp"
#include "rstl/algorithm.hpp"

#include <alloca.h>
#include <dolphin/dvd.h>
#include <dolphin/os/OSCache.h>
#include <stdlib.h>
#include <string.h>

// The complex loading paths below remain scaffolds. This TU is NonMatching.

rstl::string CGameArea::IGetInternalAreaName() const { return mInternalAreaName; }

IGameArea::~IGameArea() {}

int CGameArea::VerifyHeader() const {
  if (!mPostConstructed->mMreaSectionBuffers.empty()) {
    const uint* header =
        reinterpret_cast< const uint* >(mPostConstructed->mMreaSectionBuffers.front().first.get());
    if (header[0] == 0xdeadbeef && header[1] >= 23 && header[1] <= 25) {
      return header[1];
    }
  }
  return 0;
}

int CGameArea::GetSectionIndex(int section) const {
  if (VerifyHeader() < 11) {
    return -1;
  }

  const uint* header =
      reinterpret_cast< const uint* >(mPostConstructed->mMreaSectionBuffers.front().first.get());
  if (section >= 0 && section < 8) {
    return header[17 + section] + 2;
  }
  if (section == 8 || section == 9) {
    return header[26 + section - 8] + 2;
  }
  return -1;
}

CGameArea::CGameArea(CInputStream& in, int index, int mlvlVersion)
: mSelfIdx(index)
, mNameSTRG(in.ReadInt32())
, mTransform(in)
, mBounds(in)
, mAreaAssetId(in.ReadInt32())
, mAreaSaveId(in.ReadInt32())
, mAttachedAreaIndices(in)
, mDependencies1(in)
, mDependencies2(in)
, mSerializedDependencySize(0)
, mDependenciesInAram(nullptr)
, mPhase(kP_Allocate)
, mNext(nullptr)
, mPrev(nullptr)
, mCurrentChain(-1)
, mPostConstructed(nullptr)
, mLoadPaused(false)
, mValidationPaused(false)
, mActive(true)
, mUnloading(false) {
  mBounds = mBounds.GetTransformedAABox(mTransform);
  mLayerDependencyOffsets = rstl::vector< uint >(in);

  const int dockCount = in.ReadInt32();
  mDocks.reserve(dockCount);
  for (int i = 0; i < dockCount; ++i) {
    mDocks.push_back(Dock(in, mTransform));
  }

  if (mlvlVersion > 18) {
    mRelModules = rstl::vector< rstl::string >(in);
    if (mlvlVersion > 20) {
      mRelOffsets = rstl::vector< uint >(in);
    }
  }
  if (mlvlVersion > 19) {
    const rstl::string name(in);
    if (mNameSTRG == kInvalidAssetId) {
      mInternalAreaName = name;
    }
  }

  SortTextureDependencies();
  mSerializedDependencySize = CalculateDependencyListByteCount();
  mDependenciesInAram = CARAMManager::Alloc(mSerializedDependencySize);
  {
    rstl::auto_ptr< uchar > buffer(static_cast< uchar* >(
        CMemory::Alloc(mSerializedDependencySize, IAllocator::kHI_RoundUpLen)));
    {
      CMemoryStreamOut out(buffer.get(), mSerializedDependencySize, CMemoryStreamOut::kOS_NotOwned,
                           64);
      mDependencies2.PutTo(out);
    }

    const uint handle = CARAMManager::DMAToARAM(
        buffer.get(), mDependenciesInAram, mSerializedDependencySize, CARAMManager::kDMAPrio_One);
    mDependencies2 = rstl::vector< rstl::pair< CAssetId, uint > >();
    CARAMManager::WaitForDMACompletion(handle);
  }

  ClearTokenList();
  fn_80054F74();
}

CGameArea::~CGameArea() {
  if (IsLoaded()) {
    RemoveStaticGeometry();
  } else {
    while (!Invalidate(nullptr)) {
    }
  }
  CARAMManager::Free(mDependenciesInAram);
}

void CGameArea::ClearTokenList() {
  mLayerPhases.clear();
  if (mPostConstructed.get()) {
    mPostConstructed->mLayerTokens.clear();
    mPostConstructed->mLayerRelTokens.clear();
    mPostConstructed->mSortedRelTokens.clear();
  }
}

void CGameArea::AddLayerTokens(int layer, rstl::vector< CToken >& tokens) {
  if (mDependencies2.empty()) {
    return;
  }

  const int first = mLayerDependencyOffsets[layer];
  const int last = layer + 1 < mLayerDependencyOffsets.size() ? mLayerDependencyOffsets[layer + 1]
                                                              : mDependencies2.size();
  for (int i = first; i < last; ++i) {
    const SObjectTag tag(mDependencies2[i].second, mDependencies2[i].first);
    if (tag.type == 'AGSC' && !gpSimplePool->HasObject(SObjectTag(tag))) {
      tokens.push_back_unsafe(TToken< int >(rs_new int));
    } else {
      tokens.push_back_unsafe(gpSimplePool->GetObj(tag));
    }
  }
  mLayerPhases[layer] = kLP_Loading;
}

bool CGameArea::UpdateDependencyLoading(CStateManager& mgr) {
  bool finished = true;
  for (int layer = 0; layer < mLayerDependencyOffsets.size(); ++layer) {
    if (mLayerPhases[layer] == kLP_Loading) {
      finished = false;
      break;
    }
  }
  if (!finished) {
    const bool loadTexturesToAram = mgr.IsFullyInitialized();
    int pending = 0;
    for (int i = 0; i < mPostConstructed->mLayerTokens.size(); ++i) {
      const int layer = i != 0 ? i - 1 : mPostConstructed->mLayerTokens.size() - 1;
      const int priorPending = pending;
      if (mLayerPhases[layer] == kLP_Loading) {
        rstl::vector< CToken >& tokens = mPostConstructed->mLayerTokens[layer];
        for (int j = 0; j < tokens.size(); ++j) {
          CToken& token = tokens[j];
          if (token.IsLoaded()) {
            token.Lock();
            if (token.GetReferenceType() == 'TXTR') {
              TToken< CTexture > texture(token);
              CTexture* resource = texture.GetT();
              resource->MakeSwappable();
              if (loadTexturesToAram) {
                resource->LoadToARAM();
              }
            }
          } else {
            if (!token.HasLock()) {
              const SObjectTag tag('MREA', mAreaAssetId);
              gpResourceFactory->GetResLoader().FindResource(tag);
              token.Lock();
            }
            ++pending;
          }
        }
        if (pending > 80) {
          return false;
        }

        for (rstl::list< rstl::pair< int, rstl::auto_ptr< CDvdRequest > > >::const_iterator it =
                 mPostConstructed->mLayerLoadTransactions.begin();
             it != mPostConstructed->mLayerLoadTransactions.end(); ++it) {
          if (it->first == layer) {
            ++pending;
            break;
          }
        }
        if (layer < mPostConstructed->mLayerRelTokens.size()) {
          rstl::vector< CRELFileToken >& rels = mPostConstructed->mLayerRelTokens[layer];
          for (int j = 0; j < rels.size(); ++j) {
            if (!rels[j].IsLoaded()) {
              ++pending;
            }
          }
        }
        if (pending == priorPending) {
          mLayerPhases[layer] = kLP_Ready;
        }
      }
    }

    for (int i = 0; i < mPostConstructed->mSortedRelTokens.size(); ++i) {
      mPostConstructed->mSortedRelTokens[i]->Load();
      if (!mPostConstructed->mSortedRelTokens[i]->IsLoaded()) {
        ++pending;
      }
    }
    if (pending != 0) {
      return false;
    }
  }
  return true;
}

void CGameArea::VerifyTokenList(CStateManager& mgr) {
  const CWorldLayerState& layers = *mgr.m_currentWorldLayerState;
  if (GetTokenCount() == 0) {
    ClearTokenList();
    mLayerPhases.resize(mLayerDependencyOffsets.size(), kLP_Inactive);
    mPostConstructed->mActiveLayers.resize(layers.GetAreaLayerCount(mSelfIdx), false);
    mPostConstructed->mLayerTokens.resize(mLayerDependencyOffsets.size(), rstl::vector< CToken >());

    for (int layer = 0; layer < mLayerDependencyOffsets.size(); ++layer) {
      rstl::vector< CToken >& tokens = mPostConstructed->mLayerTokens[layer];
      const int first = mLayerDependencyOffsets[layer];
      const int last = layer + 1 < mLayerDependencyOffsets.size()
                           ? mLayerDependencyOffsets[layer + 1]
                           : mDependencies2.size();
      tokens.clear();
      tokens.reserve(last - first);
    }

    if (!mDependencies2.empty()) {
      for (int layer = mLayerDependencyOffsets.size() - 1; layer >= 0; --layer) {
        if (layers.IsLayerActive(mSelfIdx, TLayerId(layer))) {
          AddLayerTokens(layer, mPostConstructed->mLayerTokens[layer]);
        }
      }
      for (uint layer = 0; layer < layers.GetAreaLayerCount(mSelfIdx); ++layer) {
        mPostConstructed->mActiveLayers[layer] = layers.IsLayerActive(mSelfIdx, TLayerId(layer));
      }
    }
  }

  const int relLayerCount = mRelOffsets.size() / 2;
  if (relLayerCount != mPostConstructed->mLayerRelTokens.size()) {
    mPostConstructed->mLayerRelTokens.resize(relLayerCount, rstl::vector< CRELFileToken >());
  }
  if (!mRelModules.empty() && mPostConstructed->mSortedRelTokens.empty()) {
    for (int layer = 0; layer < mPostConstructed->mActiveLayers.size(); ++layer) {
      if (mPostConstructed->mActiveLayers[layer]) {
        LoadLayerRelModules(mgr, TLayerId(layer));
      }
    }
    SortRelTokens(layers);
  }
}

void CGameArea::SortRelTokens(const CWorldLayerState& layers) {
  if (mRelModules.empty()) {
    return;
  }

  const int layerCount = mPostConstructed->mLayerRelTokens.size();
  int count = 0;
  for (int layer = 0; layer < layerCount; ++layer) {
    if (mLayerPhases[layer] == kLP_Loading) {
      count += mPostConstructed->mLayerRelTokens[layer].size();
    }
  }

  typedef rstl::pair< uint, CRELFileToken* > SRelLocation;
  SRelLocation* locations = static_cast< SRelLocation* >(alloca(count * sizeof(SRelLocation)));
  int cursor = 0;
  for (int layer = 0; layer < layerCount; ++layer) {
    if (mLayerPhases[layer] == kLP_Loading) {
      rstl::vector< CRELFileToken >& tokens = mPostConstructed->mLayerRelTokens[layer];
      for (int i = 0; i < tokens.size(); ++i, ++cursor) {
        CRELFileToken& token = tokens[i];
        uint offset = 0;
        DVDFileInfo info;
        if (DVDOpen(const_cast< char* >(token.GetFileName().data()), &info)) {
          offset = info.startAddr;
          DVDClose(&info);
        }
        locations[cursor] = SRelLocation(offset, &token);
      }
    }
  }
  rstl::sort(locations, locations + count,
             rstl::pair_sorter_finder< SRelLocation, rstl::less< uint > >(rstl::less< uint >()));

  mPostConstructed->mSortedRelTokens.clear();
  mPostConstructed->mSortedRelTokens.reserve(count);
  for (int i = 0; i < count; ++i) {
    mPostConstructed->mSortedRelTokens.push_back_unsafe(locations[i].second);
  }
}

void CGameArea::FillInStaticGeometry() {
  // TODO: Construct model instances, surface records, and ambient-light lookup arrays.
}

void CGameArea::PostConstructArea() {
  // TODO: Decode MREA sections and construct collision, geometry, lights, PVS, paths, portals, and
  // object lists.
}

void CGameArea::FinishDependencyLoading(CStateManager& mgr) {
  if (IsLoaded()) {
    return;
  }

  for (rstl::list< rstl::auto_ptr< CDvdRequest > >::iterator it =
           mPostConstructed->mLoadTransactions.begin();
       it != mPostConstructed->mLoadTransactions.end(); ++it) {
    if (it->get()) {
      if (!(*it)->IsComplete()) {
        (*it)->WaitUntilComplete();
      }
      ClearDecompressionRequest(it->get());
    }
  }
  for (rstl::list< rstl::pair< int, rstl::auto_ptr< CDvdRequest > > >::iterator it =
           mPostConstructed->mLayerLoadTransactions.begin();
       it != mPostConstructed->mLayerLoadTransactions.end(); ++it) {
    if (it->second.get()) {
      if (!it->second->IsComplete()) {
        it->second->WaitUntilComplete();
      }
      ClearDecompressionRequest(it->second.get());
    }
  }
  while (!mPostConstructed->mDecompressionRequests.empty()) {
    DecompressAreaData();
  }

  if (!mPostConstructed->mLayerRelTokens.empty()) {
    for (int layer = 0; layer < mPostConstructed->mLayerRelTokens.size(); ++layer) {
      for (int i = 0; i < mPostConstructed->mLayerRelTokens[layer].size(); ++i) {
        mPostConstructed->mLayerRelTokens[layer][i].Load();
        while (!mPostConstructed->mLayerRelTokens[layer][i].IsLoaded()) {
          gpRelFileManager->Update();
        }
      }
    }
  }

  if (GetTokenCount() == 0) {
    VerifyTokenList(mgr);
    for (int layer = 0; layer < mPostConstructed->mLayerTokens.size(); ++layer) {
      for (rstl::vector< CToken >::iterator it = mPostConstructed->mLayerTokens[layer].begin();
           it != mPostConstructed->mLayerTokens[layer].end(); ++it) {
        it->Lock();
      }
    }
    for (int layer = 0; layer < mPostConstructed->mLayerTokens.size(); ++layer) {
      for (rstl::vector< CToken >::iterator it = mPostConstructed->mLayerTokens[layer].begin();
           it != mPostConstructed->mLayerTokens[layer].end(); ++it) {
        it->GetObj();
      }
    }
    for (int layer = 0; layer < mPostConstructed->mActiveLayers.size(); ++layer) {
      if (mPostConstructed->mActiveLayers[layer]) {
        mLayerPhases[layer] = kLP_Ready;
      }
    }
    mLayerPhases[mLayerDependencyOffsets.size() - 1] = kLP_Ready;
  }
  mPostConstructed->mLoadTransactions.clear();
  mPostConstructed->mLayerLoadTransactions.clear();
}

void CGameArea::PrepareScriptObjects(CStateManager& mgr) {
  const CWorldLayerState& layers = *mgr.m_currentWorldLayerState;
  const int count = layers.GetAreaLayerCount(mSelfIdx);
  mPostConstructed->mLayerEditorIds.clear();
  mPostConstructed->mLayerEditorIds.resize(count);
  mPostConstructed->mScriptLoadState = rs_new CScriptObjectLoaderHelper::SLoadContext(mSelfIdx);
}

bool CGameArea::LoadScriptObjects(CStateManager& mgr) {
  CScriptObjectLoaderHelper& loader = mgr.ScriptObjectLoaderHelper();
  CScriptObjectLoaderHelper::SLoadContext& context = *mPostConstructed->mScriptLoadState;
  for (int i = 0; i < mPostConstructed->mActiveLayers.size(); ++i) {
    const TLayerId layer(i);
    if (context.mLayerIndex != layer.Value()) {
      continue;
    }
    if (mPostConstructed->mActiveLayers[i]) {
      if (context.mRemainingObjects == 0) {
        const rstl::pair< const uchar*, int > buffer = GetLayerScriptBuffer(layer);
        rstl::auto_ptr< CInputStream > stream(rs_new CMemoryInStream(buffer.first, buffer.second));
        rstl::vector< TEditorId >& ids = mPostConstructed->mLayerEditorIds[i];
        loader.BeginLayerLoad(context, stream, ids);
      }
      uint timeBudget = 4000;
      if (gpMain->GetAverageDrawTime() + gpMain->GetAverageTickTime() > 0.8f) {
        timeBudget = 1000;
      }
      if (loader.ContinueLayerLoad(context, timeBudget, mgr)) {
        mLayerPhases[i] = kLP_Active;
        ++context.mLayerIndex;
        return false;
      }
    } else {
      ++context.mLayerIndex;
    }
  }
  const bool complete = context.mLayerIndex == mPostConstructed->mActiveLayers.size();
  return complete;
}

void CGameArea::FinishScriptObjects(CStateManager& mgr) {
  CScriptObjectLoaderHelper& loader = mgr.ScriptObjectLoaderHelper();
  for (int i = 0; i < mPostConstructed->mActiveLayers.size(); ++i) {
    mPostConstructed->mMreaSectionBuffers[i + mPostConstructed->mFirstScriptSection].first =
        rstl::auto_ptr< char >();
    mPostConstructed->mLayerScriptBuffers[i] = rstl::auto_ptr< char >();
  }
  mPostConstructed->mScriptObjectsInitialized = false;

  const rstl::pair< const uchar*, int > buffer = GetGeneratedScriptBuffer();
  CMemoryInStream stream(buffer.first, buffer.second);
  if (stream.Get< uint >() == 'SCGN') {
    stream.ReadUint8();
    loader.LoadGeneratedScriptObjects(GetId(), stream);
  }
  loader.RegisterScriptObjects(mPostConstructed->mScriptLoadState->mObjects, mgr);
  InitializeDocks(mgr);

  if (mPostConstructed->mPvs.get() && mPostConstructed->mPvsHasActors) {
    for (int i = 0; i < mPostConstructed->mPvs->GetNumActors(); ++i) {
      const CPostConstructed* post = mPostConstructed.get();
      const TEditorId editorId(post->mPvs->GetEntityIdByIndex(i) | (mSelfIdx.Value() << 16));
      const TUniqueId id = mgr.GetIdForScript(editorId);
      if (id != kInvalidUniqueId) {
        const CPVSAreaSet* pvs = mPostConstructed->mPvs.get();
        const int index = i + (pvs->GetNumFeatures() - pvs->GetNumActors());
        if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(id))) {
          actor->SetPvsIndex(index);
        }
      }
    }
  }
  mPostConstructed->mScriptObjectsInitialized = true;
  mPostConstructed->mScriptLoadState = nullptr;
}

bool CGameArea::StartStreamIn(CStateManager& mgr) {
  if (IsLoaded()) {
    return true;
  }
  if (mPostConstructed.get() && mPostConstructed->mStreamingDelay) {
    --mPostConstructed->mStreamingDelay;
    return false;
  }
  if (mLoadPaused) {
    return false;
  }
  return StartStreamingMainArea(mgr);
}

void CGameArea::Validate(CStateManager& mgr) {
  while (!StartStreamIn(mgr)) {
    gpResourceFactory->AsyncIdle(5000, false);
  }
}

void CGameArea::CullDeadAreaRequests() {
  while (!mPostConstructed->mLoadTransactions.empty() &&
         mPostConstructed->mLoadTransactions.front()->IsComplete()) {
    ClearDecompressionRequest(mPostConstructed->mLoadTransactions.front().get());
    mPostConstructed->mLoadTransactions.pop_front();
  }
}

bool CGameArea::Invalidate(CStateManager* mgr) {
  mUnloading = true;
  if (mPhase == kP_Allocate) {
    ClearTokenList();
  } else if (mPhase < kP_LoadScriptObjects) {
    if (mPostConstructed->mDependencyDmaHandle != CARAMManager::GetInvalidDMAHandle()) {
      if (CARAMManager::IsDMACompleted(mPostConstructed->mDependencyDmaHandle)) {
        mPostConstructed->mDependencyDmaHandle = CARAMManager::GetInvalidDMAHandle();
        mPostConstructed->mSerializedDependencies = nullptr;
      } else {
        return false;
      }
    }

    ClearTokenList();
    for (rstl::list< rstl::auto_ptr< CDvdRequest > >::iterator it =
             mPostConstructed->mLoadTransactions.begin();
         it != mPostConstructed->mLoadTransactions.end();) {
      rstl::list< rstl::auto_ptr< CDvdRequest > >::iterator cur = it;
      ++it;
      if (!(*cur)->IsComplete()) {
        (*cur)->PostCancelRequest();
      } else {
        mPostConstructed->mLoadTransactions.erase(cur);
      }
    }
    if (!mPostConstructed->mLoadTransactions.empty()) {
      return false;
    }

    for (rstl::list< rstl::pair< int, rstl::auto_ptr< CDvdRequest > > >::iterator it =
             mPostConstructed->mLayerLoadTransactions.begin();
         it != mPostConstructed->mLayerLoadTransactions.end();) {
      rstl::list< rstl::pair< int, rstl::auto_ptr< CDvdRequest > > >::iterator cur = it;
      ++it;
      if (!cur->second->IsComplete()) {
        cur->second->PostCancelRequest();
      } else {
        mPostConstructed->mLayerLoadTransactions.erase(cur);
      }
    }
    if (!mPostConstructed->mLayerLoadTransactions.empty()) {
      return false;
    }

    mPostConstructed = nullptr;
    mPhase = kP_Allocate;
    ResetLayerData();
  } else {
    if (mgr != nullptr) {
      if (mPhase != kP_Loaded && !StartStreamIn(*mgr)) {
        return false;
      }
      mgr->PrepareAreaUnload(GetId());
    }
    RemoveStaticGeometry();
    mPostConstructed = nullptr;
    mPhase = kP_Allocate;
    ResetLayerData();
    ClearTokenList();
    if (mgr != nullptr) {
      mgr->AreaUnloaded(GetId());
    }
    fn_80054F74();
  }
  mUnloading = false;
  return true;
}

void CGameArea::ResetLayerData() {
  mLayerPhases = rstl::vector< ELayerPhase >();
  mDependencies2 = rstl::vector< rstl::pair< CAssetId, uint > >();
}

char* CGameArea::AllocNewAreaData(int offset, int size) {
  char* buffer = static_cast< char* >(CMemory::Alloc(size, IAllocator::kHI_RoundUpLen));
  rstl::pair< rstl::auto_ptr< char >, int > section(buffer, size);
  mPostConstructed->mMreaSectionBuffers.push_back_unsafe(section);

  const SObjectTag tag('MREA', mAreaAssetId);
  mPostConstructed->mLoadTransactions.push_back(
      gpResourceFactory->GetResLoader().LoadResourcePartAsync(tag, offset, size, buffer));
  return buffer;
}

uint CGameArea::CalculateDependencyListByteCount() const {
  return ALIGN_UP(4 + mDependencies2.size() * 8, 32);
}

int CGameArea::GetNumPartSizes() const {
  const uint* header =
      reinterpret_cast< const uint* >(mPostConstructed->mMreaSectionBuffers.front().first.get());
  return header[16];
}

int CGameArea::GetNumCompressedBlocks() const {
  const uint* header =
      reinterpret_cast< const uint* >(mPostConstructed->mMreaSectionBuffers.front().first.get());
  return header[1] < 24 ? 0 : header[28];
}

bool CGameArea::ReloadAllUnloadedTextures() {
  bool finished = true;

  for (int layer = 0; layer < mPostConstructed->mLayerTokens.size(); ++layer) {
    rstl::vector< CToken >& tokens = mPostConstructed->mLayerTokens[layer];
    for (int i = 0; i < tokens.size(); ++i) {
      CToken& token = tokens[i];
      if (token.GetReferenceType() == 'TXTR' && token.IsLoaded() && token.HasLock()) {
        TToken< CTexture > texture(token);
        CTexture* resource = texture.GetT();
        resource->MakeSwappable();
        if (!resource->LoadToMRAM()) {
          finished = false;
        }
      }
    }
  }
  return finished;
}

bool CGameArea::UnloadAllloadedTextures() {
  bool finished = true;

  for (int layer = 0; layer < mPostConstructed->mLayerTokens.size(); ++layer) {
    rstl::vector< CToken >& tokens = mPostConstructed->mLayerTokens[layer];
    for (int i = 0; i < tokens.size(); ++i) {
      CToken& token = tokens[i];
      if (token.GetReferenceType() == 'TXTR' && token.IsLoaded() && token.HasLock()) {
        TToken< CTexture > texture(token);
        CTexture* resource = texture.GetT();
        resource->MakeSwappable();
        resource->LoadToARAM();
        if (resource->IsARAMTransferInProgress()) {
          finished = false;
        }
      }
    }
  }
  return finished;
}

bool CGameArea::StartStreamingMainArea(CStateManager& mgr) {
  switch (mPhase) {
  case kP_Allocate:
    mPostConstructed = rs_new CPostConstructed(*this);
    mPhase = kP_ReadDependencies;
    mPostConstructed->mSerializedDependencies = static_cast< uchar* >(
        CMemory::Alloc(mSerializedDependencySize, IAllocator::kHI_RoundUpLen));
    mPostConstructed->mDependencyDmaHandle = CARAMManager::DMAToMRAM(
        mDependenciesInAram, mPostConstructed->mSerializedDependencies.get(),
        mSerializedDependencySize, CARAMManager::kDMAPrio_One);
    break;
  case kP_ReadDependencies:
    if (!CARAMManager::IsDMACompleted(mPostConstructed->mDependencyDmaHandle)) {
      break;
    }
    {
      CMemoryInStream in(mPostConstructed->mSerializedDependencies.get(), mSerializedDependencySize,
                         CMemoryInStream::kOS_NotOwned);
      mDependencies2 = rstl::vector< rstl::pair< CAssetId, uint > >(in);
    }
    mPostConstructed->mSerializedDependencies = nullptr;
    mPostConstructed->mDependencyDmaHandle = CARAMManager::GetInvalidDMAHandle();
    mPhase = kP_PrepareDependencies;
    // Fall through.
  case kP_PrepareDependencies:
    VerifyTokenList(mgr);
    mPhase = kP_WaitForDependencies;
    mPostConstructed->mStreamingDelay = 4;
    break;
  case kP_WaitForDependencies:
    if (!UpdateDependencyLoading(mgr)) {
      break;
    }
    mPhase = kP_LoadHeader;
    // Fall through.
  case kP_LoadHeader:
    mPostConstructed->mMreaSectionBuffers.reserve(3);
    AllocNewAreaData(0, 0x80);
    mPhase = kP_LoadSectionSizes;
    // Fall through.
  case kP_LoadSectionSizes: {
    CullDeadAreaRequests();
    if (!mPostConstructed->mLoadTransactions.empty()) {
      break;
    }
    mPostConstructed->mMreaVersion = VerifyHeader();
    const int sectionBytes = ALIGN_UP(GetNumPartSizes() * 4, 32);
    const int headerBytes = mPostConstructed->mMreaSectionBuffers[0].second;
    AllocNewAreaData(headerBytes, sectionBytes);
    if (mPostConstructed->mMreaVersion >= 24) {
      AllocNewAreaData(headerBytes + sectionBytes, ALIGN_UP(GetNumCompressedBlocks() * 16, 32));
    }
    mPhase = kP_ReserveSections;
    break;
  }
  case kP_ReserveSections: {
    CullDeadAreaRequests();
    if (!mPostConstructed->mLoadTransactions.empty()) {
      break;
    }
    const int partCount = GetNumPartSizes();
    mPostConstructed->mMreaSectionBuffers.reserve(partCount + 3);
    int offset = mPostConstructed->mMreaSectionBuffers[0].second;
    offset += mPostConstructed->mMreaSectionBuffers[1].second;
    if (mPostConstructed->mMreaVersion >= 24) {
      offset += mPostConstructed->mMreaSectionBuffers[2].second;
    }
    mPostConstructed->mLoadedSectionCount = 0;
    mPostConstructed->mLoadedBlockCount = 0;
    mPostConstructed->mMreaDataOffset = offset;
    mPhase = kP_LoadDataSections;
    break;
  }
  case kP_LoadDataSections: {
    CullDeadAreaRequests();
    if (mPostConstructed->mMreaVersion < 24) {
      const int firstSection = mPostConstructed->mLoadedSectionCount;
      int totalSize = 0;
      const int partCount = GetNumPartSizes();
      const int* sizes =
          reinterpret_cast< const int* >(mPostConstructed->mMreaSectionBuffers[1].first.get());
      const SObjectTag tag('MREA', mAreaAssetId);
      bool load = true;
      const int scriptStart = GetSectionIndex(1) - 2;
      const int scriptCount =
          reinterpret_cast< const int* >(mPostConstructed->mMreaSectionBuffers[0].first.get())[15];
      int endSection = firstSection;
      if (firstSection >= scriptStart && firstSection < scriptStart + scriptCount) {
        const int layer = firstSection - scriptStart;
        if (mPostConstructed->mFirstScriptSection == -1) {
          mPostConstructed->mFirstScriptSection = mPostConstructed->mMreaSectionBuffers.size();
        }
        if (!mPostConstructed->mActiveLayers[layer]) {
          load = false;
        }
        totalSize = sizes[firstSection];
        endSection = firstSection + 1;
        if (scriptCount != mPostConstructed->mLayerFileOffsets.capacity()) {
          mPostConstructed->mLayerFileOffsets.reserve(scriptCount);
        }
        mPostConstructed->mLayerFileOffsets.push_back_unsafe(mPostConstructed->mMreaDataOffset);
      } else {
        for (endSection = firstSection; endSection < partCount; ++endSection) {
          const int size = sizes[endSection];
          const bool isScript = endSection >= scriptStart && endSection < scriptStart + scriptCount;
          if (endSection != firstSection && (isScript || size + totalSize > 0x20000)) {
            break;
          }
          totalSize += size;
        }
      }

      rstl::auto_ptr< char > buffer(
          load ? static_cast< char* >(CMemory::Alloc(totalSize, IAllocator::kHI_RoundUpLen))
               : nullptr);
      if (load) {
        mPostConstructed->mLoadTransactions.push_back(
            gpResourceFactory->GetResLoader().LoadResourcePartAsync(
                tag, mPostConstructed->mMreaDataOffset, totalSize, buffer.get()));
      }
      mPostConstructed->mMreaDataOffset += totalSize;
      const int firstSize = sizes[firstSection];
      int offset = firstSize;
      mPostConstructed->mMreaSectionBuffers.push_back_unsafe(
          rstl::pair< rstl::auto_ptr< char >, int >(buffer, firstSize));
      for (int i = firstSection + 1; i < endSection; ++i) {
        rstl::auto_ptr< char > section(buffer.get() + offset);
        section.release();
        const int size = sizes[i];
        mPostConstructed->mMreaSectionBuffers.push_back_unsafe(
            rstl::pair< rstl::auto_ptr< char >, int >(section, size));
        offset += size;
      }
      mPostConstructed->mLoadedSectionCount = endSection;
      if (endSection == partCount) {
        mPostConstructed->mMreaSize = mPostConstructed->mMreaDataOffset;
        mPhase = kP_WaitForData;
      }
    } else {
      const int* sizes =
          reinterpret_cast< const int* >(mPostConstructed->mMreaSectionBuffers[1].first.get());
      const SObjectTag tag('MREA', mAreaAssetId);
      const SMreaCompressedBlock& block = reinterpret_cast< const SMreaCompressedBlock* >(
          mPostConstructed->mMreaSectionBuffers[2]
              .first.get())[mPostConstructed->mLoadedBlockCount];
      bool load = true;
      if (block.mSectionCount == 1) {
        const uint scriptStart = GetSectionIndex(1) - 2;
        const uint section = mPostConstructed->mLoadedSectionCount;
        const int scriptCount = reinterpret_cast< const int* >(
            mPostConstructed->mMreaSectionBuffers[0].first.get())[15];
        if (section >= scriptStart && section < scriptStart + scriptCount) {
          const int layer = section - scriptStart;
          if (mPostConstructed->mFirstScriptSection == -1) {
            mPostConstructed->mFirstScriptSection = mPostConstructed->mMreaSectionBuffers.size();
          }
          if (scriptCount != mPostConstructed->mLayerFileOffsets.size()) {
            mPostConstructed->mLayerFileOffsets.resize(scriptCount, 0u);
          }
          if (!mPostConstructed->mActiveLayers[layer]) {
            load = false;
          }
          mPostConstructed->mLayerFileOffsets[layer] = mPostConstructed->mMreaDataOffset;
        }
      }

      char* memory;
      if (load) {
        const int bufferSize = block.mBufferSize;
        memory = static_cast< char* >(CMemory::Alloc(bufferSize, IAllocator::kHI_RoundUpLen));
      } else {
        memory = nullptr;
      }
      rstl::auto_ptr< char > buffer(memory);
      const int compressedSize = ALIGN_UP(block.mCompressedSize, 32);
      const int readSize = compressedSize ? compressedSize : ALIGN_UP(block.mDecompressedSize, 32);
      if (load) {
        mPostConstructed->mLoadTransactions.push_back(
            gpResourceFactory->GetResLoader().LoadResourcePartAsync(
                tag, mPostConstructed->mMreaDataOffset, readSize,
                buffer.get() + block.mBufferSize - readSize));
        if (compressedSize != 0) {
          mPostConstructed->mDecompressionRequests.push_back(
              SDecompressionRequest(mPostConstructed->mLoadTransactions.back().get(),
                                    reinterpret_cast< uchar* >(buffer.get()),
                                    reinterpret_cast< const uchar* >(
                                        buffer.get() + block.mBufferSize - block.mCompressedSize),
                                    block.mCompressedSize, block.mDecompressedSize));
        }
      }
      mPostConstructed->mMreaDataOffset += readSize;
      const int firstSize = sizes[mPostConstructed->mLoadedSectionCount];
      int offset = firstSize;
      mPostConstructed->mMreaSectionBuffers.push_back_unsafe(
          rstl::pair< rstl::auto_ptr< char >, int >(buffer, firstSize));
      for (int i = 1; i < block.mSectionCount; ++i) {
        rstl::auto_ptr< char > section(buffer.get() + offset);
        section.release();
        const int size = sizes[mPostConstructed->mLoadedSectionCount + i];
        mPostConstructed->mMreaSectionBuffers.push_back_unsafe(
            rstl::pair< rstl::auto_ptr< char >, int >(section, size));
        offset += size;
      }
      mPostConstructed->mLoadedSectionCount += block.mSectionCount;
      ++mPostConstructed->mLoadedBlockCount;
      if (mPostConstructed->mLoadedSectionCount == GetNumPartSizes()) {
        mPostConstructed->mMreaSize = mPostConstructed->mMreaDataOffset;
        mPhase = kP_WaitForData;
      }
    }
    break;
  }
  case kP_WaitForData:
    CullDeadAreaRequests();
    DecompressAreaData();
    if (mPostConstructed->mLoadTransactions.empty() &&
        mPostConstructed->mDecompressionRequests.empty()) {
      mPhase = kP_WaitForValidation;
    }
    break;
  case kP_WaitForValidation:
    if (!mValidationPaused) {
      mPhase = kP_FinishDependencies;
    }
    break;
  case kP_FinishDependencies:
    FinishDependencyLoading(mgr);
    mPhase = kP_PostConstruct;
    break;
  case kP_PostConstruct:
    PostConstructArea();
    PrepareScriptObjects(mgr);
    mPhase = kP_LoadScriptObjects;
    break;
  case kP_LoadScriptObjects:
    if (LoadScriptObjects(mgr)) {
      mPhase = kP_FinishScriptObjects;
    }
    break;
  case kP_FinishScriptObjects:
    mPhase = kP_Loaded;
    FinishScriptObjects(mgr);
    if (mSelfIdx != kInvalidAreaId) {
      mgr.World()->MoveAreaToChain3(mSelfIdx);
    }
    mgr.AreaLoaded(GetId());
    return true;
  case kP_Unknown14:
    mPhase = kP_Unknown15;
    break;
  case kP_Loaded:
    return true;
  }
  return false;
}

void CGameArea::DecompressAreaData() {
  if (mPostConstructed->mDecompressionRequests.empty()) {
    return;
  }

  uint decompressed = 0;
  while (!mPostConstructed->mDecompressionRequests.empty() && decompressed + 0x4000 <= 0x18000) {
    SDecompressionRequest& request = mPostConstructed->mDecompressionRequests.front();
    if (request.mRequest) {
      break;
    }

    const short chunkSize = *reinterpret_cast< const short* >(request.mInput);
    uint outputSize = request.mRemainingSize;
    if (chunkSize < 0) {
      const int uncompressedSize = -chunkSize;
      memcpy(request.mOutput, request.mInput + 2, uncompressedSize);
      outputSize = uncompressedSize;
    } else {
      CLZOSupport::Inflate(request.mInput + 2, chunkSize, request.mOutput, outputSize);
    }
    DCFlushRange(request.mOutput, outputSize);

    request.mOutput += outputSize;
    request.mRemainingSize -= outputSize;
    request.mInput += abs(chunkSize) + 2;
    decompressed += outputSize;
    if (request.mRemainingSize == 0) {
      mPostConstructed->mDecompressionRequests.pop_front();
    }
  }
}

int CGameArea::SetChain(CGameArea* next, int chain) {
  if (mCurrentChain == chain) {
    return mCurrentChain;
  }

  if (mPrev) {
    mPrev->mNext = mNext;
  }
  if (mNext) {
    mNext->mPrev = mPrev;
  }
  mPrev = nullptr;
  mNext = next;
  if (next) {
    next->mPrev = this;
  }
  const int oldChain = mCurrentChain;
  mCurrentChain = chain;
  return oldChain;
}

bool CGameArea::TransferARAMTokensOver(EARAMTransfer mode) {
  if (mPostConstructed->mModelsInMram) {
    return true;
  }

  bool finished = true;
  int part = mPostConstructed->mFirstAramSection;
  for (int i = 0; i < mPostConstructed->mAramTokens.size(); ++i) {
    rstl::pair< CARAMToken, int >& entry = mPostConstructed->mAramTokens[i];
    if (entry.first.GetStatus() != CARAMToken::kS_One) {
      mPostConstructed->mAramBytes -= entry.first.GetSize();
    }
    if (mode == kAT_Async && !entry.first.LoadToMRAM()) {
      finished = false;
    } else if (finished) {
      char* buffer = static_cast< char* >(entry.first.GetMRAMSafe());
      int offset = 0;
      for (int j = 0; j < entry.second; ++j) {
        rstl::auto_ptr< char > section(buffer + offset);
        section.release();
        offset += mPostConstructed->mMreaSectionBuffers[part].second;
        mPostConstructed->mMreaSectionBuffers[part++].first = section;
      }
    }
  }
  mPostConstructed->mModelsInMram = finished;
  return finished;
}

bool CGameArea::TransferTokensToARAM() {
  bool finished = true;
  int part = mPostConstructed->mFirstAramSection;
  rstl::auto_ptr< char > empty;
  for (int i = 0; i < mPostConstructed->mAramTokens.size(); ++i) {
    rstl::pair< CARAMToken, int >& entry = mPostConstructed->mAramTokens[i];
    for (int j = 0; j < entry.second; ++j) {
      mPostConstructed->mMreaSectionBuffers[part++].first = empty;
    }
    const CARAMToken::EStatus oldStatus = entry.first.GetStatus();
    entry.first.LoadToARAM();
    if (oldStatus == CARAMToken::kS_One && entry.first.GetStatus() != CARAMToken::kS_One) {
      mPostConstructed->mAramBytes += entry.first.GetSize();
    }
    if (entry.first.GetStatus() >= CARAMToken::kS_Two &&
        entry.first.GetStatus() <= CARAMToken::kS_Five) {
      finished = false;
    }
  }
  mPostConstructed->mModelsInMram = false;
  mPostConstructed->mModelsConstructed = false;
  return finished;
}

void CGameArea::AddStaticGeometry() {
  if (mPostConstructed->mOcclusionState != kOS_Visible) {
    mPostConstructed->mOcclusionFrameCount = 0;
    mPostConstructed->mOcclusionState = kOS_Visible;
    TransferARAMTokensOver(kAT_Blocking);
    if (!mPostConstructed->mModelsConstructed) {
      FillInStaticGeometry();
    }
    CPostConstructed& post = *mPostConstructed;
    gpRender->AddStaticGeometry(
        &post.mModelInstances,
        post.mRenderOctTree.valid() ? post.mRenderOctTree.get_ptr() : nullptr, &post.mSurfaces,
        &post.mAmbientLightIds, &post.mAmbientLightIndices, mSelfIdx.Value());
  }
}

void CGameArea::RemoveStaticGeometry() {
  if (IsLoaded() && mPostConstructed.get() && mPostConstructed->mOcclusionState != kOS_Occluded) {
    mPostConstructed->mOcclusionFrameCount = 0;
    mPostConstructed->mOcclusionState = kOS_Occluded;
    gpRender->RemoveStaticGeometry(&mPostConstructed->mModelInstances);
  }
}

void CGameArea::SetOcclusionState(EOcclusionState state) {
  if (IsLoaded() && state != mPostConstructed->mOcclusionState) {
    if (state == kOS_Occluded) {
      mPostConstructed->x178_2_ = true;
      mPostConstructed->mFinishedOccluding = false;
      RemoveStaticGeometry();
    } else {
      ReloadAllUnloadedTextures();
      AddStaticGeometry();
    }
  }
}

void CGameArea::AliveUpdate(float dt) {
  if (mPostConstructed->mOcclusionState == kOS_Occluded) {
    mPostConstructed->mOccludedTime += dt;
  } else {
    mPostConstructed->mOccludedTime = 0.f;
  }
  UpdateFog(dt);
  UpdateWeaponWorldLighting(dt);
  fn_80054F74();
}

void CGameArea::UpdateDynamicLayers(CStateManager& mgr) {
  typedef rstl::list< rstl::pair< int, rstl::auto_ptr< CDvdRequest > > > TRequests;
  if (!mPostConstructed->mLayerLoadTransactions.empty()) {
    for (TRequests::iterator it = mPostConstructed->mLayerLoadTransactions.begin();
         it != mPostConstructed->mLayerLoadTransactions.end();) {
      if (it->second->IsComplete()) {
        ClearDecompressionRequest(it->second.get());
        it = mPostConstructed->mLayerLoadTransactions.erase(it);
      } else {
        ++it;
      }
    }
  }
  DecompressAreaData();
  if (mPostConstructed->mDecompressionRequests.empty()) {
    for (int layer = 0; layer < mPostConstructed->mLayerTokens.size(); ++layer) {
      UpdateLayerLoading(mgr, TLayerId(layer));
    }
  }
}

bool CGameArea::HasPendingLayerLoads() const {
  if (!mPostConstructed->mDecompressionRequests.empty()) {
    return true;
  }
  for (int i = 0; i < mPostConstructed->mLayerTokens.size(); ++i) {
    switch (mLayerPhases[i]) {
    case kLP_Inactive:
    case kLP_Ready:
    case kLP_Active:
      break;
    default:
      return true;
    }
  }
  return false;
}

int CGameArea::GetLayerRequestCount(const TLayerId layer) const {
  const int layerIdx = layer.Value();
  int count = 0;
  typedef rstl::list< rstl::pair< int, rstl::auto_ptr< CDvdRequest > > > TRequests;
  for (TRequests::iterator it = mPostConstructed->mLayerLoadTransactions.begin();
       it != mPostConstructed->mLayerLoadTransactions.end(); ++it) {
    if (it->first == layerIdx) {
      ++count;
    }
  }
  return count;
}

void CGameArea::UpdateLayerLoading(CStateManager& mgr, const TLayerId layer) {
  const int layerIdx = layer.Value();
  const int requestCount = GetLayerRequestCount(layer);
  switch (mLayerPhases[layerIdx]) {
  case kLP_RestartPending:
    if (requestCount == 0) {
      StartLayerLoad(mgr, layer);
    }
    break;
  case kLP_CancelPending:
    if (requestCount == 0) {
      ClearLayer(mgr, layer);
    }
    break;
  case kLP_Loading: {
    int pending = 0;
    rstl::vector< CToken >& tokens = mPostConstructed->mLayerTokens[layerIdx];
    for (int i = 0; i < tokens.size(); ++i) {
      CToken& token = tokens[i];
      if (token.IsLoaded()) {
        token.Lock();
        if (token.GetReferenceType() == 'TXTR') {
          TToken< CTexture > texture(token);
          CTexture* resource = texture.GetT();
          resource->MakeSwappable();
          if (mPostConstructed->mOcclusionState == kOS_Occluded) {
            resource->LoadToARAM();
          }
        }
      } else {
        if (!token.HasLock()) {
          gpResourceFactory->GetResLoader().FindResource(SObjectTag('MREA', mAreaAssetId));
          token.Lock();
        }
        ++pending;
      }
    }

    if (pending <= 80) {
      rstl::vector< CRELFileToken >& rels = mPostConstructed->mLayerRelTokens[layerIdx];
      for (int i = 0; i < rels.size(); ++i) {
        rels[i].Load();
        if (!rels[i].IsLoaded()) {
          ++pending;
        }
      }
      pending += requestCount;
      pending += mPostConstructed->mDecompressionRequests.size();
      if (pending == 0) {
        mLayerPhases[layerIdx] = kLP_Ready;
      }
    }
    break;
  }
  default:
    break;
  }
}

void CGameArea::PreRender() {
  if (IsLoaded()) {
    if (mPostConstructed->mOcclusionPinged) {
      mPostConstructed->mOcclusionPinged = false;
    } else {
      PingOcclusionState();
    }
  }
}

void CGameArea::PingOcclusionState() {
  if (mPostConstructed->mOcclusionState == kOS_Occluded) {
    if (mPostConstructed->mOcclusionFrameCount < 2) {
      ++mPostConstructed->mOcclusionFrameCount;
      return;
    }
    mPostConstructed->mOcclusionFrameCount = 3;
    if (!mPostConstructed->mFinishedOccluding) {
      const bool unloaded = UnloadAllloadedTextures();
      const bool transferred = TransferTokensToARAM();
      if (unloaded && transferred) {
        mPostConstructed->mFinishedOccluding = true;
      }
    }
  }
  mPostConstructed->x178_2_ = true;
}

void CGameArea::OtherAreaOcclusionChanged() {
  if (mPostConstructed->mOcclusionFrameCount == 3 &&
      mPostConstructed->mOcclusionState == kOS_Occluded) {
    const bool unloaded = UnloadAllloadedTextures();
    const bool transferred = TransferTokensToARAM();
    mPostConstructed->mFinishedOccluding = unloaded && transferred;
    return;
  }
  if (mPostConstructed->mOcclusionState == kOS_Visible) {
    ReloadAllUnloadedTextures();
  }
}

IGameArea::Dock::Dock(CInputStream& in, const CTransform4f& xf)
: mReferenceCount(0), mIsReferenced(false) {
  const int count = in.ReadInt32();
  mDockReferences.reserve(count);
  for (int i = 0; i < count; ++i) {
    const TAreaId area(in.ReadInt32());
    const short dock = in.ReadInt32();
    mDockReferences.push_back(SDockReference(area, dock));
  }
  const int vertexCount = in.ReadInt32();
  for (int i = 0; i < vertexCount; ++i) {
    mPlaneVertices.push_back(xf * CVector3f(in));
  }
}

bool IGameArea::Dock::IsReferenced() const { return mIsReferenced; }

int IGameArea::Dock::GetReferenceCount() const { return mReferenceCount; }

void IGameArea::Dock::SetReferenceCount(int count) {
  mReferenceCount = count;
  mIsReferenced = true;
}

TAreaId IGameArea::Dock::GetConnectedAreaId(int other) const {
  return mDockReferences.empty() ? TAreaId(-1) : mDockReferences[other].mArea;
}

int IGameArea::Dock::GetOtherDockNumber(int other) const {
  return mDockReferences.empty() ? -1 : mDockReferences[other].mDock;
}

void IGameArea::Dock::SetShouldLoadOther(int other, bool should) {
  if (other < mDockReferences.size()) {
    mDockReferences[other].mLoadOther = should;
  }
}

bool IGameArea::Dock::GetShouldLoadOther(int other) const {
  if (other < mDockReferences.size()) {
    return mDockReferences[other].mLoadOther;
  }
  return false;
}

void IGameArea::Dock::SetLoadOtherBlocked(int other, bool blocked) {
  if (other < mDockReferences.size()) {
    mDockReferences[other].mLoadOtherBlocked = blocked;
  }
}

bool IGameArea::Dock::GetLoadOtherBlocked(int other) const {
  if (other < mDockReferences.size()) {
    return mDockReferences[other].mLoadOtherBlocked;
  }
  return false;
}

uchar CGameArea::CAreaObjectList::IsQualified(const CEntity& entity) {
  return mAreaId == entity.GetCurrentAreaId();
}

CGameArea::CAreaFog::CAreaFog()
: mFogMode(kRFM_None)
, mRangeCur(0.f, 1024.f)
, mRangeTarget(mRangeCur)
, mRangeDelta(0.f, 0.f)
, mColorCur(0.5f, 0.5f, 0.5f)
, mColorTarget(mColorCur)
, mColorDelta(0.f) {}

void CGameArea::CAreaFog::DisableFog() { mFogMode = kRFM_None; }

bool CGameArea::CAreaFog::IsFogDisabled() const { return mFogMode == kRFM_None; }

void CGameArea::CAreaFog::SetFogExplicit(ERglFogMode mode, const CColor& color,
                                         const CVector2f& range) {
  mFogMode = mode;
  mColorCur = mColorTarget = CVector3f(color.GetRed(), color.GetGreen(), color.GetBlue());
  mRangeCur = mRangeTarget = range;
}

void CGameArea::CAreaFog::FadeFog(ERglFogMode mode, const CColor& color, const CVector2f& range,
                                  float colorSpeed, const CVector2f& rangeSpeed) {
  if (mFogMode == kRFM_None) {
    mFogMode = mode;
    mColorCur = mColorTarget = CVector3f(color.GetRed(), color.GetGreen(), color.GetBlue());
    mRangeCur = CVector2f(range.GetY(), range.GetY());
    mRangeTarget = range;
  } else {
    mFogMode = mode;
    mColorTarget = CVector3f(color.GetRed(), color.GetGreen(), color.GetBlue());
    mRangeTarget = range;
  }
  mColorDelta = colorSpeed;
  mRangeDelta = rangeSpeed;
}

void CGameArea::CAreaFog::RollFogOut(float rangeSpeed, float colorSpeed, const CColor& color) {
  mRangeDelta = CVector2f(rangeSpeed, 2.f * rangeSpeed);
  mRangeTarget = CVector2f(4096.f, 4096.f);
  mColorDelta = colorSpeed;
  mColorTarget = CVector3f(color.GetRed(), color.GetGreen(), color.GetBlue());
}

CColor CGameArea::CAreaFog::GetColor() const {
  return CColor(mColorCur.GetX(), mColorCur.GetY(), mColorCur.GetZ(), 1.f);
}

void CGameArea::CAreaFog::Update(float dt) {
  if (mFogMode == kRFM_None || (mColorDelta <= 0.f && mRangeDelta == CVector2f(0.f, 0.f))) {
    return;
  }

  const float current[5] = {mColorCur.GetX(), mColorCur.GetY(), mColorCur.GetZ(), mRangeCur.GetX(),
                            mRangeCur.GetY()};
  const float target[5] = {mColorTarget.GetX(), mColorTarget.GetY(), mColorTarget.GetZ(),
                           mRangeTarget.GetX(), mRangeTarget.GetY()};
  const float step[5] = {mColorDelta * dt, mColorDelta * dt, mColorDelta * dt,
                         dt * mRangeDelta.GetX(), dt * mRangeDelta.GetY()};
  float result[5];
  int finished = 0;

  for (int i = 0; i < 5; ++i) {
    const float delta = target[i] - current[i];
    if (step[i] < CMath::AbsF(delta)) {
      result[i] = current[i] + CMath::FastFSel(delta, step[i], -step[i]);
    } else {
      result[i] = target[i];
      ++finished;
    }
  }

  if (finished == 5) {
    mColorDelta = 0.f;
    mRangeDelta = CVector2f(0.f, 0.f);
    if (result[3] == result[4]) {
      mFogMode = kRFM_None;
    }
  }
  if (result[3] > result[4]) {
    result[3] = result[4];
  }
  mColorCur = CVector3f(result[0], result[1], result[2]);
  mRangeCur = CVector2f(result[3], result[4]);
}

void CGameArea::CAreaFog::SetCurrent() const {
  gpRender->SetWorldFog(mFogMode, mRangeCur.GetX(), mRangeCur.GetY(), GetColor());
}

void CGameArea::UpdateFog(float dt) {
  if (mPostConstructed->mAreaFog.get()) {
    mPostConstructed->mAreaFog->Update(dt);
  }
}

bool CGameArea::DoesAreaNeedSkyNow() const {
  if (mPostConstructed.get() && mPostConstructed->mAreaAttributes) {
    return mPostConstructed->mAreaAttributes->GetNeedsSky();
  }
  return false;
}

int CGameArea::DoesAreaNeedEnvFx() const {
  if (!mPostConstructed.get() || !mPostConstructed->mAreaAttributes ||
      mPostConstructed->mOcclusionState != kOS_Visible) {
    return 0;
  }
  return mPostConstructed->mAreaAttributes->GetEnvFxType();
}

bool CGameArea::TryTakingOutOfARAM() {
  if (mPostConstructed->mOcclusionState == kOS_Occluded) {
    mPostConstructed->mOcclusionPinged = true;
  }
  return TransferARAMTokensOver(kAT_Async) && ReloadAllUnloadedTextures();
}

const CTransform4f& CGameArea::IGetTM() const { return mTransform; }

CAssetId CGameArea::IGetStringTableAssetId() const { return mNameSTRG; }

uint CGameArea::IGetNumAttachedAreas() const { return mAttachedAreaIndices.size(); }

TAreaId CGameArea::IGetAttachedAreaId(int index) const {
  return TAreaId(mAttachedAreaIndices[index]);
}

bool CGameArea::IIsActive() const { return mActive; }

CAssetId CGameArea::IGetAreaAssetId() const { return mAreaAssetId; }

int CGameArea::IGetAreaSaveId() const { return mAreaSaveId; }

CDummyGameArea::CDummyGameArea(CInputStream& in, int index, int mlvlVersion)
: mSelfIdx(index), mNameSTRG(kInvalidAssetId), mTransform(CTransform4f::Identity()) {
  mNameSTRG = in.ReadInt32();
  mTransform = CTransform4f(in);
  const CAABox bounds(in);
  mAreaAssetId = in.ReadInt32();
  if (mlvlVersion > 15) {
    mAreaSaveId = in.ReadInt32();
  }
  mAttachedAreaIndices = rstl::vector< ushort >(in);
  {
    const rstl::vector< rstl::pair< CAssetId, uint > > dependencies1(in);
    const rstl::vector< rstl::pair< CAssetId, uint > > dependencies2(in);
  }
  if (mlvlVersion > 13) {
    const rstl::vector< uint > layerOffsets(in);
  }
  const int dockCount = in.ReadInt32();
  mDocks.reserve(dockCount);
  for (int i = 0; i < dockCount; ++i) {
    mDocks.push_back(Dock(in, mTransform));
  }
  if (mlvlVersion > 18) {
    mRelModules = rstl::vector< rstl::string >(in);
    if (mlvlVersion > 20) {
      mRelOffsets = rstl::vector< uint >(in);
    }
  }
  if (mlvlVersion > 19) {
    const rstl::string name(in);
    if (mNameSTRG == kInvalidAssetId) {
      mInternalAreaName = name;
    }
  }
}

const CTransform4f& CDummyGameArea::IGetTM() const { return mTransform; }

CAssetId CDummyGameArea::IGetStringTableAssetId() const { return mNameSTRG; }

uint CDummyGameArea::IGetNumAttachedAreas() const { return mAttachedAreaIndices.size(); }

TAreaId CDummyGameArea::IGetAttachedAreaId(int index) const {
  return TAreaId(mAttachedAreaIndices[index]);
}

bool CDummyGameArea::IIsActive() const { return true; }

CAssetId CDummyGameArea::IGetAreaAssetId() const { return mAreaAssetId; }

int CDummyGameArea::IGetAreaSaveId() const { return mAreaSaveId; }

bool CGameArea::IsFinishedOccluding() const {
  return mPostConstructed->mOcclusionState != kOS_Occluded || mPostConstructed->mFinishedOccluding;
}

rstl::pair< const uchar*, int > CGameArea::GetLayerScriptBuffer(const TLayerId layer) const {
  if (mPhase > kP_WaitForData) {
    return rstl::pair< const uchar*, int >(
        reinterpret_cast< const uchar* >(
            mPostConstructed->mLayerScriptBuffers[layer.Value()].get()),
        GetLayerScriptSize(layer));
  }
  return rstl::pair< const uchar*, int >(nullptr, 0);
}

rstl::pair< const uchar*, int > CGameArea::GetGeneratedScriptBuffer() const {
  if (mPhase > kP_WaitForData) {
    return rstl::pair< const uchar*, int >(
        reinterpret_cast< const uchar* >(mPostConstructed->mGeneratedScriptBuffer.get()),
        mPostConstructed->mGeneratedScriptSize);
  }
  return rstl::pair< const uchar*, int >(nullptr, 0);
}

int CGameArea::GetLayerScriptSize(const TLayerId layer) const {
  return mPhase > kP_WaitForData ? mPostConstructed->mLayerScriptSizes[layer.Value()] : 0;
}

void CGameArea::SetLoadPauseState(bool paused) {
  bool loading = false;
  for (int i = 0; i < mLayerPhases.size(); ++i) {
    if (mLayerPhases[i] == kLP_Loading) {
      loading = true;
      break;
    }
  }
  if (loading) {
    mLoadPaused = paused;
    if (paused) {
      for (int layer = 0; layer < mPostConstructed->mLayerTokens.size(); ++layer) {
        rstl::vector< CToken >& tokens = mPostConstructed->mLayerTokens[layer];
        for (int i = 0; i < tokens.size(); ++i) {
          if (!tokens[i].IsLoaded()) {
            tokens[i].Unlock();
          }
        }
      }
    }
  }
}

void CGameArea::SetAreaAttributes(CScriptAreaProperties* attributes) {
  mPostConstructed->mAreaAttributes = attributes;
}

void CGameArea::SetXRaySpeedAndTarget(float speed, float target) {
  mPostConstructed->mXraySpeed = speed;
  mPostConstructed->mXrayTarget = target;
}

void CGameArea::SetWeaponWorldLighting(float speed, float target) {
  mPostConstructed->mWeaponWorldLightingSpeed = speed;
  mPostConstructed->mWeaponWorldLightingTarget = target;
}

void CGameArea::UpdateWeaponWorldLighting(float dt) {
  // TODO: Blend x-ray and weapon lighting, then mark area actors' lighting dirty.
}

uint CGameArea::Get1stPVSLightFeature(uint index) const {
  // TODO: Resolve first-set light feature indices through CPVSAreaSet.
  return uint(-1);
}

uint CGameArea::Get2ndPVSLightFeature(uint index) const {
  // TODO: Resolve second-set light feature indices through CPVSAreaSet.
  return uint(-1);
}

void CGameArea::InitializeDocks(CStateManager& mgr) {
  // TODO: Filter dock IDs and initialize connected-area loading.
}

void CGameArea::UpdateDocks(CStateManager& mgr) {
  // TODO: Select nearby docks and send Echoes script messages.
}

void CGameArea::fn_80054F74() {}

int CGameArea::GetTokenCount() const {
  int count = 0;
  if (mPostConstructed.get()) {
    for (int i = 0; i < mPostConstructed->mLayerTokens.size(); ++i) {
      count += mPostConstructed->mLayerTokens[i].size();
    }
  }
  return count;
}

CGameArea::ELayerPhase CGameArea::GetLayerPhase(const TLayerId layer) const {
  if (layer.Value() < mLayerPhases.size() && layer.Value() >= 0) {
    return mLayerPhases[layer.Value()];
  }
  return kLP_Inactive;
}

rstl::vector< CRELFileToken >* CGameArea::GetLayerRelTokens(const TLayerId layer) const {
  if (mPostConstructed.get() && layer.Value() >= 0 &&
      layer.Value() < mPostConstructed->mLayerRelTokens.size()) {
    return &mPostConstructed->mLayerRelTokens[layer.Value()];
  }
  return nullptr;
}

bool CGameArea::IsValidLayerNumber(CStateManager& mgr, const TLayerId layer) const {
  const int layerCount = mgr.m_currentWorldLayerState->GetAreaLayerCount(mSelfIdx);
  if (layer.Value() < layerCount && layer.Value() >= 0) {
    return true;
  }
  return false;
}

void CGameArea::LoadLayerDynamic(CStateManager& mgr, const TLayerId layer) {
  CWorldLayerState& layers = *mgr.m_currentWorldLayerState;
  if (!IsValidLayerNumber(mgr, layer)) {
    return;
  }
  if (!layers.IsLayerActive(mSelfIdx, layer)) {
    return;
  }

  const int layerIdx = layer.Value();
  const ELayerPhase phase = mLayerPhases[layerIdx];
  switch (phase) {
  case kLP_Inactive:
    StartLayerLoad(mgr, layer);
    break;
  case kLP_CancelPending:
    mLayerPhases[layerIdx] = kLP_RestartPending;
    break;
  default:
    break;
  }
}

void CGameArea::StartLayerLoad(CStateManager& mgr, const TLayerId layer) {
  if (!IsValidLayerNumber(mgr, layer)) {
    return;
  }

  const int layerIdx = layer.Value();
  const int first = mLayerDependencyOffsets[layerIdx];
  const int last = layerIdx + 1 < mLayerDependencyOffsets.size()
                       ? mLayerDependencyOffsets[layerIdx + 1]
                       : mDependencies2.size();
  const int count = last - first;
  rstl::vector< CToken >& tokens = mPostConstructed->mLayerTokens[layerIdx];
  if (tokens.capacity() == 0) {
    tokens.reserve(count);
  }
  AddLayerTokens(layerIdx, mPostConstructed->mLayerTokens[layerIdx]);

  if (mPostConstructed->mMreaVersion < 24) {
    const SObjectTag tag('MREA', mAreaAssetId);
    const int size = GetLayerScriptSize(layer);
    rstl::auto_ptr< char > buffer(
        static_cast< char* >(CMemory::Alloc(size, IAllocator::kHI_RoundUpLen)));
    mPostConstructed->mLayerLoadTransactions.push_back(
        rstl::pair< int, rstl::auto_ptr< CDvdRequest > >(
            layer.Value(),
            gpResourceFactory->GetResLoader().LoadResourcePartAsync(
                tag, mPostConstructed->mLayerFileOffsets[layerIdx], size, buffer.get())));
    mPostConstructed->mMreaSectionBuffers[layerIdx + mPostConstructed->mFirstScriptSection] =
        rstl::pair< rstl::auto_ptr< char >, int >(buffer, size);
    mPostConstructed->mLayerScriptBuffers[layerIdx] = rstl::auto_ptr< char >(buffer.get());
    mPostConstructed->mLayerScriptBuffers[layerIdx].release();
  } else {
    rstl::auto_ptr< char > buffer;
    rstl::auto_ptr< CDvdRequest > request;
    const int size = GetLayerScriptSize(layer);
    ReadCompressedLayer(mPostConstructed->mLayerFileOffsets[layerIdx], request, buffer);
    mPostConstructed->mLayerLoadTransactions.push_back(
        rstl::pair< int, rstl::auto_ptr< CDvdRequest > >(layer.Value(), request));
    mPostConstructed->mMreaSectionBuffers[layerIdx + mPostConstructed->mFirstScriptSection] =
        rstl::pair< rstl::auto_ptr< char >, int >(buffer, size);
    mPostConstructed->mLayerScriptBuffers[layerIdx] = rstl::auto_ptr< char >(buffer.get());
    mPostConstructed->mLayerScriptBuffers[layerIdx].release();
  }

  mgr.World()->CancelLayerRelUnload(mSelfIdx, layer);
  LoadLayerRelModules(mgr, layer);
}

void CGameArea::RemoveLayerObjects(CStateManager& mgr, const TLayerId layer) {
  CScriptObjectLoaderHelper& loader = mgr.ScriptObjectLoaderHelper();
  loader.RemoveLayerObjects(mSelfIdx, layer, mgr);
  mgr.World()->QueueLayerRelUnload(CWorld::SLayerRelUnload(mSelfIdx, layer, 2));
}

void CGameArea::ClearLayer(CStateManager& mgr, const TLayerId layer) {
  if (!IsValidLayerNumber(mgr, layer)) {
    return;
  }

  const int layerIdx = layer.Value();
  RemoveLayerObjects(mgr, layer);
  mPostConstructed->mLayerTokens[layerIdx] = rstl::vector< CToken >();
  mPostConstructed->mMreaSectionBuffers[layerIdx + mPostConstructed->mFirstScriptSection].first =
      rstl::auto_ptr< char >();
  mPostConstructed->mLayerScriptBuffers[layerIdx] = rstl::auto_ptr< char >();
  mLayerPhases[layerIdx] = kLP_Inactive;
}

void CGameArea::UnloadLayerDynamic(CStateManager& mgr, const TLayerId layer) {
  if (!IsValidLayerNumber(mgr, layer)) {
    return;
  }

  const int layerIdx = layer.Value();
  const ELayerPhase phase = mLayerPhases[layerIdx];
  bool cleared = false;
  switch (phase) {
  case kLP_RestartPending:
  case kLP_Loading:
    mLayerPhases[layerIdx] = kLP_CancelPending;
    break;
  case kLP_Ready:
  case kLP_Active:
    ClearLayer(mgr, layer);
    cleared = true;
    break;
  default:
    break;
  }
  if (!cleared) {
    RemoveLayerObjects(mgr, layer);
  }
}

void CGameArea::ActivateLayerDynamic(CStateManager& mgr, const TLayerId layer) {
  if (!IsValidLayerNumber(mgr, layer)) {
    return;
  }

  CScriptObjectLoaderHelper& loader = mgr.ScriptObjectLoaderHelper();
  const CWorldLayerState& layers = *mgr.m_currentWorldLayerState;
  if (!layers.IsLayerActive(mSelfIdx, layer)) {
    return;
  }

  const int layerIdx = layer.Value();
  layers.GetAreaLayerCount(mSelfIdx);
  rstl::vector< TEditorId >& ids = mPostConstructed->mLayerEditorIds[layerIdx];
  ids = rstl::vector< TEditorId >();
  const rstl::pair< const uchar*, int > buffer = GetLayerScriptBuffer(layer);
  CMemoryInStream in(buffer.first, buffer.second);
  mPostConstructed->mScriptObjectsInitialized = false;
  loader.LoadScriptObjects(GetId(), in, ids, mgr);
  loader.InitScriptObjects(ids, mgr);
  mPostConstructed->mScriptObjectsInitialized = true;

  mPostConstructed->mMreaSectionBuffers[layerIdx + mPostConstructed->mFirstScriptSection].first =
      rstl::auto_ptr< char >();
  mPostConstructed->mLayerScriptBuffers[layerIdx] = rstl::auto_ptr< char >();
  mLayerPhases[layerIdx] = kLP_Active;
}

void CGameArea::LoadLayerRelModules(CStateManager& mgr, const TLayerId layer) {
  const int layerIdx = layer.Value();
  const int first = mRelOffsets[layerIdx * 2];
  const int last = mRelOffsets[layerIdx * 2 + 1];
  const int count = last - first;
  rstl::vector< CRELFileToken >& tokens = mPostConstructed->mLayerRelTokens[layerIdx];
  if (count > 0 && count != tokens.size()) {
    mLayerPhases[layerIdx] = kLP_Loading;
    tokens.clear();
    tokens.reserve(count);
    for (int i = first; i < last; ++i) {
      tokens.push_back_unsafe(CRELFileToken(mRelModules[i], 1));
    }
  }
}

void CGameArea::SortTextureDependencies() {
  // TODO: Sort contiguous TXTR runs within each layer by resource offset.
}

void CGameArea::DisableDocks(CStateManager& mgr) {
  if (!mPostConstructed->mDockIds.empty()) {
    mPostConstructed->mDocksDisabled = true;
    for (rstl::list< TUniqueId >::const_iterator it = mPostConstructed->mDockIds.begin();
         it != mPostConstructed->mDockIds.end(); ++it) {
      mgr.SendScriptMsg(*it, kInvalidUniqueId, kSM_SetToZero, kInvalidUniqueId);
    }
  }
}

void CGameArea::EnableDocks() {
  if (!mPostConstructed->mDockIds.empty()) {
    mPostConstructed->mDocksDisabled = false;
  }
}

void CGameArea::ClearDecompressionRequest(CDvdRequest* request) {
  for (rstl::list< SDecompressionRequest >::iterator it =
           mPostConstructed->mDecompressionRequests.begin();
       it != mPostConstructed->mDecompressionRequests.end(); ++it) {
    if (it->mRequest == request) {
      it->mRequest = nullptr;
    }
  }
}

void CGameArea::ReadCompressedLayer(const int offset, rstl::auto_ptr< CDvdRequest >& request,
                                    rstl::auto_ptr< char >& buffer) {
  const uint* header =
      reinterpret_cast< const uint* >(mPostConstructed->mMreaSectionBuffers.front().first.get());
  if (mPostConstructed->mMreaVersion < 24) {
    return;
  }

  const SMreaCompressedBlock* blocks = reinterpret_cast< const SMreaCompressedBlock* >(
      mPostConstructed->mMreaSectionBuffers[2].first.get());
  uint blockOffset = 0;
  for (int i = 0; i < 3; ++i) {
    blockOffset += mPostConstructed->mMreaSectionBuffers[i].second;
  }
  const int blockCount = header[28];
  for (int i = 0; i < blockCount; ++i) {
    const SMreaCompressedBlock& block = blocks[i];
    const int readSize =
        ALIGN_UP(block.mCompressedSize ? block.mCompressedSize : block.mBufferSize, 32);
    if (offset == blockOffset) {
      const int bufferSize = block.mBufferSize;
      buffer = rstl::auto_ptr< char >(
          static_cast< char* >(CMemory::Alloc(bufferSize, IAllocator::kHI_RoundUpLen)));
      const SObjectTag tag('MREA', mAreaAssetId);
      request = gpResourceFactory->GetResLoader().LoadResourcePartAsync(
          tag, offset, readSize, buffer.get() + block.mBufferSize - readSize);
      if (block.mCompressedSize) {
        mPostConstructed->mDecompressionRequests.push_back(
            SDecompressionRequest(request.get(), reinterpret_cast< uchar* >(buffer.get()),
                                  reinterpret_cast< const uchar* >(
                                      buffer.get() + block.mBufferSize - block.mCompressedSize),
                                  block.mCompressedSize, block.mDecompressedSize));
      }
      break;
    }
    blockOffset += readSize;
  }
}

rstl::string CDummyGameArea::IGetInternalAreaName() const { return mInternalAreaName; }
