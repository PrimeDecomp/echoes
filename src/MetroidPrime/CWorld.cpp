#include "MetroidPrime/CWorld.hpp"

#include "MetroidPrime/CDummyWorld.hpp"
#include "MetroidPrime/CWorldLayerState.hpp"
#include "MetroidPrime/CWorldLayers.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/CDvdRequest.hpp" // IWYU pragma: keep
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CInGameTweakManager.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CMapWorld.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CGameMode.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CWorldState.hpp"
#include "MetroidPrime/Player/CWorldTransManager.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAreaProperties.hpp"
#include "MetroidPrime/ScriptObjects/CScriptStreamedMusic.hpp"

#include "alloca.h"

CGameArea::CConstChainIterator CWorld::skGlobalEnd;
CGameArea::CChainIterator CWorld::skGlobalNonConstEnd;

void CWorldLayers::ReadWorldLayers(CInputStream& in, int version, CAssetId mlvlId) {
  rstl::vector< Area > areas(in);
  rstl::rc_ptr< rstl::vector< rstl::string > > names(rs_new rstl::vector< rstl::string >(in));
  rstl::rc_ptr< rstl::vector< int > > indices(rs_new rstl::vector< int >(in));
  gpGameState->StateForWorld(mlvlId).GetLayerState()->InitializeWorldLayers(areas, names, indices);
}

CRelay::CRelay(CInputStream& in)
: mRelay(in.ReadInt32())
, mTarget(in.ReadInt32())
, mMsg(in.ReadUint16())
, mActive(in.ReadBool()) {}

IWorld::~IWorld() {}

CWorld::CWorld(IObjectStore& objStore, CResFactory& resFactory, CAssetId mlvlId)
: mLoadPhase(kP_Loading)
, mMlvlId(mlvlId)
, mStrgId(kInvalidAssetId)
, mDarkStrgId(kInvalidAssetId)
, mSavwId(kInvalidAssetId)
, mAreas()
, mMapwId(kInvalidAssetId)
, mMapWorld()
, mLoadToken()
, mLoadBuf()
, mBufSize(0)
, mChainHeads()
, mObjectStore(&objStore)
, mResFactory(&resFactory)
, mCurAreaId(kInvalidAreaId)
, mCurrentAreaNeedsAllocation(true)
, mLoadPaused(false)
, mSkyboxActive(false)
, mSkyboxVisible(false)
, mDefAudioTrack()
, mSkyboxWorld()
, mSkyboxWorldLoaded()
, mSkyboxOverride()
, mSkyboxFogMode(kRFM_None)
, mSkyboxFogStart(0.f)
, mSkyboxFogEnd(0.f)
, mSkyboxFogColor(CColor::Black())
, mNeededEnvFx(0)
, mGlobalSfxHandles()
, mPendingLayerRelUnloads()
, mSkyboxLightingLevel(1.f) {
  SObjectTag mlvl('MLVL', mlvlId);
  mBufSize = gpResourceFactory->GetResLoader().ResourceSize(mlvl);
  mLoadBuf = static_cast< char* >(CMemory::Alloc(mBufSize, IAllocator::kHI_RoundUpLen));
  mLoadToken = resFactory.GetResLoader().LoadResourceAsync(mlvl, mLoadBuf.get());
}

bool CWorld::CheckWorldComplete(CStateManager* mgr, TAreaId aid, CAssetId mreaId) {
  if (mreaId != kInvalidAssetId) {
    mCurAreaId = TAreaId(0);
    int areaCount = mAreas.size();
    for (int i = 0; i < areaCount; ++i) {
      if (GetArea(TAreaId(i))->GetAreaAssetId() == mreaId) {
        mCurAreaId = TAreaId(i);
        break;
      }
    }
  } else {
    mCurAreaId = aid;
  }

  const bool loadSky = mgr != nullptr;
  switch (mLoadPhase) {
  case kP_Loading: {
    if (!mLoadToken->IsComplete()) {
      return false;
    }
    CMemoryInStream in(mLoadBuf.get(), mBufSize);
    in.ReadInt32();
    int version = in.ReadInt32();
    mStrgId = in.ReadInt32();
    if (version >= 22u) {
      mDarkStrgId = in.ReadInt32();
    }
    if (version >= 23u) {
      mTempleKeyWorldIndex = in.ReadInt32();
    }
    mSavwId = in.ReadInt32();
    CAssetId skyboxId = in.ReadInt32();
    if (skyboxId != kInvalidAssetId && loadSky) {
      mSkyboxWorld = TCachedToken< CModel >(gpSimplePool->GetObj(SObjectTag('CMDL', skyboxId)));
      mSkyboxWorld->Lock();
    }
    if (version == 17u) {
      rstl::vector< CRelay > relays(in);
    }

    int areaCount = in.ReadInt32();
    if (version < 18u) {
      in.ReadInt32();
    }
    mAreas.reserve(areaCount);
    for (int i = 0; i < areaCount; ++i) {
      mAreas.push_back_unsafe(rs_new CGameArea(in, i, version));
    }
    mChainHeads.resize(5, nullptr);
    for (int i = 0; i < areaCount; ++i) {
      MoveToChain(mAreas[i].get(), kC_Deallocated);
    }

    mMapwId = in.ReadInt32();
    mMapWorld =
        rs_new TCachedToken< CMapWorld >(gpSimplePool->GetObj(SObjectTag('MAPW', mMapwId)));
    mMapWorld->Lock();
    if (mgr && version < 18u) {
      rstl::vector< TEditorId > ids;
      CScriptObjectLoaderHelper& loader = mgr->ScriptObjectLoaderHelper();
      loader.LoadScriptObjects(kInvalidAreaId, in, ids, *mgr);
      loader.InitScriptObjects(ids, *mgr);
    }

    in.ReadInt32();
    mDefAudioTrack = rstl::string(in);
    {
      rstl::string trackKey = CInGameTweakManager::GetIdentifierForWorldDefaultMusic(mMlvlId);
      char volume = 127;
      if (gpTweakManager->HasTweakValue(trackKey)) {
        mDefAudioTrack = gpTweakManager->GetTweakValue(trackKey)->GetAudio().GetFileName();
        volume =
            CCast::ToInt8(127.f * gpTweakManager->GetTweakValue(trackKey)->GetAudio().GetVolume());
      }
      if (!CScriptStreamedMusic::IsDSPFile(mDefAudioTrack)) {
        CStreamAudioManager::SetDefaultAudio(mDefAudioTrack, 0.f, 0.f, volume);
      }
    }
    CWorldLayers::ReadWorldLayers(in, version, mMlvlId);
    mLoadToken = nullptr;
    mLoadBuf = nullptr;
    mBufSize = 0;
    mLoadPhase = kP_LoadingMap;
  }
  case kP_LoadingMap: {
    if (!mMapWorld->IsLoaded()) {
      return false;
    }
    if (mCurAreaId == kInvalidAreaId) {
      GetMapWorld()->SetWhichMapAreasLoaded(*this, 0, 9999);
    } else {
      GetMapWorld()->SetWhichMapAreasLoaded(*this, mCurAreaId.Value(), 3);
    }
    mLoadPhase = kP_LoadingMapAreas;
  }
  case kP_LoadingMapAreas: {
    if (mMapWorld->GetObject()->IsMapAreasStreaming()) {
      return false;
    }
    mLoadPhase = kP_LoadingSkyBox;
  }
  case kP_LoadingSkyBox: {
    mSkyboxActive = true;
    mSkyboxVisible = false;
    if (mSkyboxWorld) {
      if (!mSkyboxWorld->IsLoaded()) {
        return false;
      }
      CModel* skybox = mSkyboxWorld->GetObject();
      skybox->Touch(0);
      if (!skybox->IsLoaded(0)) {
        return false;
      }
      mSkyboxWorldLoaded = TLockedToken< CModel >(*mSkyboxWorld);
    }
    mLoadPhase = kP_Done;
  }
  case kP_Done:
    return true;
  default:
    break;
  }
  return false;
}

CWorld::~CWorld() {
  StopSounds();
  CWorldTransManager* transManager = gpGameState->WorldTransitionManager().GetPtr();
  if (transManager->GetTransType() != CWorldTransManager::kTT_Disabled &&
      gpMain->GetRestartMode() == CMain::kRM_None) {
    CStreamAudioManager::StopOneShot();
  } else if ((gpGameState->GetGameMode().GetGameModeType() != 'DTHM' &&
              gpGameState->GetGameMode().GetGameModeType() != 'COIN') ||
             !gpGameState->GetGameMode().IsGameOver()) {
    CStreamAudioManager::StopAll();
  }
}

bool CWorld::ScheduleAreaToLoad(CGameArea* area, CStateManager& mgr) {
  if (!area->IsLoaded()) {
    if (area->IsUnloading()) {
      return false;
    }
    MoveToChain(area, kC_Loading);
    return true;
  } else {
    if (area->GetCurChain() != kC_Alive) {
      if (area->GetCurChain() != kC_AliveJudgement) {
        mCurrentAreaNeedsAllocation = true;
      }
      MoveToChain(area, kC_Alive);
    }
    return false;
  }
}

bool CWorld::UnloadAllAreasExcept(CStateManager& mgr, TAreaId& aid) {
  CFrameDelayedKiller::StallAndFlushAllAllocations();
  bool failed = false;
  for (int i = 0; i < mAreas.size(); ++i) {
    CGameArea* area = Area(TAreaId(i));
    if (area->GetId() != aid && area->GetCurChain() != kC_Deallocated) {
      if (area->Invalidate(&mgr)) {
        MoveToChain(area, kC_Deallocated);
      } else {
        failed = true;
      }
    }
  }
  return !failed;
}

void CWorld::TravelToArea(const TAreaId& aid, CStateManager& mgr, EAreaTravelType travelType) {
  if (aid.Value() < 0 || aid.Value() >= mAreas.size())
    return;
  mCurrentAreaNeedsAllocation = false;
  mCurAreaId = aid;
  CGameArea* toDeallocateAreas = mChainHeads[kC_ToDeallocate];
  while (toDeallocateAreas) {
    if (toDeallocateAreas->Invalidate(&mgr)) {
      MoveToChain(toDeallocateAreas, kC_Deallocated);
      break;
    }
    toDeallocateAreas = toDeallocateAreas->GetNext();
  }

  CGameArea* aliveAreas = mChainHeads[kC_Alive];
  while (aliveAreas) {
    CGameArea* aliveArea = aliveAreas;
    aliveAreas = aliveAreas->GetNext();
    MoveToChain(aliveArea, kC_AliveJudgement);
  }
  CGameArea* loadingAreas = mChainHeads[kC_Loading];
  while (loadingAreas) {
    CGameArea* loadingArea = loadingAreas;
    loadingAreas = loadingAreas->GetNext();
    MoveToChain(loadingArea, kC_ToDeallocate);
  }

  CGameArea* const area = mAreas[aid.Value()].get();
  if (area->GetCurChain() != kC_AliveJudgement)
    mCurrentAreaNeedsAllocation = true;
  area->Validate(mgr);
  MoveToChain(area, kC_Alive);
  area->SetOcclusionState(CGameArea::kOS_Visible);

  bool otherLoading = false;
  CGameArea* otherLoadArea = nullptr;
  if (travelType == kATT_LoadAdjacent) {
    const int maxLoaded = area->GetDockCount();
    TAreaId* loadedIds = static_cast< TAreaId* >(alloca(maxLoaded * sizeof(TAreaId)));
    int loadedCount = 0;
    for (int i = 0; i < area->GetDockCount(); ++i) {
      IGameArea::Dock& dock = const_cast< IGameArea::Dock& >(area->GetDock(i));
      const int dockRefCount = dock.GetDockRefs().size();
      for (int j = 0; j < dockRefCount; ++j) {
        if (!dock.GetLoadOtherBlocked(j))
          continue;
        CGameArea* cArea = Area(dock.GetConnectedAreaId(j));
        if (cArea->GetPhase() == CGameArea::kP_Allocate && cArea->GetCurChain() == kC_Deallocated) {
          dock.SetLoadOtherBlocked(j, false);
        }
        if (loadedCount < maxLoaded) {
          loadedIds[loadedCount++] = cArea->GetId();
        }
      }
    }

    for (int i = 0; i < area->GetDockCount(); ++i) {
      const IGameArea::Dock& dock = area->GetDock(i);
      const int dockRefCount = dock.GetDockRefs().size();
      for (int j = 0; j < dockRefCount; ++j) {
        if (!dock.GetShouldLoadOther(j) || dock.GetLoadOtherBlocked(j))
          continue;
        CGameArea* cArea = Area(dock.GetConnectedAreaId(j));
        bool alreadyLoaded = false;
        for (int k = 0; k < loadedCount; ++k) {
          if (loadedIds[k] == cArea->GetId()) {
            alreadyLoaded = true;
            break;
          }
        }
        if (alreadyLoaded || !cArea->IsActive())
          continue;
        if (!otherLoading) {
          otherLoading = ScheduleAreaToLoad(cArea, mgr);
          if (!otherLoading)
            continue;
          otherLoadArea = cArea;
        } else
          ScheduleAreaToLoad(cArea, mgr);
      }
    }
  }
  int toStreamCount = 0;
  CGameArea* judgementAreas = mChainHeads[kC_AliveJudgement];
  while (judgementAreas) {
    CGameArea* judgementArea = judgementAreas;
    judgementAreas = judgementArea->GetNext();
    MoveToChain(judgementArea, kC_ToDeallocate);
  }

  toDeallocateAreas = mChainHeads[kC_ToDeallocate];
  while (toDeallocateAreas) {
    toDeallocateAreas->RemoveStaticGeometry();
    toDeallocateAreas = toDeallocateAreas->GetNext();
    ++toStreamCount;
  }

  if (!toStreamCount && otherLoadArea && !mLoadPaused)
    otherLoadArea->StartStreamIn(mgr);

  MapWorld()->SetWhichMapAreasLoaded(*this, aid.Value(), 3);
}

void CWorld::MoveToChain(CGameArea* area, EChain chain) {
  if (area->GetCurChain() == chain) {
    return;
  }

  if (area->GetCurChain() != kC_Invalid) {
    CGameArea*& head = mChainHeads[area->GetCurChain()];
    if (head == area) {
      head = area->GetNext();
    }
  }

  CGameArea*& newHead = mChainHeads[chain];
  area->SetChain(newHead, chain);
  newHead = area;
}

CMapWorld* CWorld::GetMapWorld() const { return mMapWorld->GetObject(); }

CAssetId CWorld::IGetWorldAssetId() const { return GetWorldAssetId(); }

CAssetId CWorld::IGetStringTableAssetId() const { return mStrgId; }

CAssetId CWorld::IGetDarkStringTableAssetId() const { return mDarkStrgId; }

CAssetId CWorld::IGetSaveWorldAssetId() const { return mSavwId; }

const CMapWorld* CWorld::IGetMapWorld() const { return GetMapWorld(); }

CMapWorld* CWorld::IMapWorld() { return GetMapWorld(); }

const IGameArea* CWorld::IGetAreaAlways(TAreaId id) const { return &GetAreaAlways(id); }

TAreaId CWorld::IGetCurrentAreaId() const { return mCurAreaId; }

bool CWorld::ICheckWorldComplete() {
  return CheckWorldComplete(nullptr, kInvalidAreaId, kInvalidAssetId);
}

rstl::string CWorld::IGetDefaultAudioTrack() const { return mDefAudioTrack; }

int CWorld::IGetAreaCount() const { return mAreas.size(); }

uint CWorld::IGetTempleKeyWorldIndex() const { return mTempleKeyWorldIndex; }

bool CWorld::ICancelLoad() { return true; }

CDummyWorld::CDummyWorld(CAssetId mlvlId, bool loadMap)
: mLoadMap(loadMap)
, mPhase(kP_Loading)
, mMlvlId(mlvlId)
, mSavwId(kInvalidAssetId)
, mAreas()
, mMapWorldId(kInvalidAssetId)
, mMapWorld()
, mLoadToken()
, mLoadBuf()
, mBufSize(0)
, mCurrentAreaId(kInvalidAreaId) {
  const SObjectTag mlvl('MLVL', mlvlId);
  mBufSize = gpResourceFactory->GetResLoader().ResourceSize(mlvl);
  mLoadBuf = static_cast< char* >(CMemory::Alloc(mBufSize, IAllocator::kHI_RoundUpLen));
  mLoadToken = gpResourceFactory->GetResLoader().LoadResourceAsync(mlvl, mLoadBuf.get());
}

CDummyWorld::~CDummyWorld() {}

bool CDummyWorld::ICheckWorldComplete() {
  switch (mPhase) {
  case kP_Loading: {
    if (!mLoadToken->IsComplete()) {
      return false;
    }

    CMemoryInStream r(mLoadBuf.get(), mBufSize);
    r.ReadInt32();
    int version = r.ReadInt32();
    mStrgId = r.ReadInt32();
    if (version >= 22u) {
      mDarkStrgId = r.ReadInt32();
    }
    if (version >= 23u) {
      mTempleKeyWorldIndex = r.ReadInt32();
    }
    mSavwId = r.ReadInt32();
    r.ReadInt32();
    if (version == 17u) {
      rstl::vector< CRelay > relays(r);
    }

    int areaCount = r.ReadInt32();
    if (version < 18u) {
      r.ReadInt32();
    }

    mAreas.reserve(areaCount);
    for (int i = 0; i < areaCount; ++i) {
      mAreas.push_back_unsafe(rs_new CDummyGameArea(r, i, version));
    }

    mMapWorldId = r.ReadInt32();
    if (mLoadMap) {
      mMapWorld = rs_new TCachedToken< CMapWorld >(
          gpSimplePool->GetObj(SObjectTag('MAPW', mMapWorldId)));
      mMapWorld->Lock();
    }

    if (version < 18u) {
      r.ReadUint8();
      r.ReadInt32();
    }

    int audioGroupCount = r.ReadInt32();
    for (int i = 0; i < audioGroupCount; ++i) {
      int groupId = r.ReadInt32();
      CAssetId agscId = r.ReadInt32();
    }

    {
      rstl::string s(r);
    }

    CWorldLayers::ReadWorldLayers(r, version, mMlvlId);

    mLoadToken = nullptr;
    mLoadBuf = nullptr;
    mBufSize = 0;

    if (!mLoadMap) {
      mPhase = kP_Done;
      break;
    }
    mPhase = kP_LoadingMap;
  }
  case kP_LoadingMap: {
    if (!mMapWorld->IsLoaded()) {
      return false;
    }

    IMapWorld()->SetWhichMapAreasLoaded(*this, 0, 9999);
    mPhase = kP_LoadingMapAreas;
  }
  case kP_LoadingMapAreas: {
    if (mMapWorld->GetObject()->IsMapAreasStreaming()) {
      return false;
    }

    mPhase = kP_Done;
  }
  case kP_Done:
    return true;
  default:
    break;
  }
  return false;
}

CAssetId CDummyWorld::IGetWorldAssetId() const { return mMlvlId; }

CAssetId CDummyWorld::IGetSaveWorldAssetId() const { return mSavwId; }

CAssetId CDummyWorld::IGetStringTableAssetId() const { return mStrgId; }

CAssetId CDummyWorld::IGetDarkStringTableAssetId() const { return mDarkStrgId; }

const CMapWorld* CDummyWorld::IGetMapWorld() const { return mMapWorld->GetObject(); }

CMapWorld* CDummyWorld::IMapWorld() { return mMapWorld->GetObject(); }

const IGameArea* CDummyWorld::IGetAreaAlways(TAreaId id) const { return &*mAreas[id.Value()]; }

TAreaId CDummyWorld::IGetCurrentAreaId() const { return mCurrentAreaId; }

TAreaId CDummyWorld::IGetAreaId(CAssetId id) const {
  if (id != kInvalidAssetId) {
    int areaCount = mAreas.size();
    for (int i = 0; i < areaCount; ++i) {
      if (IGetAreaAlways(TAreaId(i))->IGetAreaAssetId() == id) {
        return TAreaId(i);
      }
    }
  }
  return kInvalidAreaId;
}

rstl::string CDummyWorld::IGetDefaultAudioTrack() const { return rstl::string_l(""); }

int CDummyWorld::IGetAreaCount() const { return mAreas.size(); }

uint CDummyWorld::IGetTempleKeyWorldIndex() const { return mTempleKeyWorldIndex; }

bool CDummyWorld::ICancelLoad() {
  if (!mLoadToken.get()) {
    return true;
  }
  if (mLoadToken->IsComplete()) {
    return true;
  }
  mLoadToken->PostCancelRequest();
  return false;
}

void CWorld::TouchSky() const {
  if (mSkyboxWorldLoaded) {
    (*mSkyboxWorldLoaded)->Touch(0);
  }
  if (mSkyboxOverride) {
    (*mSkyboxOverride)->Touch(0);
  }
}

void CWorld::CancelLayerRelUnload(TAreaId aid, TLayerId layer) {
  rstl::list< SLayerRelUnload >::iterator it = mPendingLayerRelUnloads.begin();
  while (it != mPendingLayerRelUnloads.end()) {
    if (it->mAreaId == aid && it->mLayerId.Value() == layer.Value()) {
      it = mPendingLayerRelUnloads.erase(it);
    } else {
      ++it;
    }
  }
}

void CWorld::Update(float dt) {
  mNeededEnvFx = 0;
  bool needsSky = false;
  bool skyVisible = false;
  int areaCount = 0;
  CAssetId overrideSkyId = kInvalidAssetId;

  if (mPendingLayerRelUnloads.size() != 0) {
    rstl::list< SLayerRelUnload >::iterator it = mPendingLayerRelUnloads.begin();
    while (it != mPendingLayerRelUnloads.end()) {
      if (--it->mFramesLeft == 0) {
        if (DoesAreaExist(it->mAreaId) && Area(it->mAreaId)->IsLoaded()) {
          CGameArea* area = Area(it->mAreaId);
          rstl::vector< CRELFileToken >* tokens = area->GetLayerRelTokens(it->mLayerId);
          if (tokens) {
            *tokens = rstl::vector< CRELFileToken >();
          }
        }
        it = mPendingLayerRelUnloads.erase(it);
      } else {
        ++it;
      }
    }
  }

  const CScriptAreaProperties* skyAttrs = nullptr;
  for (CGameArea::CChainIterator it = ChainHead(kC_Alive); it != skGlobalNonConstEnd;
       ++it, ++areaCount) {
    it->AliveUpdate(dt);
    if (it->DoesAreaNeedSkyNow()) {
      const CScriptAreaProperties* attrs = it->GetPostConstructed()->mAreaAttributes;
      if (attrs) {
        if (attrs->GetSkyModel() != kInvalidAssetId) {
          overrideSkyId = attrs->GetSkyModel();
        }
        skyAttrs = attrs;
      }
      needsSky = true;
      if (it->GetOcclusionState() == CGameArea::kOS_Visible) {
        skyVisible = true;
      }
    }
    int envFx = it->DoesAreaNeedEnvFx();
    if (envFx != 0) {
      mNeededEnvFx = envFx;
    }
  }

  if (areaCount == 0) {
    return;
  }
  if (overrideSkyId != kInvalidAssetId && needsSky) {
    mSkyboxActive = true;
    mSkyboxVisible = skyVisible;
    mSkyboxOverride =
        TLockedToken< CModel >(gpSimplePool->GetObj(SObjectTag('CMDL', overrideSkyId)));
    mSkyboxWorldLoaded = rstl::optional_object< TLockedToken< CModel > >();
    mSkyboxFogMode = skyAttrs->GetSkyFogMode();
    mSkyboxFogColor = skyAttrs->GetSkyFogColor();
    mSkyboxFogStart = skyAttrs->GetSkyFogStart();
    mSkyboxFogEnd = skyAttrs->GetSkyFogEnd();
    if (mSkyboxWorld) {
      mSkyboxWorld->Unlock();
    }
  } else {
    mSkyboxOverride = rstl::optional_object< TLockedToken< CModel > >();
    mSkyboxFogMode = kRFM_None;
    if (!mSkyboxWorld) {
      mSkyboxActive = false;
      mSkyboxVisible = false;
    } else if (!needsSky) {
      mSkyboxWorldLoaded = rstl::optional_object< TLockedToken< CModel > >();
      mSkyboxWorld->Unlock();
      mSkyboxActive = false;
      mSkyboxVisible = false;
    } else {
      if (!mSkyboxWorldLoaded) {
        mSkyboxWorld->Lock();
        if (mSkyboxWorld->IsLoaded()) {
          CModel* skybox = mSkyboxWorld->GetObject();
          skybox->Touch(0);
          if (skybox->IsLoaded(0)) {
            mSkyboxWorldLoaded = TLockedToken< CModel >(*mSkyboxWorld);
          }
        }
      }
      mSkyboxActive = true;
      mSkyboxVisible = skyVisible;
    }
  }
}

void CWorld::PreRender() {
  const CScriptAreaProperties* skyAttrs = nullptr;
  for (CGameArea::CChainIterator it = ChainHead(kC_Alive); it != skGlobalNonConstEnd; ++it) {
    it->PreRender();
  }
}

void CWorld::DrawSky(const CTransform4f& xf, bool noFog) const {
  if ((mSkyboxWorldLoaded || mSkyboxOverride) && mSkyboxVisible) {
    if (!noFog) {
      CGraphics::SetFog(mSkyboxFogMode, mSkyboxFogStart, mSkyboxFogEnd, mSkyboxFogColor);
    }
    if (mSkyboxFogMode == kRFM_None) {
      CGraphics::SetClearColor(CColor(0));
    } else {
      CGraphics::SetClearColor(mSkyboxFogColor.WithAlphaOf(0.f));
    }
    CGraphics::DisableAllLights();
    gpRender->SetModelMatrix(xf);
    gpRender->SetAmbientColor(
        CColor(mSkyboxLightingLevel, mSkyboxLightingLevel, mSkyboxLightingLevel, 1.f));
    CGraphics::SetDepthRange(1.f, 1.f);
    const CModelFlags& flags = CModelFlags::Normal().DepthCompareUpdate(true, false);
    (*(mSkyboxOverride ? mSkyboxOverride : mSkyboxWorldLoaded))->Draw(flags);
    CGraphics::SetDepthRange(0.125f, 1.f);
    gpRender->SetModelMatrix(CTransform4f::Identity());
    if (!noFog) {
      CGraphics::SetFog(kRFM_None, 0.f, 0.f, CColor::Black());
    }
  }
}

bool CWorld::AreSkyNeedsMet() const {
  if (mSkyboxActive) {
    if (mSkyboxOverride) {
      return (*mSkyboxOverride)->IsLoaded(0);
    }
    if (mSkyboxWorldLoaded) {
      return (*mSkyboxWorldLoaded)->IsLoaded(0);
    }
    return false;
  }
  return true;
}

TAreaId CWorld::GetAreaId(CAssetId assetId) const {
  TAreaId result(-1);
  if (assetId != kInvalidAssetId) {
    int areaCount = mAreas.size();
    for (int i = 0; i < areaCount; ++i) {
      if (assetId == GetArea(TAreaId(i))->GetAreaAssetId()) {
        result = TAreaId(i);
        break;
      }
    }
  }
  return result;
}

TAreaId CWorld::IGetAreaId(CAssetId assetId) const { return GetAreaId(assetId); }

TAreaId CWorld::GetAreaIdForSaveId(uint saveId) const {
  TAreaId result(-1);
  if (saveId != kInvalidAssetId) {
    int areaCount = mAreas.size();
    for (int i = 0; i < areaCount; ++i) {
      if (saveId == GetArea(TAreaId(i))->GetAreaSaveId()) {
        result = TAreaId(i);
        break;
      }
    }
  }
  return result;
}

void CWorld::SetLoadPauseState(bool paused) {
  for (CGameArea::CConstChainIterator it = GetChainHead(kC_Loading); skGlobalEnd != it; ++it) {
    const_cast< CGameArea& >(*it).SetLoadPauseState(paused);
  }
  mLoadPaused = paused;
}

void CWorld::MoveAreaToChain3(TAreaId aid) { MoveToChain(Area(aid), kC_Alive); }

bool CWorld::HasGlobalSound(ushort soundId) const {
  for (rstl::reserved_vector< rstl::pair< ushort, CSfxHandle >, 10 >::const_iterator it = mGlobalSfxHandles.begin(); it != mGlobalSfxHandles.end(); ++it) {
    if (it->first == soundId) {
      return true;
    }
  }
  return false;
}

void CWorld::AddGlobalSound(ushort soundId, CSfxHandle handle) {
  if (mGlobalSfxHandles.size() >= mGlobalSfxHandles.capacity()) {
    return;
  }
  mGlobalSfxHandles.push_back(rstl::pair< ushort, CSfxHandle >(soundId, handle));
}

void CWorld::StopGlobalSound(ushort soundId) {
  for (rstl::reserved_vector< rstl::pair< ushort, CSfxHandle >, 10 >::iterator it = mGlobalSfxHandles.begin(); it != mGlobalSfxHandles.end(); ++it) {
    if (it->first == soundId) {
      CSfxManager::RemoveEmitter(it->second);
      mGlobalSfxHandles.erase(it);
      return;
    }
  }
}

void CWorld::StopSounds() {
  for (rstl::reserved_vector< rstl::pair< ushort, CSfxHandle >, 10 >::iterator it = mGlobalSfxHandles.begin(); it != mGlobalSfxHandles.end(); ++it) {
    CSfxManager::RemoveEmitter(it->second);
  }
  mGlobalSfxHandles.clear();
}

void CWorld::PauseAndUnpauseAreaLoading() {
  if (!mLoadPaused) {
    SetLoadPauseState(true);
    SetLoadPauseState(false);
  }
}
