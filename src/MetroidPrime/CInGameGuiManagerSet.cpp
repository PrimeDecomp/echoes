#include "MetroidPrime/CInGameGuiManagerSet.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiFrameLoader.hpp"
#include "Kyoto/CDependencyGroup.hpp"
#include "Kyoto/CDependencyGroupToken.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CAutoMapper.hpp"
#include "MetroidPrime/CInGameGuiManager.hpp"
#include "MetroidPrime/CInGameQuitScreen.hpp"
#include "MetroidPrime/CMessageScreen.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CMultiplayerGui.hpp"
#include "MetroidPrime/CPauseScreen.hpp"
#include "MetroidPrime/CPauseScreenBlur.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CTurretHud.hpp"
#include "MetroidPrime/HUD/CSamusHud.hpp"
#include "MetroidPrime/Player/CPlayerVisor.hpp"
#include "MetroidPrime/Player/CSamusFaceReflection.hpp"

static const char* const skMemoFrameNames[] = {"FRME_HudMessage", "FRME_HudMessage2",
                                               "FRME_HudMessage4"};

CInGameGuiManagerSet::CInGameGuiManagerSet(const CStateManager&, CArchitectureQueue&)
: mPreloadDGRP(gpSimplePool->GetObj("PreLoadIGGM_DGRP"))
, mLoadPhase(kLP_LoadDepsGroup)
, mRandom(1234) {
  mPreloadDGRP.Lock();
}

CInGameGuiManagerSet::~CInGameGuiManagerSet() {}

bool CInGameGuiManagerSet::CheckPlayerGuiLoadComplete(const CStateManager& mgr) {
  for (int i = 0; i < mPlayerGuiManagers.size(); ++i) {
    if (!mPlayerGuiManagers[i]->CheckLoadComplete(mgr)) {
      return false;
    }
  }
  return true;
}

inline CMultiplayerGui::~CMultiplayerGui() {}

inline CSamusFaceReflection::~CSamusFaceReflection() {}

inline CInGameGuiManager::~CInGameGuiManager() {}

bool CInGameGuiManagerSet::CheckLoadComplete(const CStateManager& mgr) {
  switch (mLoadPhase) {
  case kLP_LoadDepsGroup:
    if (mPreloadDGRP.IsLoaded()) {
      const rstl::vector< SObjectTag >& tags = mPreloadDGRP->GetObjectTagVector();
      mPreloadTokens.reserve(tags.size());
      for (rstl::vector< SObjectTag >::const_iterator it = tags.begin(); it != tags.end(); ++it) {
        CToken token = gpSimplePool->GetObj(*it);
        token.Lock();
        mPreloadTokens.push_back_unsafe(token);
      }
      mPreloadDGRP.Unlock();
      const char* hudFrameName = CSamusHud::GetHudFrameName(mgr.GetViewportLayoutIndex());
      mHudFrameLoader =
          rs_new CGuiFrameLoader(gpResourceFactory->GetResourceIdByName(hudFrameName)->id,
                                 *gpResourceFactory, *gpSimplePool);
      mMemoFrameLoader = rs_new CGuiFrameLoader(
          gpResourceFactory->GetResourceIdByName(skMemoFrameNames[mgr.GetViewportLayoutIndex()])
              ->id,
          *gpResourceFactory, *gpSimplePool);
      if (!mgr.IsMultiplayer()) {
        mDarkMaskFrameLoader =
            rs_new CGuiFrameLoader(gpResourceFactory->GetResourceIdByName("FRME_DarkVisorMask")->id,
                                   *gpResourceFactory, *gpSimplePool);
      } else {
        mMultiplayerGui = rs_new CMultiplayerGui(mgr);
      }
      if (mgr.GetViewportLayoutIndex() == 0) {
        mHelmetFrameLoader =
            rs_new CGuiFrameLoader(gpResourceFactory->GetResourceIdByName("FRME_Helmet")->id,
                                   *gpResourceFactory, *gpSimplePool);
      }
      mLoadPhase = kLP_PreloadDeps;
    } else {
      return false;
    }
  // Fall through: each phase can complete in the same call.
  case kLP_PreloadDeps:
    if (!mHudFrameLoader->IsFinishedLoading()) {
      return false;
    }
    if (!mMemoFrameLoader->IsFinishedLoading()) {
      return false;
    }
    if (!mDarkMaskFrameLoader.null() && !mDarkMaskFrameLoader->IsFinishedLoading()) {
      return false;
    }
    if (!mHelmetFrameLoader.null() && !mHelmetFrameLoader->IsFinishedLoading()) {
      return false;
    }
    for (rstl::vector< CToken >::const_iterator it = mPreloadTokens.begin();
         it != mPreloadTokens.end(); ++it) {
      if (!it->IsLoaded()) {
        return false;
      }
    }
    mLoadPhase = kLP_LoadPlayerGui;
    for (uint i = 0; i < uint(mgr.GetNumPlayers()); ++i) {
      mPlayerGuiManagers.push_back(rstl::auto_ptr< CInGameGuiManager >(
          rs_new CInGameGuiManager(mgr, *mHudFrameLoader, *mMemoFrameLoader,
                                   mHelmetFrameLoader.get(), mDarkMaskFrameLoader.get(), i)));
    }
    mHudFrameLoader = nullptr;
    // Fall through.
  case kLP_LoadPlayerGui:
    if (!CheckPlayerGuiLoadComplete(mgr)) {
      return false;
    }
    mPreloadTokens = rstl::vector< CToken >();
    for (rstl::reserved_vector< rstl::auto_ptr< CInGameGuiManager >, 4 >::iterator it =
             mPlayerGuiManagers.begin();
         it != mPlayerGuiManagers.end(); ++it) {
      (*it)->BeginStateTransition(kIGGS_InGame, mgr);
    }
    mLoadPhase = kLP_Done;
    return true;
  case kLP_Done:
    return true;
  default:
    return false;
  }
}

void CInGameGuiManagerSet::StartFadeIn() {
  mFadeFilter.SetFilter(CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_Fullscreen, 0.f,
                        CColor::Black(), kInvalidAssetId);
  mFadeFilter.DisableFilter(0.5f);
}

void CInGameGuiManagerSet::Draw(const CStateManager& mgr, int playerIndex) const {
  if (!GetIsGameDraw()) {
    gpRender->SetRequestRGBA6(true);
  }
  mPlayerGuiManagers[playerIndex]->Draw(mgr);
  mFadeFilter.Draw();
}

bool CInGameGuiManagerSet::IsInPausedState() const {
  for (int i = 0; i < mPlayerGuiManagers.size(); ++i) {
    if (mPlayerGuiManagers[i]->IsInPausedState()) {
      return true;
    }
  }
  return false;
}

void CInGameGuiManagerSet::PrepareScanDisplay(const CStateManager& mgr, int playerIndex) {
  mPlayerGuiManagers[playerIndex]->PrepareScanDisplay(mgr, playerIndex);
}

void CInGameGuiManagerSet::PreDraw(CStateManager& mgr, bool cameraActive) {
  for (rstl::reserved_vector< rstl::auto_ptr< CInGameGuiManager >, 4 >::iterator it =
           mPlayerGuiManagers.begin();
       it != mPlayerGuiManagers.end(); ++it) {
    (*it)->PreDraw(mgr, cameraActive);
  }
}

void CInGameGuiManagerSet::PauseGame(const CStateManager& mgr, EInGameGuiState state) {
  for (rstl::reserved_vector< rstl::auto_ptr< CInGameGuiManager >, 4 >::iterator it =
           mPlayerGuiManagers.begin();
       it != mPlayerGuiManagers.end(); ++it) {
    (*it)->PauseGame(mgr, state);
  }
}

void CInGameGuiManagerSet::Update(const CStateManager& mgr, float dt, CArchitectureQueue& queue,
                                  bool cameraActive, int playerIndex) {
  mPlayerGuiManagers[playerIndex]->Update(mgr, dt, mRandom, queue, cameraActive);
  mFadeFilter.Update(dt);
}

void CInGameGuiManagerSet::UpdateMultiplayerGui(float dt, const CStateManager& mgr) {
  if (!mMultiplayerGui.null()) {
    mMultiplayerGui->Update(dt, mgr);
  }
}

void CInGameGuiManagerSet::DrawMultiplayerGui() const {
  if (!mMultiplayerGui.null()) {
    mMultiplayerGui->Draw();
  }
}

void CInGameGuiManagerSet::ProcessControllerInput(const CStateManager& mgr,
                                                  const CFinalInput& input,
                                                  CArchitectureQueue& queue) {
  int i = 0;
  const int controller = input.ControllerNumber();
  for (; i < mgr.GetNumPlayers(); ++i) {
    const int selection = mgr.GetPlayerState(i)->GetPlayerSelection();
    if (selection == controller) {
      mPlayerGuiManagers[i]->ProcessControllerInput(mgr, input, queue);
      break;
    }
  }
}

bool CInGameGuiManagerSet::GetIsGameDraw() const {
  if (mPlayerGuiManagers.size() != 1) {
    return true;
  }
  return mPlayerGuiManagers[0]->GetIsGameDraw();
}

void CInGameGuiManagerSet::StopSounds(const CStateManager& mgr) {
  if (mPlayerGuiManagers.size() == 1) {
    mPlayerGuiManagers[0]->StopSounds(mgr);
  }
}
