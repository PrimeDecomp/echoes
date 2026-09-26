#include "MetroidPrime/CAutoMapper.hpp"

#include "GuiSys/CGuiTextPane.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CEulerAngles.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CMapArea.hpp"
#include "MetroidPrime/CMapUniverse.hpp"
#include "MetroidPrime/CMapWorld.hpp"
#include "MetroidPrime/CMapWorldInfo.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/IWorld.hpp"
#include "MetroidPrime/IGameArea.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CWorldState.hpp"
#include "MetroidPrime/Tweaks/CTweakAutoMapper.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"
#include "rstl/math.hpp"

// Work in progress: the map-loading, input and drawing bodies are not yet reconstructed.

static const char* const skFRME_MapScreen = "FRME_MapScreen";
static const char* const skFRME_MapScreenBackground = "FRME_MapScreenBackground";

// Guessed name
struct SMapKeyEntry {
  const char* mLabel;
  CPlayerState::EItemType mItemType;
  int mTempleWorldIndex;
};

static const SMapKeyEntry skMapKeys[] = {
    {"", CPlayerState::kIT_AgonKey1, 2},
    {"", CPlayerState::kIT_AgonKey2, 2},
    {"", CPlayerState::kIT_AgonKey3, 2},
    {"", CPlayerState::kIT_TorvusKey1, 3},
    {"", CPlayerState::kIT_TorvusKey2, 3},
    {"", CPlayerState::kIT_TorvusKey3, 3},
    {"", CPlayerState::kIT_HiveKey1, 4},
    {"", CPlayerState::kIT_HiveKey2, 4},
    {"", CPlayerState::kIT_HiveKey3, 4},
};

static inline float Lerp(float a, float b, float t) { return a * (1.f - t) + b * t; }

CAutoMapper::SAutoMapperRenderState::SAutoMapperRenderState(
    const CVector2i& viewportSize, const CQuaternion& camOrientation, float camDist, float camAngle,
    const CVector3f& areaPoint, float drawDepth1, float drawDepth2, float alphaSurfaceVisited,
    float alphaOutlineVisited, float alphaSurfaceUnvisited, float alphaOutlineUnvisited)
: mViewportSize(viewportSize)
, mCamOrientation(camOrientation)
, mCamDist(camDist)
, mCamAngle(camAngle)
, mAreaPoint(areaPoint)
, mDrawDepth1(drawDepth1)
, mDrawDepth2(drawDepth2)
, mAlphaSurfaceVisited(alphaSurfaceVisited)
, mAlphaOutlineVisited(alphaOutlineVisited)
, mAlphaSurfaceUnvisited(alphaSurfaceUnvisited)
, mAlphaOutlineUnvisited(alphaOutlineUnvisited)
, mViewportEase(kE_None)
, mCamEase(kE_None)
, mPointEase(kE_None)
, mDepth1Ease(kE_None)
, mDepth2Ease(kE_None)
, mAlphaEase(kE_None) {}

void CAutoMapper::SAutoMapperRenderState::InterpolateWithClamp(const SAutoMapperRenderState& a,
                                                               SAutoMapperRenderState& out,
                                                               const SAutoMapperRenderState& b,
                                                               float t) {
  const float ct = CMath::Clamp(0.f, t, 1.f);
  const float easeIn = CMath::Clamp(0.f, ct * ct * ct, 1.f);
  const float omt = 1.f - ct;
  const float easeOut = CMath::Clamp(0.f, 1.f - omt * omt * omt, 1.f);
  float easeInOut;
  if (ct < 0.5f) {
    easeInOut = CMath::Clamp(0.f, 1.f - (0.5f * CMath::SqrtF(2.f * omt - 1.f) + 0.5f), 1.f);
  } else {
    easeInOut = CMath::Clamp(0.f, 0.5f * CMath::SqrtF(2.f * ct - 1.f) + 0.5f, 1.f);
  }
  const float eases[5] = {0.f, ct, easeOut, easeIn, easeInOut};

  if (b.mViewportEase != kE_None) {
    out.mViewportSize = CVector2i::Lerp(a.mViewportSize, b.mViewportSize, eases[b.mViewportEase]);
  }
  if (b.mCamEase != kE_None) {
    const float ease = eases[b.mCamEase];
    out.mCamOrientation = CQuaternion::Slerp(a.mCamOrientation, b.mCamOrientation, ease);
    out.mCamDist = Lerp(a.mCamDist, b.mCamDist, ease);
    out.mCamAngle = Lerp(a.mCamAngle, b.mCamAngle, ease);
  }
  if (b.mPointEase != kE_None) {
    out.mAreaPoint = CVector3f::Lerp(a.mAreaPoint, b.mAreaPoint, eases[b.mPointEase]);
  }
  if (b.mDepth1Ease != kE_None) {
    out.mDrawDepth1 = Lerp(a.mDrawDepth1, b.mDrawDepth1, eases[b.mDepth1Ease]);
  }
  if (b.mDepth2Ease != kE_None) {
    out.mDrawDepth2 = Lerp(a.mDrawDepth2, b.mDrawDepth2, eases[b.mDepth2Ease]);
  }
  if (b.mAlphaEase != kE_None) {
    const float ease = eases[b.mAlphaEase];
    out.mAlphaSurfaceVisited = Lerp(a.mAlphaSurfaceVisited, b.mAlphaSurfaceVisited, ease);
    out.mAlphaOutlineVisited = Lerp(a.mAlphaOutlineVisited, b.mAlphaOutlineVisited, ease);
    out.mAlphaSurfaceUnvisited = Lerp(a.mAlphaSurfaceUnvisited, b.mAlphaSurfaceUnvisited, ease);
    out.mAlphaOutlineUnvisited = Lerp(a.mAlphaOutlineUnvisited, b.mAlphaOutlineUnvisited, ease);
  }
}

void CAutoMapper::SAutoMapperRenderState::ResetInterpolation() {
  mViewportEase = kE_None;
  mCamEase = kE_None;
  mPointEase = kE_None;
  mDepth1Ease = kE_None;
  mDepth2Ease = kE_None;
  mAlphaEase = kE_None;
}

CAutoMapper::SAutoMapperHintLocation::SAutoMapperHintLocation(uint showBeacon, float beaconAlpha,
                                                              CAssetId worldId, int areaId)
: mShowBeacon(showBeacon), mBeaconAlpha(beaconAlpha), mWorldId(worldId), mAreaId(areaId) {}

CAutoMapper::CAutoMapper(const CStateManager& mgr, int playerIndex)
: mPlayerIndex(playerIndex)
, mLoadPhase(kLP_LoadResources)
, mMapu(gpSimplePool->GetObj("MAPU_MapUniverse"))
, mDummyWorlds()
, mWorldsPendingUnload()
, mWorld(const_cast< CWorld* >(mgr.GetWorld()))
, mFrmeMapScreen()
, mFrmeInitialized(nullptr)
, mFrmeMapScreenBackground()
, mFrmeBackgroundInitialized(nullptr)
, mMiniMapSamus(gpSimplePool->GetObj("CMDL_MiniMapSamus"))
, mHintBeacon(gpSimplePool->GetObj("TXTR_HintBeacon"))
, mCompassModel(gpSimplePool->GetObj("CMDL_CompassModel"))
, mCompassShellModel(gpSimplePool->GetObj("CMDL_CompassShellModel"))
, mMapIcons()
, mAreaHintDescId(kInvalidAssetId)
, mAreaHintDesc()
, mMapAreaStringId(kInvalidAssetId)
, mMapAreaString()
, mWorldIdx(0)
, mCurAreaId(mWorld->IGetCurrentAreaId())
, mOtherAreaId(mCurAreaId)
, mRenderState0(BuildMiniMapWorldRenderState(
      mgr,
      CQuaternion::FromMatrix(
          mgr.GetCameraManager(playerIndex)->GetCurrentCamera(mgr, 1)->GetTransform()),
      mCurAreaId.value))
, mRenderState1(mRenderState0)
, mRenderState2(mRenderState0)
, mState(kAMS_MiniMap)
, mNextState(kAMS_MiniMap)
, mInterpDur(0.f)
, mInterpTime(0.f)
, mMapMode(kMM_Normal)
, mPanningSfx()
, mRotatingSfx()
, mZoomingSfx()
, mFlashTimer(0.f)
, mPlayerFlashPulse(0.f)
, mHintSteps()
, mHintLocations()
, mLstick()
, mCstick()
, mLtrigger()
, mRtrigger()
, mAbutton()
, mLStickPos(0)
, mRStickPos(0)
, mLTriggerPos(0)
, mRTriggerPos(0)
, mAButtonPos(0)
, mTextpaneLabel(nullptr)
, mTextpaneAreaname(nullptr)
, mTextpaneHint(nullptr)
, mTextpaneInstructions(nullptr)
, mTextpaneInstructions1(nullptr)
, mTextpaneInstructions2(nullptr)
, mTextpaneLeft(nullptr)
, mTextpaneRight(nullptr)
, mTextpaneYicon(nullptr)
, mTextpaneMapLegend(nullptr)
, mTextpaneMapLegend1(nullptr)
, mTextpaneXicon(nullptr)
, mTextpaneRight3(nullptr)
, mTextpaneXicon1(nullptr)
, mTextpaneKeylegend(nullptr)
, mBasewidgetLeftPane(nullptr)
, mBasewidgetYButtonPane(nullptr)
, mBasewidgetBottomPane(nullptr)
, mBasewidgetHintgroup(nullptr)
, mBackgroundHexagons()
, mBackgroundAnimationPhase(0.f)
, mLeftPanePos(0.f)
, mYButtonPanePos(0.f)
, mBottomPanePos(0.f)
, mDarkWorldBlend(mgr.GetIsDarkWorld() ? 1.f : 0.f)
, mZoomState(kZS_None)
, mTransitionState(kTS_Idle)
, mLoadingDummyWorld(false) {
  mMapu.Lock();
  mMiniMapSamus.Lock();
  mHintBeacon.Lock();
  mCompassModel.Lock();
  mCompassShellModel.Lock();

  mMapIcons.push_back(
      gpSimplePool->GetObj(SObjectTag('TXTR', gpTweakPlayerRes->GetSaveStationIcon())));
  mMapIcons.push_back(
      gpSimplePool->GetObj(SObjectTag('TXTR', gpTweakPlayerRes->GetMissileStationIcon())));
  mMapIcons.push_back(
      gpSimplePool->GetObj(SObjectTag('TXTR', gpTweakPlayerRes->GetElevatorIcon())));
  mMapIcons.push_back(gpSimplePool->GetObj(SObjectTag('TXTR', gpTweakPlayerRes->GetPortalIcon())));
  mMapIcons.push_back(
      gpSimplePool->GetObj(SObjectTag('TXTR', gpTweakPlayerRes->GetDownArrowIcon())));
  mMapIcons.push_back(gpSimplePool->GetObj(SObjectTag('TXTR', gpTweakPlayerRes->GetUpArrowIcon())));
  mMapIcons.push_back(
      gpSimplePool->GetObj(SObjectTag('TXTR', gpTweakPlayerRes->GetTranslatorDoorIcon())));
  for (CToken* it = mMapIcons.begin(); it != mMapIcons.end(); ++it) {
    it->Lock();
  }

  for (int i = 0; i < 9; ++i) {
    mLstick.push_back(gpSimplePool->GetObj(SObjectTag('TXTR', gpTweakPlayerRes->mLStick[i])));
    mCstick.push_back(gpSimplePool->GetObj(SObjectTag('TXTR', gpTweakPlayerRes->mCStick[i])));
  }
  for (int i = 0; i < 2; ++i) {
    mLtrigger.push_back(gpSimplePool->GetObj(SObjectTag('TXTR', gpTweakPlayerRes->mLTrigger[i])));
    mRtrigger.push_back(gpSimplePool->GetObj(SObjectTag('TXTR', gpTweakPlayerRes->mRTrigger[i])));
    mAbutton.push_back(gpSimplePool->GetObj(SObjectTag('TXTR', gpTweakPlayerRes->mAButton[i])));
  }

  UpdateTempleKeys(mgr);
}

template < class T >
void CAutoMapper::SetResLockState(T& list, bool lock) {
  for (typename T::iterator it = list.begin(); it != list.end(); ++it) {
    if (lock) {
      it->Lock();
    } else {
      it->Unlock();
    }
  }
}

CAutoMapper::~CAutoMapper() { CSfxManager::KillAll(CSfxManager::kSC_PauseScreen); }

bool CAutoMapper::CheckLoadComplete() {
  switch (mLoadPhase) {
  case kLP_LoadResources: {
    for (const CToken* it = mMapIcons.begin(); it != mMapIcons.end(); ++it) {
      if (!it->IsLoaded()) {
        return false;
      }
    }
    if (mMiniMapSamus.IsLoaded() && mHintBeacon.IsLoaded() && mCompassModel.IsLoaded() &&
        mCompassShellModel.IsLoaded()) {
      mLoadPhase = kLP_LoadUniverse;
    } else {
      return false;
    }
  }
  // Fall through after the map resources are ready.
  case kLP_LoadUniverse:
    if (mMapu.IsLoaded()) {
      const int numWorlds = mMapu.GetObject()->GetNumMapWorldDatas();
      mDummyWorlds =
          rstl::vector< rstl::auto_ptr< IWorld > >(numWorlds, rstl::auto_ptr< IWorld >());
      SetCurWorldAssetId(mWorld->IGetWorldAssetId());
      mLoadPhase = kLP_Done;
    } else {
      return false;
    }
    // Fall through.
  case kLP_Done:
    return true;
  default:
    return false;
  }
}

void CAutoMapper::SetupTeleportNavigation() {
  mHintSteps.clear();
  mHintSteps.push_back(SAutoMapperHintStep(SAutoMapperHintStep::kHST_SwitchToUniverse, 0));
  mHintSteps.push_back(SAutoMapperHintStep(SAutoMapperHintStep::kHST_ZoomOut, 0));
}

void CAutoMapper::OnNewInGameGuiState(EInGameGuiState state, CStateManager& mgr) {
  if (state == kIGGS_MapScreen) {
    CMain::EnsureWorldPaksReady();
    CWorld* world = mgr.World();
    world->GetMapWorld()->SetWhichMapAreasLoaded(*world, 0, 9999);
    if (mMapMode == kMM_Teleport) {
      SetupTeleportNavigation();
    } else {
      SetupHintNavigation();
    }
    BeginMapperStateTransition(kAMS_MapScreen, mgr);

    mFrmeMapScreen = rs_new TCachedToken< CGuiFrame >(gpSimplePool->GetObj(skFRME_MapScreen));
    mFrmeMapScreen->Lock();
    mFrmeMapScreenBackground =
        rs_new TCachedToken< CGuiFrame >(gpSimplePool->GetObj(skFRME_MapScreenBackground));
    mFrmeMapScreenBackground->Lock();
    SetResLockState(mLstick, true);
    SetResLockState(mCstick, true);
    SetResLockState(mLtrigger, true);
    SetResLockState(mRtrigger, true);
    SetResLockState(mAbutton, true);
  } else {
    CMain::EnsureWorldPakReady(gpGameState->CurrentWorldAssetId());
    if (mState == kAMS_MapScreenUniverse || mWorld == mgr.GetWorld()) {
      BeginMapperStateTransition(kAMS_MiniMap, mgr);
      mTransitionState = kTS_Idle;
    }
    LeaveMapScreenState();
  }
}

bool CAutoMapper::CanLeaveMapScreen(const CStateManager& mgr) const {
  return mTransitionState == kTS_MiniMapReady && CanLeaveMapScreenInternal(mgr);
}

bool CAutoMapper::CanLeaveMapScreenInternal(const CStateManager& mgr) const {
  if (!NotHintNavigating(mgr)) {
    return false;
  }
  if (IsRenderStateInterpolating()) {
    return false;
  }
  if (IsInMapperState(kAMS_MapScreenUniverse)) {
    return true;
  }
  return mWorld == mgr.GetWorld() && IsInMapperState(kAMS_MapScreen);
}

bool CAutoMapper::NotHintNavigating(const CStateManager& mgr) const {
  if (mHintSteps.size() > 0 &&
      mHintSteps.front().mType != SAutoMapperHintStep::kHST_LeaveMapScreen) {
    return false;
  }
  return true;
}

void CAutoMapper::UnmuteAllLoopedSounds() {
  CSfxManager::SfxVolume(mPanningSfx, 127);
  CSfxManager::SfxVolume(mRotatingSfx, 127);
  CSfxManager::SfxVolume(mZoomingSfx, 127);
}

bool CAutoMapper::HasCurrentMapUniverseWorld(const CStateManager& mgr) const {
  const CMapUniverse* mapu = mMapu.GetObject();
  const CAssetId worldId = mWorld->IGetWorldAssetId();
  for (int i = 0; i < mapu->GetNumMapWorldDatas(); ++i) {
    if (mapu->GetMapWorldData(i).GetWorldAssetId() == worldId) {
      return true;
    }
  }
  return false;
}

bool CAutoMapper::CheckDummyWorldLoad(CStateManager& mgr) {
  const uint worldIdx = mWorldIdx;
  IWorld* dummyWorld = mDummyWorlds[worldIdx].get();
  const CMapUniverse::CMapWorldData& worldData = mMapu.GetObject()->GetMapWorldData(worldIdx);
  if (dummyWorld != nullptr) {
    if (dummyWorld->ICheckWorldComplete()) {
      const CAssetId worldId = dummyWorld->IGetWorldAssetId();
      CMapWorldInfo* info = gpGameState->StateForWorld(worldId).GetMapWorldInfo().GetPtr();
      const CVector3f& localPoint =
          worldData.GetWorldTransform().GetQuickInverse() * mRenderState0.mAreaPoint;
      const CMatrix3f camRot(mRenderState0.mCamOrientation.BuildTransform());
      const CUnitVector3f camDir(camRot.GetColumn(kDY), CUnitVector3f::kN_No);
      const int areaId = FindClosestVisibleArea(localPoint, camDir, mgr, *dummyWorld, *info);
      if (areaId != -1) {
        if (mMapMode == kMM_Teleport) {
          const int teleportArea = FindTeleportArea(*dummyWorld->IGetMapWorld());
          if (teleportArea != -1) {
            SetCurAreaId(teleportArea);
            mgr.SetMapTeleportWorldId(worldId);
            dummyWorld->IMapWorld()->RecalculateWorldSphere(*info, *dummyWorld);
            mWorld = dummyWorld;
            UpdateTempleKeys(mgr);
            BeginMapperStateTransition(kAMS_MapScreen, mgr);
            mHintSteps.push_back(SAutoMapperHintStep(SAutoMapperHintStep::kHST_ShowBeacon, 1.f));
            mHintSteps.push_back(SAutoMapperHintStep(SAutoMapperHintStep::kHST_LeaveMapScreen, 0));
          } else {
            if (!dummyWorld->IGetMapWorld()->IsMapAreasStreaming()) {
              mLoadingDummyWorld = false;
            }
            return false;
          }
        } else {
          SetCurAreaId(areaId);
          dummyWorld->IMapWorld()->RecalculateWorldSphere(*info, *dummyWorld);
          mWorld = dummyWorld;
          UpdateTempleKeys(mgr);
          BeginMapperStateTransition(kAMS_MapScreen, mgr);
        }
        mLoadingDummyWorld = false;
        return true;
      }
      mLoadingDummyWorld = false;
      return false;
    }
    return true;
  }
  mLoadingDummyWorld = false;
  return false;
}

bool CAutoMapper::TryLeaveMapScreen(CStateManager& mgr) {
  if (CanLeaveMapScreenInternal(mgr)) {
    LeaveMapScreen(mgr);
    if (mMapMode == kMM_Teleport) {
      mgr.SetMapTeleportWorldId(mgr.GetWorld()->GetWorldAssetId());
    }
  } else if (NotHintNavigating(mgr)) {
    BeginMapperStateTransition(kAMS_MapScreenUniverse, mgr);
    mTransitionState = kTS_LeaveViaUniverse;
  } else {
    return false;
  }
  return true;
}

bool CAutoMapper::SwitchLightDarkWorld() {
  if (mDarkWorldBlend == 0.f) {
    mTransitionState = kTS_SwitchToDarkWorld;
    CSfxManager::SfxStart(0x21fe, 127, 64);
    if (mTextpaneLabel != nullptr) {
      mTextpaneLabel->TextSupport().SetText(gpStringTable->GetString("MapScreenTitleDark"));
    }
  } else if (mDarkWorldBlend == 1.f) {
    mTransitionState = kTS_SwitchToLightWorld;
    CSfxManager::SfxStart(0x21fd, 127, 64);
    if (mTextpaneLabel != nullptr) {
      mTextpaneLabel->TextSupport().SetText(gpStringTable->GetString("MapScreenTitle"));
    }
  } else {
    return false;
  }
  return true;
}

void CAutoMapper::BeginMapperStateTransition(EAutoMapperState state, CStateManager& mgr) {
  if (state == mNextState) {
    return;
  }

  if ((state == kAMS_MiniMap && mNextState != kAMS_MiniMap) ||
      (state != kAMS_MiniMap && mNextState == kAMS_MiniMap)) {
    CSfxManager::KillAll(CSfxManager::kSC_PauseScreen);
  }

  mState = mNextState;
  mNextState = state;
  mRenderState2 = mRenderState0;
  mRenderState1 = mRenderState0;

  if (mState == kAMS_MiniMap && state == kAMS_MapScreen) {
    mRenderState1 = BuildMapScreenWorldRenderState(mgr, mRenderState0.mCamOrientation,
                                                 mCurAreaId.Value(), false);
    ResetInterpolationTimer(gpTweakAutoMapper->GetOpenMapScreenTime());
  } else if (mState == kAMS_MapScreen && state == kAMS_MiniMap) {
    SetCurAreaId(mWorld->IGetCurrentAreaId().Value());
    mRenderState1 =
        BuildMiniMapWorldRenderState(mgr, mRenderState0.mCamOrientation, mCurAreaId.Value());
    ResetInterpolationTimer(gpTweakAutoMapper->GetCloseMapScreenTime());
    mHintLocations.clear();
  } else if (mState == kAMS_MapScreen && state == kAMS_MapScreenUniverse) {
    CSfxManager::SfxStart(0x679, 127, 64);
    mRenderState1 =
        BuildMapScreenUniverseRenderState(mgr, mRenderState0.mCamOrientation, mCurAreaId.Value());
    TransformRenderStatesWorldToUniverse();
    ResetInterpolationTimer(gpTweakAutoMapper->GetSwitchToFromUniverseTime());
  } else if (mState == kAMS_MapScreenUniverse && state == kAMS_MapScreen) {
    CSfxManager::SfxStart(0x67a, 127, 64);
    mRenderState1 = BuildMapScreenWorldRenderState(
        mgr, mRenderState0.mCamOrientation, mCurAreaId.Value(), mHintSteps.size() > 0);
    TransformRenderStateWorldToUniverse(mRenderState1);
    ResetInterpolationTimer(gpTweakAutoMapper->GetSwitchToFromUniverseTime());
    for (int i = 0; i < mDummyWorlds.size(); ++i) {
      if (mDummyWorlds[i].get() != mWorld || mWorld == mgr.GetWorld()) {
        mDummyWorlds[i] = rstl::auto_ptr< IWorld >();
      }
    }
  } else if (mState == kAMS_MapScreenUniverse && state == kAMS_MiniMap) {
    mWorld = mgr.World();
    SetCurAreaId(mWorld->IGetCurrentAreaId().Value());
    mRenderState1 =
        BuildMiniMapWorldRenderState(mgr, mRenderState0.mCamOrientation, mCurAreaId.Value());
    SetCurWorldAssetId(mWorld->IGetWorldAssetId());
    TransformRenderStateWorldToUniverse(mRenderState1);
    ResetInterpolationTimer(gpTweakAutoMapper->GetCloseMapScreenTime());
    mHintLocations.clear();
    for (int i = 0; i < mDummyWorlds.size(); ++i) {
      mDummyWorlds[i] = rstl::auto_ptr< IWorld >();
    }
  }
}

void CAutoMapper::CompleteMapperStateTransition(CStateManager& mgr) {
  if (mState == kAMS_MapScreenUniverse) {
    TransformRenderStatesUniverseToWorld();
  }

  if (mNextState == kAMS_MapScreen) {
    const CMapWorld* mapWorld = mWorld->IGetMapWorld();
    mapWorld->IsMapAreasStreaming();
    CMapWorldInfo* info =
        gpGameState->StateForWorld(mWorld->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();
    mapWorld->RecalculateWorldSphere(*info, *mWorld);
    mFlashTimer = 0.f;
    mPlayerFlashPulse = 0.f;
  }

  if (mNextState == kAMS_MiniMap) {
    mFrmeMapScreen = nullptr;
    mFrmeInitialized = nullptr;
    mFrmeMapScreenBackground = nullptr;
    mFrmeBackgroundInitialized = nullptr;
    mTextpaneHint = nullptr;
    mBasewidgetHintgroup = nullptr;
    mTextpaneInstructions = nullptr;
    mTextpaneInstructions1 = nullptr;
    mTextpaneInstructions2 = nullptr;
    mTextpaneAreaname = nullptr;
    mTextpaneLabel = nullptr;
    mTextpaneLeft = nullptr;
    mTextpaneRight = nullptr;
    mTextpaneMapLegend = nullptr;
    mTextpaneMapLegend1 = nullptr;
    mTextpaneYicon = nullptr;
    mBasewidgetLeftPane = nullptr;
    mBasewidgetYButtonPane = nullptr;
    mBasewidgetBottomPane = nullptr;
    mTextpaneXicon1 = nullptr;
    mTextpaneRight3 = nullptr;
    mTextpaneKeylegend = nullptr;
    mTextpaneXicon = nullptr;
    mTextpaneTeleporter = nullptr;
    mBackgroundHexagons.clear();
    SetResLockState(mLstick, false);
    SetResLockState(mCstick, false);
    SetResLockState(mLtrigger, false);
    SetResLockState(mRtrigger, false);
    SetResLockState(mAbutton, false);
  }

  if (mNextState == kAMS_MapScreenUniverse && mTransitionState == kTS_LeaveViaUniverse) {
    LeaveMapScreen(mgr);
  }
  mState = mNextState;
}

void CAutoMapper::ResetInterpolationTimer(float duration) {
  mInterpDur = duration;
  mInterpTime = 0.f;
}

CAutoMapper::SAutoMapperRenderState
CAutoMapper::BuildMiniMapWorldRenderState(const CStateManager& mgr, const CQuaternion& rot,
                                         int areaId) const {
  const CTweakAutoMapper* tweak = gpTweakAutoMapper.get();
  SAutoMapperRenderState ret(
      GetMiniMapViewportSize(),
      CQuaternion::MadeLocalToFirst(rot, GetMiniMapCameraOrientation(mgr)),
      tweak->GetMiniCamDistance(), tweak->GetMiniCamAngle(), GetAreaPointOfInterest(mgr, areaId),
      GetMapAreaMiniMapDrawDepth(), GetMapAreaMiniMapDrawDepth(),
      GetMapAreaMiniMapDrawAlphaSurfaceVisited(mgr), GetMapAreaMiniMapDrawAlphaOutlineVisited(mgr),
      GetMapAreaMiniMapDrawAlphaSurfaceUnvisited(mgr),
      GetMapAreaMiniMapDrawAlphaOutlineUnvisited(mgr));
  ret.mViewportEase = SAutoMapperRenderState::kE_Out;
  ret.mCamEase = SAutoMapperRenderState::kE_Out;
  ret.mPointEase = SAutoMapperRenderState::kE_Out;
  ret.mDepth1Ease = SAutoMapperRenderState::kE_Linear;
  ret.mDepth2Ease = SAutoMapperRenderState::kE_In;
  ret.mAlphaEase = SAutoMapperRenderState::kE_Linear;
  return ret;
}

CAutoMapper::SAutoMapperRenderState
CAutoMapper::BuildMapScreenWorldRenderState(const CStateManager& mgr, const CQuaternion& rot,
                                           int areaId, bool doingHint) const {
  const CTweakAutoMapper* tweak = gpTweakAutoMapper.get();
  const float camDist = doingHint ? tweak->GetMaxCamDistance() : tweak->GetCameraDistance();
  SAutoMapperRenderState ret(
      GetMapScreenViewportSize(), rot, camDist, tweak->GetCamAngle(),
      GetAreaPointOfInterest(mgr, areaId), GetMapAreaMaxDrawDepth(mgr, areaId),
      GetMapAreaMaxDrawDepth(mgr, areaId), tweak->GetAlphaSurfaceVisited(),
      tweak->GetAlphaOutlineVisited(), tweak->GetAlphaSurfaceUnvisited(),
      tweak->GetAlphaOutlineUnvisited());
  ret.mViewportEase = SAutoMapperRenderState::kE_Out;
  ret.mCamEase = SAutoMapperRenderState::kE_Linear;
  ret.mPointEase = SAutoMapperRenderState::kE_Out;
  ret.mDepth1Ease = SAutoMapperRenderState::kE_Linear;
  ret.mDepth2Ease = SAutoMapperRenderState::kE_Out;
  ret.mAlphaEase = SAutoMapperRenderState::kE_Linear;
  return ret;
}

CAutoMapper::SAutoMapperRenderState::SAutoMapperRenderState(const SAutoMapperRenderState& other)
: mViewportSize(other.mViewportSize)
, mCamOrientation(other.mCamOrientation)
, mCamDist(other.mCamDist)
, mCamAngle(other.mCamAngle)
, mAreaPoint(other.mAreaPoint)
, mDrawDepth1(other.mDrawDepth1)
, mDrawDepth2(other.mDrawDepth2)
, mAlphaSurfaceVisited(other.mAlphaSurfaceVisited)
, mAlphaOutlineVisited(other.mAlphaOutlineVisited)
, mAlphaSurfaceUnvisited(other.mAlphaSurfaceUnvisited)
, mAlphaOutlineUnvisited(other.mAlphaOutlineUnvisited)
, mViewportEase(other.mViewportEase)
, mCamEase(other.mCamEase)
, mPointEase(other.mPointEase)
, mDepth1Ease(other.mDepth1Ease)
, mDepth2Ease(other.mDepth2Ease)
, mAlphaEase(other.mAlphaEase) {}

CAutoMapper::SAutoMapperRenderState
CAutoMapper::BuildMapScreenUniverseRenderState(const CStateManager& mgr, const CQuaternion& rot,
                                              int areaId) const {
  const CTweakAutoMapper* tweak = gpTweakAutoMapper.get();
  SAutoMapperRenderState ret(
      GetMapScreenViewportSize(), rot, tweak->GetUniverseCamDistance(), tweak->GetCamAngle(),
      GetAreaPointOfInterest(mgr, areaId), GetMapAreaMaxDrawDepth(mgr, areaId),
      GetMapAreaMaxDrawDepth(mgr, areaId), 0.f, 0.f, 0.f, 0.f);
  ret.mViewportEase = SAutoMapperRenderState::kE_Out;
  ret.mCamEase = SAutoMapperRenderState::kE_Linear;
  ret.mPointEase = SAutoMapperRenderState::kE_Out;
  ret.mDepth1Ease = SAutoMapperRenderState::kE_Linear;
  ret.mDepth2Ease = SAutoMapperRenderState::kE_Out;
  ret.mAlphaEase = SAutoMapperRenderState::kE_Linear;
  return ret;
}

void CAutoMapper::SetShouldPanningSoundBePlaying(bool shouldBePlaying) {
  if (shouldBePlaying) {
    if (!mPanningSfx) {
      mPanningSfx = CSfxManager::SfxStart(0x12c, 127, 64, CSfxManager::kAllAreas, false, true);
    }
  } else {
    CSfxManager::SfxStop(mPanningSfx);
    mPanningSfx.Clear();
  }
}

void CAutoMapper::SetShouldZoomingSoundBePlaying(bool shouldBePlaying) {
  if (shouldBePlaying) {
    if (!mZoomingSfx) {
      mZoomingSfx = CSfxManager::SfxStart(0x78, 127, 64, CSfxManager::kAllAreas, false, true);
    }
  } else {
    CSfxManager::SfxStop(mZoomingSfx);
    mZoomingSfx.Clear();
  }
}

void CAutoMapper::SetShouldRotatingSoundBePlaying(bool shouldBePlaying) {
  if (shouldBePlaying) {
    if (!mRotatingSfx) {
      mRotatingSfx = CSfxManager::SfxStart(0x77, 127, 64, CSfxManager::kAllAreas, false, true);
    }
  } else {
    CSfxManager::SfxStop(mRotatingSfx);
    mRotatingSfx.Clear();
  }
}

void CAutoMapper::LeaveMapScreenState() {
  SetShouldPanningSoundBePlaying(false);
  SetShouldZoomingSoundBePlaying(false);
  SetShouldRotatingSoundBePlaying(false);
}

CQuaternion CAutoMapper::GetMiniMapCameraOrientation(const CStateManager& mgr) const {
  const float miniCamXAngle = gpTweakAutoMapper->GetMiniCamXAngle();
  const CGameCamera* camera = mgr.GetCameraManager(mPlayerIndex)->GetCurrentCamera(mgr, 1);
  const CEulerAngles angles =
      CEulerAngles::FromQuaternion(CQuaternion::FromMatrix(camera->GetTransform()));
  return CQuaternion::ZRotation(CRelAngle::FromRadians(CMath::ClampRadians(angles.GetYaw()))) *
         CQuaternion::XRotation(CRelAngle::FromDegrees(miniCamXAngle));
}

CVector3f CAutoMapper::GetAreaPointOfInterest(const CStateManager& mgr, int areaId) const {
  const IWorld& world = *mWorld;
  CMapArea* area = world.IGetMapWorld()->GetMapArea(areaId);
  return area->GetAreaPostTransform(world, areaId) * area->GetAreaCenterPoint();
}

int CAutoMapper::FindClosestVisibleArea(const CVector3f& point, const CUnitVector3f& camDir,
                                        const CStateManager& mgr, const IWorld& world,
                                        const CMapWorldInfo& info) const {
  const CMapWorld* mapWorld = world.IGetMapWorld();
  int closestArea = -1;
  int closestOtherWorldArea = -1;
  float minDistance = 3.402823466e+38f;
  float minOtherWorldDistance = minDistance;
  const rstl::vector< int > areas = mapWorld->GetVisibleAreas(world, info);
  const bool inDarkWorld = mDarkWorldBlend >= 0.5f;

  for (int i = 0; i < areas.size(); ++i) {
    const int areaId = areas[i];
    CMapArea* area = mapWorld->GetMapArea(areaId);
    const bool otherWorld = area->IsInDarkWorld() != inDarkWorld;
    const CTransform4f xf = area->GetAreaPostTransform(world, areaId);
    const CVector3f areaPoint = xf * area->GetAreaCenterPoint();
    const CVector3f pointToArea = areaPoint - point;
    const CVector3f projectedPoint =
        pointToArea.CanBeNormalized()
            ? point +
                  (pointToArea.Magnitude() * CVector3f::Dot(pointToArea.AsNormalized(), camDir)) *
                      camDir
            : point;
    const float distance = (projectedPoint - areaPoint).Magnitude();

    if (otherWorld) {
      if (distance < minOtherWorldDistance) {
        closestOtherWorldArea = areaId;
        minOtherWorldDistance = distance;
      }
    } else if (distance < minDistance) {
      closestArea = areaId;
      minDistance = distance;
    }
  }

  return closestArea != -1 ? closestArea : closestOtherWorldArea;
}

int CAutoMapper::FindTeleportArea(const CMapWorld& world) const {
  for (int areaId = 0; areaId < static_cast< int >(world.GetNumAreas()); ++areaId) {
    const CMapArea* area = world.GetMapArea(areaId);
    for (int i = 0; i < area->GetNumMappableObjects(); ++i) {
      if (area->GetMappableObject(i).GetType() == CMappableObject::kMOT_Teleporter) {
        return areaId;
      }
    }
  }
  return -1;
}

CVector2i CAutoMapper::GetMiniMapViewportSize() {
  const float scaleX = static_cast< float >(CGraphics::GetViewport().mWidth) / 640.f;
  const float scaleY = static_cast< float >(CGraphics::GetViewport().mHeight) / 480.f;
  const CVector2f size = gpTweakAutoMapper->GetMiniMapViewportSize();
  return CVector2i(static_cast< int >(scaleX * size.GetX()),
                   static_cast< int >(scaleY * size.GetY()));
}

CVector2i CAutoMapper::GetMapScreenViewportSize() {
  return CVector2i(CGraphics::GetViewport().mWidth, CGraphics::GetViewport().mHeight);
}

float CAutoMapper::GetMapAreaMiniMapDrawDepth() { return 2.f; }

float CAutoMapper::GetMapAreaMaxDrawDepth(const CStateManager& mgr, int areaId) const {
  return static_cast< float >(mWorld->IGetMapWorld()->GetCurrentMapAreaDepth(*mWorld, areaId));
}

float CAutoMapper::GetMapAreaMiniMapDrawAlphaSurfaceVisited(const CStateManager& mgr) const {
  const float interp = gpTweakGui->GetMapAlphaInterpolant();
  return gpTweakAutoMapper->GetMiniAlphaSurfaceVisited() *
         ((1.f - interp) * mgr.GetPlayer(mPlayerIndex)->GetGunAlpha() + interp);
}

float CAutoMapper::GetMapAreaMiniMapDrawAlphaOutlineVisited(const CStateManager& mgr) const {
  const float interp = gpTweakGui->GetMapAlphaInterpolant();
  return gpTweakAutoMapper->GetMiniAlphaOutlineVisited() *
         ((1.f - interp) * mgr.GetPlayer(mPlayerIndex)->GetGunAlpha() + interp);
}

float CAutoMapper::GetMapAreaMiniMapDrawAlphaSurfaceUnvisited(const CStateManager& mgr) const {
  const float interp = gpTweakGui->GetMapAlphaInterpolant();
  return gpTweakAutoMapper->GetMiniAlphaSurfaceUnvisited() *
         ((1.f - interp) * mgr.GetPlayer(mPlayerIndex)->GetGunAlpha() + interp);
}

float CAutoMapper::GetMapAreaMiniMapDrawAlphaOutlineUnvisited(const CStateManager& mgr) const {
  const float interp = gpTweakGui->GetMapAlphaInterpolant();
  return gpTweakAutoMapper->GetMiniAlphaOutlineUnvisited() *
         ((1.f - interp) * mgr.GetPlayer(mPlayerIndex)->GetGunAlpha() + interp);
}

float CAutoMapper::GetClampedMapScreenCameraDistance(float value) const {
  if (mState == kAMS_MapScreenUniverse) {
    return CMath::Clamp(gpTweakAutoMapper->GetMinUniverseCamDistance(), value,
                        gpTweakAutoMapper->GetMaxUniverseCamDistance());
  }
  return CMath::Clamp(gpTweakAutoMapper->GetMinCamDistance(), value,
                      gpTweakAutoMapper->GetMaxCamDistance());
}

float CAutoMapper::GetDesiredMiniMapCameraDistance(const CStateManager& mgr) const {
  CMapWorldInfo* info =
      gpGameState->StateForWorld(mWorld->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();
  const CMapWorld* mapWorld = mWorld->IGetMapWorld();
  CAABox bounds = CAABox::MakeMaxInvertedBox();
  const IGameArea* area = mWorld->IGetAreaAlways(mCurAreaId);
  CMapArea* mapArea = mapWorld->GetMapArea(mCurAreaId.Value());
  const bool oneMiniMapArea = gpTweakAutoMapper->GetShowOneMiniMapArea();
  for (int i = -1; i < (oneMiniMapArea ? 0 : static_cast< int >(area->IGetNumAttachedAreas()));
       ++i) {
    const int areaId = i == -1 ? mCurAreaId.Value() : area->IGetAttachedAreaId(i).Value();
    CMapArea* attachedArea = mapWorld->GetMapArea(areaId);
    if (attachedArea->GetIsVisibleToAutoMapper(
            info->IsWorldVisible(TAreaId(areaId), attachedArea->IsInDarkWorld()),
            info->IsAreaVisible(TAreaId(areaId)))) {
      bounds.Include(attachedArea->GetBoundingBox().GetTransformedAABox(
          attachedArea->GetAreaPostTransform(*mWorld, areaId)));
    }
  }

  const CVector3f point = mapArea->GetAreaPostTransform(*mWorld, mCurAreaId.Value()) *
                         mapArea->GetAreaCenterPoint();
  const float maxX =
      rstl::max_val(point.GetX() - bounds.GetMinPoint().GetX(),
                    bounds.GetMaxPoint().GetX() - point.GetX());
  const float maxY =
      rstl::max_val(point.GetY() - bounds.GetMinPoint().GetY(),
                    bounds.GetMaxPoint().GetY() - point.GetY());
  const float maxZ =
      rstl::max_val(point.GetZ() - bounds.GetMinPoint().GetZ(),
                    bounds.GetMaxPoint().GetZ() - point.GetZ());
  const CVector3f extent =
      mapArea->GetBoundingBox().GetMaxPoint() - mapArea->GetBoundingBox().GetMinPoint();
  const float halfExtent = 0.5f * extent.Magnitude();
  const CVector3f maxMargin(maxX, maxY, maxZ);
  float distance = 0.5f * halfExtent + 0.5f * maxMargin.Magnitude();
  distance *= gpTweakAutoMapper->GetMiniMapCamDistScale();
  return distance * static_cast< float >(tan(M_PIF / 2.f -
                                            0.5f * CMath::Deg2Rad(mRenderState0.mCamAngle)));
}

float CAutoMapper::GetBaseMapScreenCameraMoveSpeed() const {
  return gpTweakAutoMapper->GetBaseMapScreenCameraMoveSpeed();
}

float CAutoMapper::GetFinalMapScreenCameraMoveSpeed() const {
  const float speed = GetBaseMapScreenCameraMoveSpeed();
  if (gpTweakAutoMapper->GetScaleMoveSpeedWithCameraDistance()) {
    return speed * mRenderState0.mCamDist / gpTweakAutoMapper->GetCameraDistance();
  }
  return speed;
}

bool CAutoMapper::IsInMapperState(EAutoMapperState state) const {
  return state == mState && state == mNextState;
}

bool CAutoMapper::IsInMapperStateTransition() const { return mState != mNextState; }

bool CAutoMapper::IsRenderStateInterpolating() const { return mInterpTime < mInterpDur; }

void CAutoMapper::TransformRenderStatesWorldToUniverse() {
  const CTransform4f& xf = mMapu.GetObject()->GetMapWorldData(mWorldIdx).GetWorldTransform();
  const CQuaternion rot = CQuaternion::FromMatrix(xf);
  mRenderState2.mCamOrientation *= rot;
  mRenderState2.mAreaPoint = xf * mRenderState2.mAreaPoint;
  mRenderState0.mCamOrientation *= rot;
  mRenderState0.mAreaPoint = xf * mRenderState0.mAreaPoint;
  mRenderState1.mCamOrientation *= rot;
  mRenderState1.mAreaPoint = xf * mRenderState1.mAreaPoint;
}

void CAutoMapper::TransformRenderStatesUniverseToWorld() {
  const CTransform4f xf =
      mMapu.GetObject()->GetMapWorldData(mWorldIdx).GetWorldTransform().GetQuickInverse();
  const CQuaternion rot = CQuaternion::FromMatrix(xf);
  mRenderState2.mCamOrientation *= rot;
  mRenderState2.mAreaPoint = xf * mRenderState2.mAreaPoint;
  mRenderState0.mCamOrientation *= rot;
  mRenderState0.mAreaPoint = xf * mRenderState0.mAreaPoint;
  mRenderState1.mCamOrientation *= rot;
  mRenderState1.mAreaPoint = xf * mRenderState1.mAreaPoint;
}

void CAutoMapper::TransformRenderStateWorldToUniverse(SAutoMapperRenderState& state) {
  const CTransform4f& xf = mMapu.GetObject()->GetMapWorldData(mWorldIdx).GetWorldTransform();
  state.mAreaPoint = xf * mRenderState1.mAreaPoint;
}

void CAutoMapper::SetCurWorldAssetId(int worldId) {
  const int numWorlds = mMapu.GetObject()->GetNumMapWorldDatas();
  for (int i = 0; i < numWorlds; ++i) {
    if (worldId == mMapu.GetObject()->GetMapWorldData(i).GetWorldAssetId()) {
      mWorldIdx = i;
      return;
    }
  }
}

void CAutoMapper::LeaveMapScreen(CStateManager& mgr) {
  if (mNextState == kAMS_MapScreenUniverse) {
    const float depth = GetMapAreaMiniMapDrawDepth();
    mRenderState1.mDrawDepth1 = depth;
    mRenderState1.mDrawDepth2 = depth;
    mRenderState0.mDrawDepth1 = depth;
    mRenderState0.mDrawDepth2 = depth;
    SetupMiniMapWorld(mgr);
  } else {
    mTransitionState = kTS_ReturnToPlayer;
    mRenderState2 = mRenderState1 = mRenderState0;
    SetCurAreaId(mWorld->IGetCurrentAreaId().Value());
    mRenderState1.mAreaPoint = GetAreaPointOfInterest(mgr, mCurAreaId.Value());
    mRenderState1.mPointEase = SAutoMapperRenderState::kE_Linear;
    mRenderState1.mDrawDepth1 = GetMapAreaMiniMapDrawDepth();
    mRenderState1.mDrawDepth2 = GetMapAreaMiniMapDrawDepth();
    mRenderState1.mDepth1Ease = SAutoMapperRenderState::kE_Linear;
    mRenderState1.mDepth2Ease = SAutoMapperRenderState::kE_Linear;
    ResetInterpolationTimer(0.25f);
  }
}

void CAutoMapper::SetupMiniMapWorld(CStateManager& mgr) {
  CWorld* world = mgr.World();
  world->GetMapWorld()->SetWhichMapAreasLoaded(*world, world->GetCurrentAreaId().Value(), 3);
  mTransitionState = kTS_MiniMapReady;
}

void CAutoMapper::UpdateTempleKeys(const CStateManager& mgr) {
  if (mTextpaneKeylegend == nullptr) {
    return;
  }

  if (mWorld != nullptr) {
    rstl::wstring text;
    const int worldIndex = mWorld->IGetTempleKeyWorldIndex();
    const CPlayerState* playerState = mgr.GetPlayerState(mPlayerIndex);
    for (uint i = 0; i < 9; ++i) {
      const SMapKeyEntry& key = skMapKeys[i];
      if (worldIndex == key.mTempleWorldIndex) {
        if (playerState->GetItemAmount(key.mItemType) > 0) {
          text.append(gpStringTable->GetString("TempleKeyFoundIcon"), -1);
        } else {
          text.append(gpStringTable->GetString("TempleKeyNotFoundIcon"), -1);
        }
        if (key.mLabel[0] != '\0') {
          text.append(gpStringTable->GetString(key.mLabel), -1);
        }
      }
    }
    mTextpaneKeylegend->TextSupport().SetText(text);
  } else {
    mTextpaneKeylegend->TextSupport().SetText(rstl::string_l(""));
  }
}

void CAutoMapper::SetCurAreaId(int areaId) {
  if (mCurAreaId.Value() != areaId &&
      (close_enough(mDarkWorldBlend, 0.f) || close_enough(mDarkWorldBlend, 1.f)) &&
      mState != kAMS_MiniMap) {
    if (mTransitionState == kTS_Idle) {
      CSfxManager::SfxStart(0x2200, 127, 64);
    } else {
      CSfxManager::SfxStart(0x21ff, 127, 64);
    }
  }
  mCurAreaId = areaId;
}
