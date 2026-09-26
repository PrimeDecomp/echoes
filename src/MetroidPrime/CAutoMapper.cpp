#include "MetroidPrime/CAutoMapper.hpp"

#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CAbsAngle.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CDummyWorld.hpp"
#include "MetroidPrime/CEulerAngles.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CMapArea.hpp"
#include "MetroidPrime/CMapUniverse.hpp"
#include "MetroidPrime/CMapWorld.hpp"
#include "MetroidPrime/CMapWorldInfo.hpp"
#include "MetroidPrime/CMemoryCard.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/IGameArea.hpp"
#include "MetroidPrime/IWorld.hpp"
#include "MetroidPrime/Player/CEnvironmentVariable.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CWorldState.hpp"
#include "MetroidPrime/Tweaks/CTweakAutoMapper.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/math.hpp"


// Work in progress: Update remains incomplete.

static const char* const skFRME_MapScreen = "FRME_MapScreen";
static const char* const skFRME_MapScreenBackground = "FRME_MapScreenBackground";

// Guessed name
struct SMapKeyEntry {
  const char* mLabel;
  CPlayerState::EItemType mItemType;
  int mTempleWorldIndex;
};

static const SMapKeyEntry skMapKeys[] = {
    {"", CPlayerState::kIT_AgonKey1, 2},   {"", CPlayerState::kIT_AgonKey2, 2},
    {"", CPlayerState::kIT_AgonKey3, 2},   {"", CPlayerState::kIT_TorvusKey1, 3},
    {"", CPlayerState::kIT_TorvusKey2, 3}, {"", CPlayerState::kIT_TorvusKey3, 3},
    {"", CPlayerState::kIT_HiveKey1, 4},   {"", CPlayerState::kIT_HiveKey2, 4},
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

void CAutoMapper::SetupHintNavigation() {
  if (!gpGameState->GameOptions().GetIsHintSystemEnabled()) {
    return;
  }

  mHintSteps.clear();
  mHintLocations.clear();
  CHintOptions& hintOptions = gpGameState->HintOptions();
  const CHintOptions::SHintState* currentHint = hintOptions.GetCurrentDisplayedHint();
  bool navigating = false;
  if (currentHint != nullptr && currentHint->CanContinue()) {
    navigating = true;
    mHintSteps.push_back(SAutoMapperHintStep(SAutoMapperHintStep::kHST_ShowBeacon, 0.75f));
    const CGameHintInfo::CGameHint& nextHint =
        gpMemoryCard->GetHints()[hintOptions.GetNextHintIdx()];
    CAssetId currentWorldId = mWorld->IGetWorldAssetId();
    const rstl::vector< CGameHintInfo::SHintLocation >& locations = nextHint.GetLocations();
    for (int i = 0; i < locations.size(); ++i) {
      const CGameHintInfo::SHintLocation& location = locations[i];
      const CAssetId nextWorldId = location.mMlvlId;
      if (nextWorldId != currentWorldId) {
        mHintSteps.push_back(SAutoMapperHintStep(SAutoMapperHintStep::kHST_SwitchToUniverse, 0));
        mHintSteps.push_back(SAutoMapperHintStep(SAutoMapperHintStep::kHST_PanToWorld,
                                               static_cast< int >(nextWorldId)));
        mHintSteps.push_back(SAutoMapperHintStep(SAutoMapperHintStep::kHST_SwitchToWorld,
                                               static_cast< int >(nextWorldId)));
        currentWorldId = nextWorldId;
      } else {
        mHintSteps.push_back(SAutoMapperHintStep(SAutoMapperHintStep::kHST_ZoomOut, 0));
      }
      mHintSteps.push_back(
          SAutoMapperHintStep(SAutoMapperHintStep::kHST_PanToArea, location.mAreaId.value));
      mHintSteps.push_back(SAutoMapperHintStep(SAutoMapperHintStep::kHST_ZoomIn, 0));
      mHintSteps.push_back(SAutoMapperHintStep(SAutoMapperHintStep::kHST_ShowBeacon, 1.f));
      mHintLocations.push_back(
          SAutoMapperHintLocation(0, 0.f, location.mMlvlId, location.mAreaId.value));
    }
  }

  const rstl::vector< CHintOptions::SHintState >& hintStates = hintOptions.GetHintStates();
  for (int i = 0; i < hintStates.size(); ++i) {
    if (navigating && i == hintOptions.GetNextHintIdx()) {
      continue;
    }
    if (hintStates[i].mState != kHS_Displaying) {
      continue;
    }
    const CGameHintInfo::CGameHint& hint = gpMemoryCard->GetHints()[i];
    const rstl::vector< CGameHintInfo::SHintLocation >& locations = hint.GetLocations();
    for (int j = 0; j < locations.size(); ++j) {
      mHintLocations.push_back(
          SAutoMapperHintLocation(1, 1.f, locations[j].mMlvlId, locations[j].mAreaId.value));
    }
  }
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

bool CAutoMapper::CanSwitchLightDarkWorld() const {
  if (IsInMapperState(kAMS_MapScreenUniverse)) {
    return false;
  }
  if (mWorld != nullptr) {
    const CAssetId worldId = mWorld->IGetWorldAssetId();
    CMapWorldInfo* info = gpGameState->StateForWorld(worldId).GetMapWorldInfo().GetPtr();
    const CMapWorld* mapWorld = mWorld->IGetMapWorld();
    const rstl::vector< int > areas = mapWorld->GetVisibleAreas(*mWorld, *info);
    const bool inDarkWorld = mDarkWorldBlend >= 0.5f;
    for (int i = 0; i < areas.size(); ++i) {
      const bool otherWorld = mapWorld->GetMapArea(areas[i])->IsInDarkWorld() != inDarkWorld;
      if (otherWorld) {
        return true;
      }
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

void CAutoMapper::UpdateHintNavigation(float dt, CStateManager& mgr) {
  SAutoMapperHintStep& nextStep = mHintSteps.front();
  const SAutoMapperHintStep::Data hintData = nextStep.mData;
  bool wasProcessing = nextStep.mProcessing;
  nextStep.mProcessing = true;

  switch (nextStep.mType) {
  case SAutoMapperHintStep::kHST_PanToArea: {
    const int areaId = nextStep.mData.mAreaId;
    const CMapWorld* mapWorld = mWorld->IGetMapWorld();
    CMapArea* mapArea = mapWorld->GetMapArea(areaId);
    if (mapArea != nullptr) {
      if ((mDarkWorldBlend > 0.f) != mapArea->IsInDarkWorld()) {
        mHintSteps.push_front(
            SAutoMapperHintStep(SAutoMapperHintStep::kHST_SwitchLightDarkWorld, 0));
      } else {
        mRenderState2 = mRenderState0;
        mRenderState1.mAreaPoint = GetAreaPointOfInterest(mgr, areaId);
        mRenderState1.ResetInterpolation();
        mRenderState1.mPointEase = SAutoMapperRenderState::kE_Linear;
        ResetInterpolationTimer(2.f * gpTweakAutoMapper->GetHintPanTime());
        mHintSteps.pop_front();
      }
    }
    break;
  }
  case SAutoMapperHintStep::kHST_PanToWorld: {
    const CMapUniverse::CMapWorldData& worldData =
        mMapu.GetObject()->GetMapWorldDataByWorldId(hintData.mWorldId);
    const CVector3f centerPoint = worldData.GetWorldCenterPoint();
    mRenderState2 = mRenderState0;
    mRenderState1.mAreaPoint = centerPoint;
    mRenderState1.ResetInterpolation();
    mRenderState1.mPointEase = SAutoMapperRenderState::kE_Linear;
    ResetInterpolationTimer(2.f * gpTweakAutoMapper->GetHintPanTime());
    mHintSteps.pop_front();
    break;
  }
  case SAutoMapperHintStep::kHST_SwitchToUniverse:
    if (HasCurrentMapUniverseWorld(mgr)) {
      BeginMapperStateTransition(kAMS_MapScreenUniverse, mgr);
      mHintSteps.pop_front();
    } else {
      mHintSteps.clear();
    }
    break;
  case SAutoMapperHintStep::kHST_SwitchToWorld:
    mHintSteps.pop_front();
    mLoadingDummyWorld = true;
    if (!CheckDummyWorldLoad(mgr)) {
      mHintSteps.clear();
    }
    break;
  case SAutoMapperHintStep::kHST_ShowBeacon: {
    SAutoMapperHintStep::Data beaconData(hintData.mFloat);
    float& beaconTime = beaconData.mFloat;
    if (!wasProcessing) {
      if (mCurAreaId == mgr.GetNextAreaId() && mWorld == mgr.GetWorld()) {
        CSfxManager::SfxStart(0xdae, 127, 64);
      } else {
        CSfxManager::SfxStart(0xdaf, 127, 64);
      }
    }

    beaconTime = rstl::max_val(0.f, beaconTime - dt);
    nextStep.mData = beaconData;
    for (rstl::list< SAutoMapperHintLocation >::iterator it = mHintLocations.begin();
         it != mHintLocations.end(); ++it) {
      if (it->mWorldId == mWorld->IGetWorldAssetId() && it->mAreaId == mCurAreaId) {
        it->mShowBeacon = 1;
        const float alpha = rstl::min_val(1.f, beaconTime / 0.5f);
        it->mBeaconAlpha = 1.f - alpha;
        break;
      }
    }
    if (beaconTime == 0.f) {
      mHintSteps.pop_front();
    }
    break;
  }
  case SAutoMapperHintStep::kHST_ZoomOut:
    mRenderState2 = mRenderState0;
    if (mMapMode == kMM_Teleport) {
      mRenderState1.mCamDist = gpTweakAutoMapper->GetMaxUniverseCamDistance();
    } else {
      mRenderState1.mCamDist = gpTweakAutoMapper->GetMaxCamDistance();
    }
    mRenderState1.ResetInterpolation();
    mRenderState1.mCamEase = SAutoMapperRenderState::kE_Linear;
    ResetInterpolationTimer(0.5f);
    mHintSteps.pop_front();
    break;
  case SAutoMapperHintStep::kHST_ZoomIn:
    mRenderState2 = mRenderState0;
    mRenderState1.mCamDist = gpTweakAutoMapper->GetCameraDistance();
    mRenderState1.ResetInterpolation();
    mRenderState1.mCamEase = SAutoMapperRenderState::kE_Linear;
    ResetInterpolationTimer(0.5f);
    mHintSteps.pop_front();
    break;
  case SAutoMapperHintStep::kHST_SwitchLightDarkWorld:
    if (SwitchLightDarkWorld()) {
      mRenderState2 = mRenderState0;
      mRenderState1.ResetInterpolation();
      ResetInterpolationTimer(0.6f);
      mHintSteps.pop_front();
    }
    break;
  case SAutoMapperHintStep::kHST_LeaveMapScreen:
    if (TryLeaveMapScreen(mgr)) {
      mHintSteps.pop_front();
    }
    break;
  default:
    break;
  }
}

void CAutoMapper::ProcessControllerInput(const CFinalInput& input, CStateManager& mgr) {
  if (!IsRenderStateInterpolating() && IsInPlayerControlState()) {
    if (mLoadingDummyWorld) {
      CheckDummyWorldLoad(mgr);
    } else if (mHintSteps.size() > 0) {
      UpdateHintNavigation(input.DeltaTime(), mgr);
    } else if (mTransitionState == kTS_Idle) {
      ProcessMapScreenInput(input, mgr);
    }
  }

  const CMatrix3f camRot(mRenderState0.mCamOrientation.BuildTransform());
  if (IsInMapperState(kAMS_MapScreen)) {
    CMapWorldInfo* info =
        gpGameState->StateForWorld(mWorld->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();
    const CUnitVector3f direction(camRot.GetColumn(kDY), CUnitVector3f::kN_No);
    const int areaId =
        FindClosestVisibleArea(mRenderState0.mAreaPoint, direction, mgr, *mWorld, *info);
    if (areaId != mCurAreaId.value) {
      SetCurAreaId(areaId);
      mRenderState0.mDrawDepth1 = GetMapAreaMaxDrawDepth(mgr, mCurAreaId.value);
      mRenderState0.mDrawDepth2 = GetMapAreaMaxDrawDepth(mgr, mCurAreaId.value);
    }
  } else if (IsInMapperState(kAMS_MapScreenUniverse)) {
    const CMapUniverse* const mapu = mMapu.GetObject();
    const int oldWorldIdx = mWorldIdx;
    if (mHintSteps.size() > 0 &&
        (mHintSteps.front().mType == SAutoMapperHintStep::kHST_PanToWorld ||
         mHintSteps.front().mType == SAutoMapperHintStep::kHST_SwitchToWorld)) {
      SetCurWorldAssetId(mHintSteps.front().mData.mWorldId);
    } else {
      const CUnitVector3f direction(camRot.GetColumn(kDY), CUnitVector3f::kN_No);
      mWorldIdx = FindClosestVisibleWorld(mRenderState0.mAreaPoint, direction, mgr).first;
    }

    if (mWorldIdx != oldWorldIdx) {
      const CAssetId currentWorldId = gpGameState->CurrentWorldAssetId();
      for (int i = 0; i < mDummyWorlds.size(); ++i) {
        const CAssetId worldId = mapu->GetMapWorldData(i).GetWorldAssetId();
        if (i == mWorldIdx && currentWorldId != worldId) {
          if (gpResourceFactory->CanBuild(SObjectTag('MLVL', worldId))) {
            mDummyWorlds[i] = rstl::auto_ptr< IWorld >(rs_new CDummyWorld(worldId, true));
          }
        } else {
          if (mWorldsPendingUnload.size() == mWorldsPendingUnload.capacity()) {
            mWorldsPendingUnload.reserve(mWorldsPendingUnload.size() + 25);
          }
          mWorldsPendingUnload.push_back_unsafe(mDummyWorlds[i]);
          mDummyWorlds[i] = rstl::auto_ptr< IWorld >();
        }
      }
      mWorld = currentWorldId == mapu->GetMapWorldData(mWorldIdx).GetWorldAssetId() ? mgr.World()
                                                                                    : nullptr;
      UpdateTempleKeys(mgr);
    }
  }

  if (mTextpaneInstructions != nullptr && mMapMode != kMM_Teleport) {
    if (mAreaHintDesc.valid() && mAreaHintDesc->IsLoaded()) {
      mTextpaneHint->TextSupport().SetText(rstl::wstring(mAreaHintDesc->GetObject()->GetString(0)));
      mBasewidgetHintgroup->SetVisibility(true, kTM_Children);
      mTextpaneInstructions1->TextSupport().SetText(rstl::wstring_l(L""));
      mTextpaneInstructions->TextSupport().SetText(rstl::wstring_l(L""));
      mTextpaneInstructions2->TextSupport().SetText(rstl::wstring_l(L""));
      mTextpaneXicon->TextSupport().SetText(rstl::wstring_l(L""));
      mTextpaneXicon1->TextSupport().SetText(rstl::wstring_l(L""));
      mTextpaneRight3->TextSupport().SetText(rstl::wstring_l(L""));
      mTextpaneRight->TextSupport().SetText(rstl::wstring_l(L""));
    } else {
      mTextpaneHint->TextSupport().SetText(rstl::wstring_l(L""));
      mBasewidgetHintgroup->SetVisibility(false, kTM_Children);

      const wchar_t imagePrefix[] = L"&image=";
      const wchar_t imageSuffix[] = L";";
      CStringTable* const stringTable = gpStringTable;
      rstl::wstring text;
      text.reserve(256);
      text.append(imagePrefix, -1);
      text.append(CStringExtras::ConvertToUNICODE(rstl::string(
          CBasics::Stringize("SI,0.6,1.0,%8.8X", gpTweakPlayerRes->mLStick[mLStickPos]))));
      text.append(imageSuffix, -1);
      text.append(stringTable->GetString("InstructionRotate"), -1);
      mTextpaneInstructions->TextSupport().SetText(text);

      text.assign(imagePrefix, -1);
      text.append(CStringExtras::ConvertToUNICODE(rstl::string(
          CBasics::Stringize("SI,0.6,1.0,%8.8X", gpTweakPlayerRes->mCStick[mRStickPos]))));
      text.append(imageSuffix, -1);
      text.append(stringTable->GetString("InstructionMove"), -1);
      mTextpaneInstructions1->TextSupport().SetText(text);

      text.assign(imagePrefix, -1);
      text.append(CStringExtras::ConvertToUNICODE(
          rstl::string(CBasics::Stringize("%8.8X", gpTweakPlayerRes->mLTrigger[mLTriggerPos]))));
      text.append(imageSuffix, -1);
      text.append(rstl::wstring_l(L" "));
      text.append(imagePrefix, -1);
      text.append(CStringExtras::ConvertToUNICODE(
          rstl::string(CBasics::Stringize("%8.8X", gpTweakPlayerRes->mRTrigger[mRTriggerPos]))));
      text.append(imageSuffix, -1);
      text.append(stringTable->GetString("InstructionZoom"), -1);
      mTextpaneInstructions2->TextSupport().SetText(text);

      mTextpaneXicon1->TextSupport().SetText(
          rstl::wstring(gpStringTable->GetString("InstructionsXButton")));
      mTextpaneXicon->TextSupport().SetText(rstl::wstring(gpStringTable->GetString(
          mDarkWorldBlend < 0.5f ? "InstructionSwitchMap" : "InstructionSwitchMapLight")));
    }
  }

  if (input.PY()) {
    int paneMode =
        gpGameState->SystemOptions().FindEnvironmentVariable("AutoMapperPaneMode")->GetValue();
    if (mMapMode == kMM_Teleport) {
      switch (paneMode) {
      case 1:
        paneMode = 2;
        CSfxManager::SfxStart(0x13b8, 127, 64);
        break;
      case 2:
        paneMode = 1;
        CSfxManager::SfxStart(0x13b7, 127, 64);
        break;
      }
    } else {
      switch (paneMode) {
      case 0:
        paneMode = 2;
        CSfxManager::SfxStart(0x13b6, 127, 64);
        break;
      case 1:
        paneMode = 0;
        CSfxManager::SfxStart(0x13b8, 127, 64);
        break;
      case 2:
        paneMode = 1;
        CSfxManager::SfxStart(0x13b7, 127, 64);
        break;
      }
    }
    gpGameState->SystemOptions().FindEnvironmentVariable("AutoMapperPaneMode")->Set(paneMode);
  }

  if ((gpGameState->ControlMapper().GetPressInput(CControlMapper::kC_ExitMap, input) ||
       input.PB()) &&
      mTransitionState == kTS_Idle && !IsRenderStateInterpolating()) {
    TryLeaveMapScreen(mgr);
  }
}

void CAutoMapper::ProcessMapScreenInput(const CFinalInput& input, CStateManager& mgr) {
  const CMatrix3f camRot(mRenderState0.mCamOrientation.BuildTransform());
  if (mState == kAMS_MapScreen) {
    if (input.PA() && mTransitionState == kTS_Idle) {
      if (HasCurrentMapUniverseWorld(mgr)) {
        BeginMapperStateTransition(kAMS_MapScreenUniverse, mgr);
      }
    }
    if (input.PX() && CanSwitchLightDarkWorld()) {
      SwitchLightDarkWorld();
    }
  } else if (mState == kAMS_MapScreenUniverse) {
    if (input.PA()) {
      const CMapUniverse::CMapWorldData& worldData = mMapu.GetObject()->GetMapWorldData(mWorldIdx);
      const CVector3f& localPoint =
          worldData.GetWorldTransform().GetQuickInverse() * mRenderState0.mAreaPoint;
      if (worldData.GetWorldAssetId() != gpGameState->CurrentWorldAssetId()) {
        mLoadingDummyWorld = true;
        CheckDummyWorldLoad(mgr);
      } else if (mMapMode != kMM_Teleport) {
        mWorld = mgr.World();
        UpdateTempleKeys(mgr);
        CMapWorldInfo* info =
            gpGameState->StateForWorld(mWorld->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();
        const int areaId = FindClosestVisibleArea(
            localPoint, CUnitVector3f(camRot.GetColumn(kDY), CUnitVector3f::kN_No), mgr, *mWorld,
            *info);
        SetCurAreaId(areaId);
        BeginMapperStateTransition(kAMS_MapScreen, mgr);
      }
    }
    if (input.PX() && CanSwitchLightDarkWorld() && mMapMode != kMM_Teleport) {
      SwitchLightDarkWorld();
    }
  }

  mAButtonPos = 0;
  if (input.PA()) {
    mAButtonPos = 1;
  }

  if (IsInPlayerControlState()) {
    mLStickPos = 0;
    mRStickPos = 0;
    mLTriggerPos = 0;
    mRTriggerPos = 0;
    ProcessMapRotateInput(input, mgr);
    ProcessMapZoomInput(input, mgr);
    ProcessMapPanInput(input, mgr);
  } else {
    SetShouldPanningSoundBePlaying(false);
    SetShouldZoomingSoundBePlaying(false);
    SetShouldRotatingSoundBePlaying(false);
  }
}

void CAutoMapper::ProcessMapRotateInput(const CFinalInput& input, const CStateManager& mgr) {
  float up = gpGameState->ControlMapper().GetAnalogInput(CControlMapper::kC_MapCircleUp, input);
  float down = gpGameState->ControlMapper().GetAnalogInput(CControlMapper::kC_MapCircleDown, input);
  float left = gpGameState->ControlMapper().GetAnalogInput(CControlMapper::kC_MapCircleLeft, input);
  float right =
      gpGameState->ControlMapper().GetAnalogInput(CControlMapper::kC_MapCircleRight, input);

  int flags = 0;
  if (up > 0.f)
    flags += 2;
  if (down > 0.f)
    flags += 1;
  if (left > 0.f)
    flags += 4;
  if (right > 0.f)
    flags += 8;

  switch (flags) {
  case 1:
    mLStickPos = 1;
    break;
  case 2:
    mLStickPos = 5;
    break;
  case 4:
    mLStickPos = 3;
    break;
  case 5:
    mLStickPos = 2;
    break;
  case 6:
    mLStickPos = 4;
    break;
  case 8:
    mLStickPos = 7;
    break;
  case 9:
    mLStickPos = 8;
    break;
  case 10:
    mLStickPos = 6;
    break;
  default:
    break;
  }

  float maxMag = up;
  int dirSlot = 0;
  if (down > up) {
    maxMag = down;
    dirSlot = 1;
  }
  if (left > maxMag) {
    maxMag = left;
    dirSlot = 2;
  }
  if (right > maxMag) {
    maxMag = right;
    dirSlot = 3;
  }

  left = 0.f;
  right = 0.f;
  down = 0.f;
  up = 0.f;
  switch (dirSlot) {
  case 0:
    up = maxMag;
    break;
  case 1:
    down = maxMag;
    break;
  case 2:
    left = maxMag;
    break;
  case 3:
    right = maxMag;
    break;
  default:
    break;
  }

  if (up > 0.f || down > 0.f || left > 0.f || right > 0.f) {
    float deltaFrames = 60.f * input.DeltaTime();
    SetShouldRotatingSoundBePlaying(true);
    float minCamRotateX = gpTweakAutoMapper->GetMinCamRotateX();
    float maxCamRotateX = gpTweakAutoMapper->GetMaxCamRotateX();
    const CEulerAngles eulers = CEulerAngles::FromQuaternion(mRenderState0.mCamOrientation);
    CAbsAngle angX = CAbsAngle::FromRadians(eulers.GetX());
    CAbsAngle angZ = CAbsAngle::FromRadians(eulers.GetZ());

    float dt = deltaFrames * gpTweakAutoMapper->GetRotateDegPerFrame();

    angZ -= CRelAngle::FromDegrees(dt * left);
    angZ += CRelAngle::FromDegrees(dt * right);

    angX -= CRelAngle::FromDegrees(dt * up);
    angX += CRelAngle::FromDegrees(dt * down);

    float angXDeg = angX.AsDegrees();
    if (angXDeg > 180.f)
      angXDeg -= 360.f;
    float clampedX = CMath::Clamp(minCamRotateX, angXDeg, maxCamRotateX);
    angX = CAbsAngle::FromDegrees(clampedX);

    mRenderState0.mCamOrientation = CQuaternion::YXZRotation(
        CRelAngle::FromRadians(0.f), CRelAngle::FromRadians(angX.AsRadians()),
        CRelAngle::FromRadians(angZ.AsRadians()));
  } else {
    SetShouldRotatingSoundBePlaying(false);
  }
}

void CAutoMapper::ProcessMapZoomInput(const CFinalInput& input, const CStateManager& mgr) {
  bool zoomIn = gpGameState->ControlMapper().GetDigitalInput(CControlMapper::kC_MapZoomIn, input);
  bool zoomOut = gpGameState->ControlMapper().GetDigitalInput(CControlMapper::kC_MapZoomOut, input);

  EZoomState nextZoomState = kZS_None;
  float oldDist = mRenderState0.mCamDist;
  switch (mZoomState) {
  case kZS_None:
  case kZS_In:
    if (zoomIn)
      nextZoomState = kZS_In;
    else if (zoomOut)
      nextZoomState = kZS_Out;
    break;
  case kZS_Out:
    if (zoomOut)
      nextZoomState = kZS_Out;
    else if (zoomIn)
      nextZoomState = kZS_In;
    break;
  default:
    break;
  }

  mZoomState = nextZoomState;

  float deltaFrames = 60.f * input.DeltaTime();
  float speedMult = mState == kAMS_MapScreen ? 1.f : 4.f;
  float delta = gpTweakAutoMapper->GetZoomUnitsPerFrame() * (deltaFrames * speedMult);

  if (mZoomState == kZS_In) {
    mRenderState0.mCamDist = GetClampedMapScreenCameraDistance(mRenderState0.mCamDist - delta);
    mRTriggerPos = 1;
    mZoomState = kZS_In;
  } else if (mZoomState == kZS_Out) {
    mRenderState0.mCamDist = GetClampedMapScreenCameraDistance(mRenderState0.mCamDist + delta);
    mLTriggerPos = 1;
    mZoomState = kZS_Out;
  }

  if (oldDist == mRenderState0.mCamDist)
    SetShouldZoomingSoundBePlaying(false);
  else
    SetShouldZoomingSoundBePlaying(true);
}

void CAutoMapper::ProcessMapPanInput(const CFinalInput& input, const CStateManager& mgr) {
  float forward =
      gpGameState->ControlMapper().GetAnalogInput(CControlMapper::kC_MapMoveForward, input);
  float back = gpGameState->ControlMapper().GetAnalogInput(CControlMapper::kC_MapMoveBack, input);
  float left = gpGameState->ControlMapper().GetAnalogInput(CControlMapper::kC_MapMoveLeft, input);
  float right = gpGameState->ControlMapper().GetAnalogInput(CControlMapper::kC_MapMoveRight, input);

  CMatrix3f camRot = mRenderState0.mCamOrientation.BuildTransform();
  if (forward > 0.f || back > 0.f || left > 0.f || right > 0.f) {
    float deltaFrames = 60.f * input.DeltaTime();
    float speed = GetFinalMapScreenCameraMoveSpeed();
    int flags = 0;
    if (forward > 0.f)
      flags += 1;
    if (back > 0.f)
      flags += 2;
    if (left > 0.f)
      flags += 4;
    if (right > 0.f)
      flags += 8;

    switch (flags) {
    case 1:
      mRStickPos = 1;
      break;
    case 2:
      mRStickPos = 5;
      break;
    case 4:
      mRStickPos = 3;
      break;
    case 5:
      mRStickPos = 2;
      break;
    case 6:
      mRStickPos = 4;
      break;
    case 8:
      mRStickPos = 7;
      break;
    case 9:
      mRStickPos = 8;
      break;
    case 10:
      mRStickPos = 6;
      break;
    default:
      break;
    }

    CVector3f dirVec = speed * (deltaFrames * CVector3f(right - left, 0.f, forward - back));
    CVector3f newPoint = mRenderState0.mAreaPoint + camRot * dirVec;
    if ((newPoint - mRenderState0.mAreaPoint).Magnitude() > input.DeltaTime()) {
      SetShouldPanningSoundBePlaying(true);
    } else {
      SetShouldPanningSoundBePlaying(false);
    }

    if (mState == kAMS_MapScreen) {
      const CMapWorld* mapWorld = mWorld->IGetMapWorld();
      mRenderState0.mAreaPoint = mapWorld->ConstrainToWorldVolume(newPoint, camRot.GetColumn(kDY));
    } else {
      const CMapUniverse* mapu = mMapu.GetObject();
      float radius = mapu->GetMapUniverseRadius();
      CVector3f localPoint = newPoint - mapu->GetMapUniverseCenterPoint();
      if (localPoint.Magnitude() > radius) {
        newPoint = mapu->GetMapUniverseCenterPoint() + radius * localPoint.AsNormalized();
      }
      mRenderState0.mAreaPoint = newPoint;
    }
  } else {
    SetShouldPanningSoundBePlaying(false);
    float speed = gpTweakAutoMapper->GetCamPanUnitsPerFrame() * GetBaseMapScreenCameraMoveSpeed();
    if (mState == kAMS_MapScreen) {
      const CMapWorld* mapWorld = mWorld->IGetMapWorld();
      const CMapArea* area = mapWorld->GetMapArea(mCurAreaId.value);
      CMapArea* transformArea = mapWorld->GetMapArea(mCurAreaId.value);
      CVector3f worldPoint = transformArea->GetAreaPostTransform(*mWorld, mCurAreaId.value) *
                             area->GetAreaCenterPoint();
      CVector3f viewPoint = worldPoint - mRenderState0.mAreaPoint;
      if (viewPoint.Magnitude() < speed) {
        mRenderState0.mAreaPoint = worldPoint;
      } else {
        mRenderState0.mAreaPoint = mRenderState0.mAreaPoint + speed * viewPoint.AsNormalized();
      }
    } else {
      const CMapUniverse* mapu = mMapu.GetObject();
      rstl::pair< int, int > result =
          FindClosestVisibleWorld(mRenderState0.mAreaPoint,
                                  CUnitVector3f(camRot.GetColumn(kDY), CUnitVector3f::kN_No), mgr);
      const CTransform4f& hex =
          mapu->GetMapWorldData(result.first).GetMapAreaData(result.second).GetTransform();
      CVector3f hexPoint = hex.GetTranslation();
      CVector3f areaToHex = hexPoint - mRenderState0.mAreaPoint;
      if (areaToHex.Magnitude() < speed) {
        mRenderState0.mAreaPoint = hexPoint;
      } else {
        mRenderState0.mAreaPoint = mRenderState0.mAreaPoint + speed * areaToHex.AsNormalized();
      }
    }
  }
}

void CAutoMapper::Draw(const CStateManager& mgr, const CTransform4f& xf, float alpha) const {
  float drawAlpha = alpha * gpGameState->GameOptions().GetHudAlpha();
  gpRender->SetBlendMode_AlphaBlended();
  CGraphics::SetCullMode(kCM_Front);

  float alphaInterp;
  if (IsFullyOutOfMiniMapState()) {
    alphaInterp = 1.f;
  } else if (IsInMapperState(kAMS_MiniMap)) {
    alphaInterp = drawAlpha;
  } else if (mNextState == kAMS_MiniMap) {
    const float t = GetInterp();
    alphaInterp = drawAlpha * t + (1.f - t);
  } else if (mState == kAMS_MiniMap) {
    const float t = GetInterp();
    alphaInterp = drawAlpha * (1.f - t) + t;
  } else {
    alphaInterp = 1.f;
  }

  const float aspect =
      float(mRenderState0.mViewportSize.GetX()) / float(mRenderState0.mViewportSize.GetY());
  const float camAngleRad = mRenderState0.mCamAngle * (1.f / 360.f) * (2.f * M_PIF);
  const float yScale = mRenderState0.mCamDist / float(tan(M_PIF / 2.f - 0.5f * camAngleRad));
  const CTransform4f camXf =
      mRenderState0.mCamOrientation.BuildTransform4f(mRenderState0.mAreaPoint);
  const CTransform4f distScale(1.f / (yScale * aspect), 0.f, 0.f, 0.f, 0.f, 0.001f, 0.f, 0.f, 0.f,
                               0.f, 1.f / yScale, 0.f);
  const CTransform4f tweakScale =
      CTransform4f::Scale(gpTweakAutoMapper->GetMapPlaneScale().GetX(), 0.f,
                          gpTweakAutoMapper->GetMapPlaneScale().GetY());
  const CTransform4f planeXf = xf * tweakScale * distScale * camXf.GetQuickInverse();

  float universeInterp = 0.f;
  if (mNextState == kAMS_MapScreenUniverse) {
    universeInterp = mState == kAMS_MapScreenUniverse ? 1.f : GetInterp();
  } else if (mState == kAMS_MapScreenUniverse) {
    universeInterp = 1.f - GetInterp();
  }
  const bool inUniverse = mState == kAMS_MapScreenUniverse || mNextState == kAMS_MapScreenUniverse;
  const CTransform4f& preXf =
      inUniverse ? mMapu.GetObject()->GetMapWorldData(mWorldIdx).GetWorldTransform()
                 : CTransform4f::Identity();
  const float objectScale = mRenderState0.mCamDist / gpTweakAutoMapper->GetMinCamDistance();
  const float mapAlpha = alphaInterp * (1.f - universeInterp);

  if (IsFullyOutOfMiniMapState()) {
    if (universeInterp < 1.f && mWorld != nullptr) {
      const CMapWorldInfo& info =
          *gpGameState->StateForWorld(mWorld->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();
      const CMapWorld* mapWorld = mWorld->IGetMapWorld();
      float hintFlash = 0.f;
      if (!mHintSteps.empty() &&
          mHintSteps.begin()->mType == SAutoMapperHintStep::kHST_ShowBeacon) {
        const float hintTime = mHintSteps.begin()->mData.mFloat;
        if (mCurAreaId == mgr.GetNextAreaId() && mWorld == mgr.GetWorld()) {
          const float pulseTime = CMath::ModF(hintTime * 8.f, 1.f);
          hintFlash = 2.f * (pulseTime < 0.5f ? pulseTime : 1.f - pulseTime);
        } else if (mMapMode == kMM_Teleport) {
          const float pulseTime =
              CMath::ModF((1.f - rstl::max_val(0.f, (hintTime - 0.5f) / 0.5f)) * 4.f, 1.f);
          hintFlash = 2.f * (pulseTime < 0.5f ? pulseTime : 1.f - pulseTime);
        } else {
          for (rstl::list< SAutoMapperHintLocation >::const_iterator it = mHintLocations.begin();
               it != mHintLocations.end(); ++it) {
            if (it->mWorldId == mWorld->IGetWorldAssetId() && it->mAreaId == mCurAreaId) {
              const float pulseTime =
                  CMath::ModF((1.f - rstl::max_val(0.f, (hintTime - 0.5f) / 0.5f)) * 4.f, 1.f);
              hintFlash = 2.f * (pulseTime < 0.5f ? pulseTime : 1.f - pulseTime);
              break;
            }
          }
        }
      }

      mapWorld->Draw(
          CMapWorld::CMapWorldDrawParms(mRenderState0.mAlphaSurfaceVisited * alphaInterp,
                                        mRenderState0.mAlphaOutlineVisited * alphaInterp,
                                        mRenderState0.mAlphaSurfaceUnvisited * alphaInterp,
                                        mRenderState0.mAlphaOutlineUnvisited * alphaInterp,
                                        mapAlpha, mgr, planeXf * preXf, camXf, *mWorld, info, 2.f,
                                        true, mPlayerFlashPulse, hintFlash, objectScale),
          mCurAreaId.value, mCurAreaId.value, mRenderState0.mDrawDepth1, mRenderState0.mDrawDepth2,
          true);
    }
    if (mBasewidgetHintgroup != nullptr) {
      mBasewidgetHintgroup->SetColor(CColor::White().WithAlphaOf(1.f - universeInterp));
    }
  } else if (IsInMapperState(kAMS_MiniMap)) {
    const CMapWorld* mapWorld = mWorld->IGetMapWorld();
    const CMapWorldInfo& info =
        *gpGameState->StateForWorld(mWorld->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();
    mapWorld->Draw(CMapWorld::CMapWorldDrawParms(mRenderState0.mAlphaSurfaceVisited * alphaInterp,
                                                 mRenderState0.mAlphaOutlineVisited * alphaInterp,
                                                 mRenderState0.mAlphaSurfaceUnvisited * alphaInterp,
                                                 mRenderState0.mAlphaOutlineUnvisited * alphaInterp,
                                                 mapAlpha, mgr, planeXf, camXf, *mWorld, info, 1.f,
                                                 false, 0.f, 0.f, objectScale),
                   mCurAreaId.value, mOtherAreaId.value, mRenderState0.mDrawDepth1,
                   mRenderState0.mDrawDepth2, false);
  } else {
    const CMapWorld* mapWorld = mWorld->IGetMapWorld();
    const CMapWorldInfo& info =
        *gpGameState->StateForWorld(mWorld->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();
    mapWorld->Draw(CMapWorld::CMapWorldDrawParms(mRenderState0.mAlphaSurfaceVisited * alphaInterp,
                                                 mRenderState0.mAlphaOutlineVisited * alphaInterp,
                                                 mRenderState0.mAlphaSurfaceUnvisited * alphaInterp,
                                                 mRenderState0.mAlphaOutlineUnvisited * alphaInterp,
                                                 mapAlpha, mgr, planeXf * preXf, camXf, *mWorld,
                                                 info, 2.f, true, 0.f, 0.f, objectScale),
                   mCurAreaId.value, mCurAreaId.value, mRenderState0.mDrawDepth1,
                   mRenderState0.mDrawDepth2, false);
  }

  if (universeInterp > 0.f) {
    const CWorld* world = mgr.GetWorld();
    const int areaId = mgr.GetNextAreaId().value;
    CMapUniverse* mapu = mMapu.GetObject();
    const CTransform4f areaXf =
        world->GetMapWorld()->GetMapArea(areaId)->GetAreaPostTransform(*world, areaId);
    const CMapUniverse::CMapWorldData& worldData =
        mapu->GetMapWorldDataByWorldId(gpGameState->CurrentWorldAssetId());
    const CTransform4f universeAreaXf = worldData.GetWorldTransform() * areaXf;
    float minDistance = 3.4028234663852886e38f;
    int closestHex = -1;
    for (int i = 0; i < worldData.GetNumMapAreaDatas(); ++i) {
      const float distance = (universeAreaXf.GetTranslation() -
                              worldData.GetMapAreaData(i).GetTransform().GetTranslation())
                                 .Magnitude();
      if (distance < minDistance) {
        closestHex = i;
        minDistance = distance;
      }
    }
    mapu->Draw(CMapUniverse::CMapUniverseDrawParms(
                   universeInterp, mWorldIdx, gpGameState->CurrentWorldAssetId(), closestHex,
                   mPlayerFlashPulse, mgr, planeXf, camXf, mMapMode == kMM_Teleport),
               CVector3f::Zero(), 0.f, 0.f);
  }

  if (!IsInMapperState(kAMS_MapScreenUniverse)) {
    const CTransform4f mapXf = planeXf * preXf;
    if (mWorld == mgr.GetWorld()) {
      const float pulse = CMath::Clamp(
          0.f, 0.5f * (1.f + CMath::FastSinR(5.f * CGraphics::GetSecondsMod900() - M_PIF / 2.f)),
          1.f);
      const float scale = rstl::min_val(objectScale, 0.6f * gpTweakAutoMapper->GetMaxCamDistance() /
                                                         gpTweakAutoMapper->GetMinCamDistance());
      const CEulerAngles eulers = CEulerAngles::FromTransform(
          mgr.GetCameraManager(mPlayerIndex)->GetCurrentCameraTransform(mgr, 1));
      const float angle = CMath::ClampRadians(eulers.GetYaw());
      const CVector3f playerPos =
          mWorld->IGetMapWorld()->GetMapArea(mgr.GetNextAreaId().value)->GetMapAdjustment() +
          mgr.GetPlayer(mPlayerIndex)->GetTranslation();
      gpRender->SetModelMatrix(
          mapXf * CTransform4f(CMatrix3f::RotateZ(CRelAngle::FromRadians(angle)), playerPos) *
          CTransform4f::Scale(scale * (0.25f * pulse + 0.75f)));

      const float worldAlpha =
          0.75f * (mgr.GetIsDarkWorld() ? mDarkWorldBlend : 1.f - mDarkWorldBlend) + 0.25f;
      const float colorAlpha =
          IsFullyOutOfMiniMapState() ? 1.f : mRenderState0.mAlphaSurfaceVisited;
      const CModelFlags flags =
          CModelFlags::AlphaBlended(gpTweakAutoMapper->GetPlayerModelColor().WithAlphaModulatedBy(
              worldAlpha * colorAlpha * mapAlpha));
      mMiniMapSamus.GetObject()->Draw(CModelFlags(flags, CModelFlags::kF_DepthCompare |
                                                             CModelFlags::kF_DepthGreater |
                                                             CModelFlags::kF_Unknown200));
    }

    if (IsInMapperState(kAMS_MapScreen)) {
      const CAssetId worldId = mWorld->IGetWorldAssetId();
      const CMapWorld* mapWorld = mWorld->IGetMapWorld();
      for (rstl::list< SAutoMapperHintLocation >::const_iterator it = mHintLocations.begin();
           it != mHintLocations.end(); ++it) {
        if (it->mWorldId != worldId) {
          continue;
        }
        CMapArea* area = mapWorld->GetMapArea(it->mAreaId.value);
        if (area == nullptr) {
          continue;
        }
        if (area->IsInDarkWorld() ? mDarkWorldBlend < 0.5f : mDarkWorldBlend >= 0.5f) {
          continue;
        }

        const CTransform4f camRot(camXf.BuildMatrix3f(), CVector3f::Zero());
        CGraphics::SetModelMatrix(
            mapXf *
            CTransform4f::Translate(
                area->GetAreaPostTransform(*mWorld, it->mAreaId.value).GetTranslation()) *
            CTransform4f::Translate(area->GetAreaCenterPoint()) * CTransform4f::Scale(objectScale) *
            camRot);
        const float beaconAlpha = it->mShowBeacon == 1 ? it->mBeaconAlpha : 0.f;
        if (beaconAlpha > 0.f) {
          CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
          mHintBeacon.GetObject()->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
          gpRender->SetBlendMode_AdditiveAlpha();
          CGraphics::StreamBegin(kP_TriangleStrip);
          const float colorAlpha =
              IsFullyOutOfMiniMapState() ? 1.f : mRenderState0.mAlphaSurfaceVisited;
          const CColor beaconColor(static_cast< uchar >(255), static_cast< uchar >(255),
                                   static_cast< uchar >(255));
          CGraphics::StreamColor(beaconColor.WithAlphaOf(beaconAlpha * colorAlpha * mapAlpha));
          CGraphics::StreamTexcoord(0.f, 1.f);
          CGraphics::StreamVertex(CVector3f(-4.f, -8.f, 8.f));
          CGraphics::StreamTexcoord(0.f, 0.f);
          CGraphics::StreamVertex(CVector3f(-4.f, -8.f, 0.f));
          CGraphics::StreamTexcoord(1.f, 1.f);
          CGraphics::StreamVertex(CVector3f(4.f, -8.f, 8.f));
          CGraphics::StreamTexcoord(1.f, 0.f);
          CGraphics::StreamVertex(CVector3f(4.f, -8.f, 0.f));
          CGraphics::StreamEnd();
        }
      }
    }
  }

  gpRender->SetDepthReadWrite(false, false);
  gpRender->SetAmbientColor(CColor::White());
  CGraphics::DisableAllLights();
  if (mFrmeInitialized != nullptr) {
    drawAlpha = IsFullyOutOfMiniMapState()   ? 1.f
                : mNextState == kAMS_MiniMap ? 1.f - GetInterp()
                                             : GetInterp();
    CGraphics::SetDepthRange(0.f, 0.f);
    mFrmeInitialized->Draw(CGuiWidgetDrawParms(drawAlpha, CVector3f::Zero()));

    CModel* compass = mCompassModel.GetObject();
    CModel* shell = mCompassShellModel.GetObject();
    if (compass != nullptr && shell != nullptr) {
      gpRender->SetDepthReadWrite(false, false);
      gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
      CGraphics::SetCullMode(kCM_None);
      const CEulerAngles eulers = CEulerAngles::FromQuaternion(mRenderState0.mCamOrientation);
      CTransform4f compassXf = CTransform4f::RotateX(CRelAngle::FromRadians(eulers.GetX()));
      CTransform4f shellXf = compassXf;
      compassXf *= CTransform4f::RotateZ(CRelAngle::FromRadians(eulers.GetYaw()));
      compassXf *= CTransform4f::Scale(32.f);
      compassXf.AddTranslation(CVector3f(224.f, 0.f, -60.f));
      shellXf *= CTransform4f::Scale(32.f);
      shellXf.AddTranslation(CVector3f(224.f, 0.f, -60.f));
      const float compassAlpha = 0.5f * drawAlpha * (1.f - mBottomPanePos);
      gpRender->SetModelMatrix(compassXf);
      compass->Draw(CModelFlags::Additive(compassAlpha));
      gpRender->SetModelMatrix(shellXf);
      shell->Draw(CModelFlags::AlphaBlended(compassAlpha));
    }
    CGraphics::SetDepthRange(0.f, 1.f / 512.f);
  }
}

CAssetId CAutoMapper::GetAreaHintDescriptionString(CAssetId areaId) {
  const rstl::vector< CHintOptions::SHintState >& hintStates =
      gpGameState->HintOptions().GetHintStates();
  for (int i = 0; i < hintStates.size(); ++i) {
    if (hintStates[i].mState != kHS_Displaying) {
      continue;
    }
    const CGameHintInfo::CGameHint& hint = gpMemoryCard->GetHints()[i];
    const rstl::vector< CGameHintInfo::SHintLocation >& locations = hint.GetLocations();
    for (int j = 0; j < locations.size(); ++j) {
      const CGameHintInfo::SHintLocation& location = locations[j];
      if (location.mMreaId != areaId) {
        continue;
      }
      for (rstl::list< SAutoMapperHintLocation >::const_iterator it = mHintLocations.begin();
           it != mHintLocations.end(); ++it) {
        if (it->mAreaId == location.mAreaId && it->mBeaconAlpha > 0.f) {
          return location.mStringId;
        }
      }
    }
  }
  return kInvalidAssetId;
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
    mRenderState1 = BuildMapScreenWorldRenderState(mgr, mRenderState0.mCamOrientation,
                                                   mCurAreaId.Value(), mHintSteps.size() > 0);
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
  SAutoMapperRenderState ret(GetMapScreenViewportSize(), rot, camDist, tweak->GetCamAngle(),
                             GetAreaPointOfInterest(mgr, areaId),
                             GetMapAreaMaxDrawDepth(mgr, areaId),
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
  SAutoMapperRenderState ret(GetMapScreenViewportSize(), rot, tweak->GetUniverseCamDistance(),
                             tweak->GetCamAngle(), GetAreaPointOfInterest(mgr, areaId),
                             GetMapAreaMaxDrawDepth(mgr, areaId),
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

rstl::pair< int, int > CAutoMapper::FindClosestVisibleWorld(const CVector3f& point,
                                                            const CUnitVector3f& camDir,
                                                            const CStateManager& mgr) const {
  const CMapUniverse* const mapu = mMapu.GetObject();
  int closestWorld = mCurAreaId.value;
  int closestArea = mCurAreaId.value;
  float minDistance = 29999.f;

  for (int w = 0; w < mapu->GetNumMapWorldDatas(); ++w) {
    const CMapUniverse::CMapWorldData& worldData = mapu->GetMapWorldData(w);
    if (!gpGameState->StateForWorld(worldData.GetWorldAssetId())
             .GetMapWorldInfo()
             .GetPtr()
             ->IsAnythingSet()) {
      continue;
    }
    if (mMapMode == kMM_Teleport && worldData.GetWorldLabel() == "TempleHub") {
      continue;
    }

    for (int i = 0; i < worldData.GetNumMapAreaDatas(); ++i) {
      const CVector3f areaPoint = worldData.GetMapAreaData(i).GetTransform().GetTranslation();
      const CVector3f pointToArea = areaPoint - point;
      const CVector3f projectedPoint =
          pointToArea.CanBeNormalized()
              ? point +
                    (pointToArea.Magnitude() * CVector3f::Dot(pointToArea.AsNormalized(), camDir)) *
                        camDir
              : point;
      const float distance = (projectedPoint - areaPoint).Magnitude();
      if (distance < minDistance) {
        minDistance = distance;
        closestWorld = w;
        closestArea = i;
      }
    }
  }

  return rstl::pair< int, int >(closestWorld, closestArea);
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

  const CVector3f point =
      mapArea->GetAreaPostTransform(*mWorld, mCurAreaId.Value()) * mapArea->GetAreaCenterPoint();
  const float maxX = rstl::max_val(point.GetX() - bounds.GetMinPoint().GetX(),
                                   bounds.GetMaxPoint().GetX() - point.GetX());
  const float maxY = rstl::max_val(point.GetY() - bounds.GetMinPoint().GetY(),
                                   bounds.GetMaxPoint().GetY() - point.GetY());
  const float maxZ = rstl::max_val(point.GetZ() - bounds.GetMinPoint().GetZ(),
                                   bounds.GetMaxPoint().GetZ() - point.GetZ());
  const CVector3f extent =
      mapArea->GetBoundingBox().GetMaxPoint() - mapArea->GetBoundingBox().GetMinPoint();
  const float halfExtent = 0.5f * extent.Magnitude();
  const CVector3f maxMargin(maxX, maxY, maxZ);
  float distance = 0.5f * halfExtent + 0.5f * maxMargin.Magnitude();
  distance *= gpTweakAutoMapper->GetMiniMapCamDistScale();
  return distance *
         static_cast< float >(tan(M_PIF / 2.f - 0.5f * CMath::Deg2Rad(mRenderState0.mCamAngle)));
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
