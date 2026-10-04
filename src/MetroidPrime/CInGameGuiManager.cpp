#include "MetroidPrime/CInGameGuiManager.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiFrameLoader.hpp"
#include "GuiSys/CGuiHeadWidget.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/CDependencyGroup.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Input/IController.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "MetroidPrime/CAutoMapper.hpp"
#include "MetroidPrime/CGameGlobalObjects.hpp"
#include "MetroidPrime/CInGameQuitScreen.hpp"
#include "MetroidPrime/CMessageScreen.hpp"
#include "MetroidPrime/CPauseScreen.hpp"
#include "MetroidPrime/CPauseScreenBlur.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CTurretHud.hpp"
#include "MetroidPrime/HUD/CSamusHud.hpp"
#include "MetroidPrime/Player/CPlayerVisor.hpp"
#include "MetroidPrime/Player/CSamusFaceReflection.hpp"
#include "rstl/algorithm.hpp"

// Structure-first scaffold. This is not a functional replacement for the original object.

CInGameGuiManager::CInGameGuiManager(const CStateManager& mgr, CGuiFrameLoader& hud,
                                     CGuiFrameLoader& memo, CGuiFrameLoader* helmet,
                                     CGuiFrameLoader* darkMask, int playerIndex)
: mPlayerIndex(playerIndex)
, mIsSinglePlayer(mgr.GetNumPlayers() == 1)
, mDeathDot(gpSimplePool->GetObj("TXTR_DeathDot"))
, mFaceplateDecoration(mgr, playerIndex)
, mPlayerVisor(nullptr)
, mSamusHud(nullptr)
, mAutoMapper(nullptr)
, mSamusReflection(nullptr)
, mPauseScreenBlur(nullptr)
, mQuitScreen(nullptr)
, mMessageScreen(nullptr)
, mPauseScreen(nullptr)
, mTurretHud(nullptr)
, mPauseGameHudMessage(kInvalidAssetId)
, mPauseGameHudTime(0.f)
, mPauseScreenDGRPs(LockPauseScreenDependencies())
, mPrevState(kIGGS_Zero)
, mNextState(kIGGS_Zero)
, mHelmetVisMode(0)
, mEnableTargetingManager(0)
, mEnableAutoMapper(0)
, mHudVisMode(0)
, mEnablePlayerVisor(0)
, mAutoMapperRotation(CQuaternion::NoRotation())
, mAutoMapperOffset(CVector3f::Zero())
, mCameraRotation(CQuaternion::NoRotation())
, mCameraOffset(CVector3f::Zero())
, mMapCameraTransform(CTransform4f::Identity())
, mVisorStaticAlpha(0.f)
, mDarkOuterMask(nullptr)
, mLoaded(false)
, mPlayerAlive(true)
, mDeferTransition(false) {
  mDeathDot.Lock();
  // TODO: construct visor/HUD/mapper/blur, read cached tweak/player values and lock game-mode
  // DGRPs.
  if (darkMask != nullptr) {
    mDarkMaskFrame = rstl::auto_ptr< CGuiFrame >(darkMask->CreateFrame());
  }
}

bool CInGameGuiManager::CheckDGRPLoadComplete() {
  for (TPauseScreenDGRPs::iterator it = mPauseScreenDGRPs.begin();
       it != mPauseScreenDGRPs.end(); ++it) {
    if (!it->IsLoaded()) {
      return false;
    }
  }
  for (rstl::vector< TToken< CDependencyGroup > >::iterator it = mInGameGuiDGRPs.begin();
       it != mInGameGuiDGRPs.end(); ++it) {
    if (!it->IsLoaded()) {
      return false;
    }
  }
  return true;
}

bool CInGameGuiManager::CheckLoadComplete(const CStateManager&) {
  // TODO: finish the mask, HUD, mapper and dependency loads before initializing textures.
  return false;
}

bool CInGameGuiManager::GetIsGameDraw() const {
  return mPauseScreenBlur->IsGameDraw();
}

void CInGameGuiManager::PrepareScanDisplay(const CStateManager& mgr, int playerIndex) {
  if (!mSamusHud.null()) {
    mSamusHud->PrepareScanDisplay(mgr, playerIndex);
  }
}

void CInGameGuiManager::PreDraw(CStateManager& mgr, bool cameraActive) {
  if (!mSamusReflection.null() && cameraActive) {
    mSamusReflection->PreDraw(mgr);
  }
}

void CInGameGuiManager::DrawDarkVisorMask() const {
  if (mDarkMaskFrame.get() != nullptr) {
    CGraphics::SetDepthRange(1.f / 512.f, 1.f / 256.f);
    mSamusHud->GetLoadedHudFrame()->GetRootWidget()->Draw(CGuiWidgetDrawParms::Default());
    mDarkMaskFrame->GetRootWidget()->Draw(CGuiWidgetDrawParms::Default());
  }
}

void CInGameGuiManager::DrawScanVisor(float, const CStateManager&, const CColor&, const CColor&,
                                      const CColor&, const CColor*, int, const CVector3f&) const {
  // TODO: forward the visor dimensions, color palette and camera direction to the renderer.
  // The effect's source name remains unresolved.
}

void CInGameGuiManager::Draw(const CStateManager&) const {
  // TODO: restore the visor/HUD/map/pause draw sequence through shared interfaces.
}

void CInGameGuiManager::Update(const CStateManager&, float, CRandom16&, CArchitectureQueue&, bool) {
  // TODO: update per-player presentation, pause screens, audio and state transitions.
}

void CInGameGuiManager::ProcessControllerInput(const CStateManager&, const CFinalInput&, float) {
  // TODO: route input to the quit screen, map, pause screen or HUD according to GUI state.
}

void CInGameGuiManager::UpdateAutoMapper(const CStateManager&, float) {
  // TODO: update map camera lag and place the map using shared quaternion/transform helpers.
}

CInGameGuiManager::TPauseScreenDGRPs CInGameGuiManager::LockPauseScreenDependencies() {
  static const char* const names[] = {"PauseScreenDontDump_DGRP", "PauseScreenDontDump_NoARAM_DGRP",
                                      "PauseScreenTokens_DGRP"};
  TPauseScreenDGRPs result;
  for (int i = 0; i < 3; ++i) {
    TToken< CDependencyGroup > token = gpSimplePool->GetObj(names[i]);
    token.Lock();
    result.push_back(token);
  }
  return result;
}

bool CInGameGuiManager::IsTransitionReady() const {
  if (!mPauseScreenBlur->IsNotTransitioning()) {
    return false;
  }
  if (!mAutoMapper.null()) {
    return mAutoMapper->GetCurrentState() == mAutoMapper->GetNextState();
  }
  return true;
}

void CInGameGuiManager::TryCompleteStateTransition() {
  if (mNextState != kIGGS_PauseGame && mNextState != kIGGS_PauseLogBook) {
    mPauseScreen = nullptr;
  }
  if (InGameGuiStates::IsGameplayState(mNextState)) {
    mMessageScreen = nullptr;
    if (!TryReloadAreaTextures()) {
      return;
    }
    CModel::EnableTextureTimeout();
  }
  mPrevState = mNextState;
}

void CInGameGuiManager::BeginStateTransition(EInGameGuiState state, const CStateManager& mgr) {
  if (mNextState == state) {
    return;
  }
  mPrevState = mNextState;
  mNextState = state;

  if (state == kIGGS_InGame) {
    CSfxManager::SetChannel(CSfxManager::kSC_Game);
  } else if (state == kIGGS_PauseHUDMessage) {
    mMessageScreen = rs_new CMessageScreen(mPauseGameHudMessage, mPauseGameHudTime);
  } else if (state != kIGGS_PauseSaveGame && InGameGuiStates::IsGameplayState(mPrevState)) {
    mDeferTransition = true;
  }

  mPauseScreenBlur->OnNewInGameGuiState(state, mgr, *this);
  if (!mDeferTransition) {
    DoStateTransition(mgr);
  }
  if (state == kIGGS_InGame && !mAutoMapper.null()) {
    mAutoMapper->UnmuteAllLoopedSounds();
  }
}

void CInGameGuiManager::DoStateTransition(const CStateManager& mgr) {
  if (!mAutoMapper.null()) {
    mAutoMapper->OnNewInGameGuiState(mNextState, const_cast< CStateManager& >(mgr));
  }
  if ((mNextState == kIGGS_PauseGame || mNextState == kIGGS_PauseLogBook) &&
      mPauseScreen.null()) {
    mPauseScreen = rs_new CPauseScreen();
  }
  const bool paused = InGameGuiStates::IsPausedState(mNextState);
  for (rstl::vector< CToken >::iterator it = mPauseResources.begin();
       it != mPauseResources.end(); ++it) {
    if (paused) {
      it->Lock();
    } else {
      it->Unlock();
    }
  }
}

void CInGameGuiManager::InitializeDumpableARAMTextures() {
  if (!mIsSinglePlayer) {
    return;
  }

  int count = 0;
  for (AUTO(it, mInGameGuiDGRPs.begin()); it != mInGameGuiDGRPs.end(); ++it) {
    count += (*it)->GetCountForResType('TXTR');
  }
  mInGameTextureIds.reserve(count);

  for (AUTO(it, mInGameGuiDGRPs.begin()); it != mInGameGuiDGRPs.end(); ++it) {
    const rstl::vector< SObjectTag >& tags = (*it)->GetObjectTagVector();
    for (AUTO(tag, tags.begin()); tag != tags.end(); ++tag) {
      if (tag->GetType() == 'TXTR' &&
          rstl::find(mInGameTextureIds.begin(), mInGameTextureIds.end(), tag->GetId()) ==
              mInGameTextureIds.end()) {
        mInGameTextureIds.push_back(tag->GetId());
      }
    }
  }
  mInGameGuiDGRPs = rstl::vector< TToken< CDependencyGroup > >();

  const rstl::vector< SObjectTag >& tags = mPauseScreenDGRPs[2]->GetObjectTagVector();
  mPauseResources.reserve(tags.size());
  for (AUTO(it, tags.begin()); it != tags.end(); ++it) {
    mPauseResources.push_back(gpSimplePool->GetObj(*it));
  }
}

void CInGameGuiManager::PauseGame(const CStateManager& mgr, EInGameGuiState state) {
  if (state == kIGGS_QuitGame) {
    mQuitScreen = rs_new CInGameQuitScreen(mgr.GetViewportLayoutIndex());
    return;
  }
  gpController->SetMotorState(kIOP_Player1, kMS_Stop);
  CSfxManager::SetChannel(CSfxManager::kSC_PauseScreen);
  if (state != kIGGS_PauseHUDMessage) {
    CStreamAudioManager::StopSfx();
  }
  BeginStateTransition(state, mgr);
}

void CInGameGuiManager::ShowPauseGameHudMessage(const CStateManager& mgr, CAssetId message,
                                                float time) {
  mPauseGameHudMessage = message;
  mPauseGameHudTime = time;
  PauseGame(mgr, kIGGS_PauseHUDMessage);
}

bool CInGameGuiManager::IsInPausedState() const {
  if (!mQuitScreen.null()) {
    return true;
  }
  bool gameplay = false;
  if (InGameGuiStates::IsGameplayState(mPrevState) &&
      InGameGuiStates::IsGameplayState(mNextState)) {
    gameplay = true;
  }
  return !gameplay;
}

void CInGameGuiManager::EnsureStates(const CStateManager& mgr) {
  if (mDeferTransition && !mPauseScreenBlur->IsGameDraw()) {
    DestroyAreaTextures(mgr);
    mDeferTransition = false;
    DoStateTransition(mgr);
  }
}

bool CInGameGuiManager::IsTextureInPauseScreen(CAssetId id) const {
  TPauseScreenDGRPs::const_iterator groupIt = mPauseScreenDGRPs.begin();
  for (int i = 0; i < 3; ++i, ++groupIt) {
    TToken< CDependencyGroup > group = *groupIt;
    const rstl::vector< SObjectTag >& tags = group->GetObjectTagVector();
    for (rstl::vector< SObjectTag >::const_iterator it = tags.begin(); it != tags.end(); ++it) {
      if (it->id == id) {
        return true;
      }
    }
  }
  return false;
}

void CInGameGuiManager::DestroyAreaTextures(const CStateManager&) {
  // TODO: gather dumpable area textures and order their reloads by resource-file offset.
}

bool CInGameGuiManager::TryReloadAreaTextures() {
  bool complete = true;
  rstl::list< TDumpedTexture >::iterator it = mDumpedTextures.begin();
  while (it != mDumpedTextures.end()) {
    if (it->second->TryReloadBitmapData(*gpResourceFactory)) {
      it = mDumpedTextures.erase(it);
    } else {
      complete = false;
      ++it;
    }
  }
  return complete;
}

void CInGameGuiManager::StopSounds() {
  // TODO: destroy the quit screen and stop HUD sounds. The HUD declaration currently requires an
  // unused manager argument, but this entry point receives no manager; reconcile that API first.
}
