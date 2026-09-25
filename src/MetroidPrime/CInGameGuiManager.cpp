#include "MetroidPrime/CInGameGuiManager.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiFrameLoader.hpp"
#include "Kyoto/CDependencyGroup.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/HUD/CSamusHud.hpp"

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
  for (int i = 0; i < mPauseScreenDGRPs.size(); ++i) {
    if (!mPauseScreenDGRPs[i].IsLoaded()) {
      return false;
    }
  }
  for (int i = 0; i < mInGameGuiDGRPs.size(); ++i) {
    if (!mInGameGuiDGRPs[i].IsLoaded()) {
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
  // TODO: query CPauseScreenBlur's game-draw state.
  return false;
}

void CInGameGuiManager::PrepareScanDisplay(const CStateManager& mgr, int playerIndex) {
  if (mSamusHud != nullptr) {
    mSamusHud->PrepareScanDisplay(mgr, playerIndex);
  }
}

void CInGameGuiManager::PreDraw(CStateManager&, bool) {
  // TODO: prepare the face reflection when this player's camera is active.
}

void CInGameGuiManager::DrawDarkVisorMask() const {
  // TODO: set the outer-mask visibility and draw the loaded dark-visor frame.
}

void CInGameGuiManager::fn_80225a30(float, const CStateManager&, const CColor&, const CColor&,
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
  // TODO: test both the blur transition and the automapper transition.
  return false;
}

void CInGameGuiManager::TryCompleteStateTransition() {
  // TODO: release obsolete screens and finish texture reloads before accepting the next state.
}

void CInGameGuiManager::BeginStateTransition(EInGameGuiState, const CStateManager&) {
  // TODO: update state, defer texture dumping when necessary, and transition blur/map/audio.
}

void CInGameGuiManager::DoStateTransition(const CStateManager&) {
  // TODO: construct the appropriate pause or message screen and update resource locks.
}

void CInGameGuiManager::InitializeDumpableARAMTextures() {
  // TODO: collect unique in-game texture IDs and retain the pause-screen resources.
}

void CInGameGuiManager::PauseGame(const CStateManager&, EInGameGuiState) {
  // TODO: open the quit screen or begin the requested pause transition and switch audio channels.
}

void CInGameGuiManager::ShowPauseGameHudMessage(const CStateManager& mgr, CAssetId message,
                                                float time) {
  mPauseGameHudMessage = message;
  mPauseGameHudTime = time;
  PauseGame(mgr, kIGGS_PauseHUDMessage);
}

bool CInGameGuiManager::IsInPausedState() const {
  return mQuitScreen != nullptr || !InGameGuiStates::IsGameplayState(mPrevState) ||
         !InGameGuiStates::IsGameplayState(mNextState);
}

void CInGameGuiManager::EnsureStates(const CStateManager&) {
  // TODO: dump area textures once the deferred blur transition stops drawing the game.
}

bool CInGameGuiManager::IsTextureInPauseScreen(CAssetId id) const {
  for (int i = 0; i < mPauseScreenDGRPs.size(); ++i) {
    TToken< CDependencyGroup > group = mPauseScreenDGRPs[i].NonConstCopy();
    const rstl::vector< SObjectTag >& tags = group->GetObjectTagVector();
    for (int j = 0; j < tags.size(); ++j) {
      if (tags[j].id == id) {
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
  // TODO: reload and erase completed entries through CTexture's bitmap-reload interface.
  return mDumpedTextures.empty();
}

void CInGameGuiManager::StopSounds() {
  // TODO: destroy the quit screen and stop HUD sounds. The HUD declaration currently requires an
  // unused manager argument, but this entry point receives no manager; reconcile that API first.
}
