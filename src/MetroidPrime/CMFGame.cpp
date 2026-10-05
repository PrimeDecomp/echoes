#include "MetroidPrime/CMFGame.hpp"

#include "MetroidPrime/CInGameGuiManagerSet.hpp"
#include "MetroidPrime/CInGameGuiManager.hpp"
#include "MetroidPrime/CAutoMapper.hpp"
#include "MetroidPrime/CDecalManager.hpp"
#include "MetroidPrime/CPortalTransition.hpp"
#include "MetroidPrime/CSaveGameScreen.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CArchitectureQueue.hpp"
#include "MetroidPrime/Decode.hpp"
#include "MetroidPrime/Player/CGameMode.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "rstl/math.hpp"

bool CMFGame::mMultiplayerGuiActive;

CMFGame::CMFGame(rstl::ncrc_ptr< CStateManager > stateManager,
                 const rstl::ncrc_ptr< CInGameGuiManagerSet >& guiManager,
                 CArchitectureQueue& architectureQueue)
: CIOWin(rstl::string_l("CMFGame"))
, mStateManager(stateManager)
, mGuiManager(guiManager)
, mFlowState(kFS_InGame)
, mFlowTime(0.f)
, mMultiplayerEndFadeTime(0.f)
, mSkippedCineCam(kInvalidUniqueId)
, mPortalTransition(nullptr)
, mTransitionPhase(kTP_None)
, mTransitionFadeTime(1.f)
, mInitialized(false)
, mPlayerAlive(true)
, mEndGameFrameCaptured(false)
, mTransitionFromDarkWorld(false) {
  gpMain->SetGameFlowBuilt(true);
  mMultiplayerGuiActive = false;
  gpGameState->PreviousGameResults().mShowResults = false;
}

CMFGame::~CMFGame() {
  gpMain->SetGameFlowBuilt(false);
  gpMain->SetMaxSpeed(false);
  CSfxManager::SetMuted(false);
  CDecalManager::Reinitialize();
  CGraphics::SetViewport(0, 0, 640, 448);
  CGraphics::SetScissor(0, 0, 640, 448);
}

void CMFGame::ActivateMultiplayerGui() { mMultiplayerGuiActive = true; }

void CMFGame::RecordMultiplayerResults() const {
  const bool showResults = gpGameState->PreviousGameResults().mShowResults;
  CGameMode& mode = gpGameState->GetGameMode();
  const int gameMode = mode.GetGameModeType();
  const int result = mode.GetResultIndex();
  const int playerCount = mode.GetNumPlayers();
  rstl::reserved_vector< CGameState::SPlayerResult, 4 > players;

  if (gameMode == 'DTHM') {
    for (int i = 0; i < playerCount; ++i) {
      CGameState& state = *gpGameState;
      const CPlayerState& player = *mStateManager->GetPlayerState(i);
      const uint selection = player.GetPlayerSelection();
      const int score = player.GetItemAmount(CPlayerState::kIT_FragCount);
      const int deaths = player.GetItemAmount(CPlayerState::kIT_DiedCount);
      const CPlayerOptions& options = state.GameOptions().PlayerOptions(i);
      players.push_back(CGameState::SPlayerResult(selection, score, deaths,
                                                options.GetUnknownFlag(), options.GetRumbleEnabled()));
    }
  } else if (gameMode == 'COIN') {
    for (int i = 0; i < playerCount; ++i) {
      CGameState& state = *gpGameState;
      const CPlayerState& player = *mStateManager->GetPlayerState(i);
      const uint selection = player.GetPlayerSelection();
      const int score = player.GetItemAmount(CPlayerState::kIT_CoinCounter);
      const int deaths = player.GetItemAmount(CPlayerState::kIT_DiedCount);
      const CPlayerOptions& options = state.GameOptions().PlayerOptions(i);
      players.push_back(CGameState::SPlayerResult(selection, score, deaths,
                                                options.GetUnknownFlag(), options.GetRumbleEnabled()));
    }
  }
  while (players.size() < 4) {
    players.push_back(CGameState::SPlayerResult());
  }
  gpGameState->PreviousGameResults() =
      CGameState::SPreviousGameResults(gameMode, showResults, result, playerCount, players);
}

CGameState::SPreviousGameResults::SPreviousGameResults(
    uint gameMode, bool showResults, int modeResult, int playerCount,
    const rstl::reserved_vector< SPlayerResult, 4 >& players)
: mGameMode(gameMode)
, mShowResults(showResults)
, x8_(modeResult)
, mPlayerCount(playerCount)
, mPlayers(players) {}

void CMFGame::FinishMultiplayerGame() {
  gpGameState->PreviousGameResults().mShowResults = true;
  gpMain->SetRestartMode(CMain::kRM_Default);
  SetFlowState(kFS_MultiplayerResults);
}

void CMFGame::EndGame(CArchitectureQueue& queue) {
  if (gpGameState->GetGameMode().GetResultIndex() == 0) {
    gpMain->SetRestartMode(CMain::kRM_EndMovie2);
    queue.Push(MakeMsg::CreateQuitGameplay(kAMT_Game));
  } else {
    gpGameState->PreviousGameResults().mShowResults = true;
    CGraphics::SetIsBeginSceneClearFb(false);
    ActivateMultiplayerGui();
  }
}

bool CMFGame::IsCameraActiveFlow() const {
  return mFlowState == kFS_InGame || mFlowState == kFS_PlayerDied ||
         (mFlowState == kFS_PortalTransition && x44_4);
}

void CMFGame::PlayerDied() {
  SetFlowState(kFS_PlayerDied);
  mPlayerAlive = false;
}

void CMFGame::UnpauseGame() {
  SetFlowState(kFS_InGame);
  CSfxManager::SetChannel(CSfxManager::kSC_Game);
  mStateManager->DeferStateTransition(kSMT_InGame);
}

void CMFGame::EnterMessageScreen(float time) {
  SetFlowState(kFS_Paused);
  mGuiManager->GetPlayerGuiManager(0).ShowPauseGameHudMessage(
      *mStateManager, mStateManager->GetPauseHUDMessage(), time);
}

void CMFGame::EnterPauseScreenState5() {
  SetFlowState(kFS_Paused);
  mGuiManager->GetPlayerGuiManager(0).PauseGame(*mStateManager, kIGGS_PauseSaveGame);
}

void CMFGame::SaveGame() {
  SetFlowState(kFS_Paused);
  // The G2ME01 call passes GUI state 4 for this path.
  mGuiManager->PauseGame(*mStateManager, kIGGS_PauseLogBook);
}

void CMFGame::EnterLogBook() {
  SetFlowState(kFS_Paused);
  // The G2ME01 call passes GUI state 3 for this path.
  mGuiManager->PauseGame(*mStateManager, kIGGS_PauseGame);
}

void CMFGame::PauseGame() {
  SetFlowState(kFS_Paused);
  CInGameGuiManager& gui = mGuiManager->GetPlayerGuiManager(0);
  gui.GetAutoMapper().SetMapMode(CAutoMapper::kMM_Teleport);
  gui.PauseGame(*mStateManager, kIGGS_MapScreen);
  mStateManager->SetInMapScreen(true);
}

void CMFGame::EnterMapScreen() {
  SetFlowState(kFS_Paused);
  CInGameGuiManager& gui = mGuiManager->GetPlayerGuiManager(0);
  gui.GetAutoMapper().SetMapMode(CAutoMapper::kMM_Normal);
  gui.PauseGame(*mStateManager, kIGGS_MapScreen);
  mStateManager->SetInMapScreen(true);
}

void CMFGame::Draw() const {
  if (!mStateManager.IsNull()) {
    mStateManager->Touch();
  }
  const bool singleViewport =
      mStateManager->IsMultiplayer() && mStateManager->GetCameraManager(0)->fn_801ABD68();

  switch (mFlowState) {
  case kFS_InGame:
  case kFS_Paused:
  case kFS_PlayerDied:
    CGraphics::SetUseStreamVertexDelay(mFlowState == kFS_Paused);
    if (mStateManager->mPendingDockArea == kInvalidAreaId || mTransitionPhase != kTP_Load) {
      DrawWorld(singleViewport);
      DrawGui(singleViewport);
    }
    if (mTransitionPhase == kTP_FadeOut || mTransitionPhase == kTP_Complete) {
      DrawTransitionFilter();
    }
    CGraphics::SetUseStreamVertexDelay(false);
    return;
  case kFS_CinematicSkip: {
    if (mFlowTime >= 1.f) {
      gpMain->SetMaxSpeed(true);
      return;
    }
    DrawWorld(singleViewport);
    DrawGui(singleViewport);
    const float intensity = CMath::Clamp(0.f, 1.f - mFlowTime, 1.f);
    CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Multiply,
                                  CCameraFilterPass::kFS_Fullscreen,
                                  CColor(intensity, intensity, intensity, 1.f), nullptr, 1.f);
    return;
  }
  case kFS_PortalTransition:
    if (mTransitionPhase > kTP_FadeOut && mTransitionPhase < kTP_Complete &&
        mPortalTransition.get()) {
      mPortalTransition->Draw();
    }
    if (mTransitionPhase != kTP_Play) {
      DrawTransitionFilter();
    }
    return;
  case kFS_State8: {
    if (mFlowTime >= 1.f / 60.f) {
      return;
    }
    DrawWorld(singleViewport);
    DrawGui(singleViewport);
    const float intensity = rstl::min_val(1.f, (1.f / 60.f - mFlowTime) / (1.f / 60.f));
    CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Multiply,
                                  CCameraFilterPass::kFS_Fullscreen,
                                  CColor(intensity, intensity, intensity, 1.f), nullptr, 1.f);
    break;
  }
  case kFS_MultiplayerEndFade:
  case kFS_MultiplayerResults:
    break;
  default:
    return;
  }

  if (!mEndGameFrameCaptured) {
    mEndGameFrameCaptured = true;
    DrawWorld(singleViewport);
    DrawGui(singleViewport);
    CGraphics::SetViewport(0, 0, CGraphics::GetRenderMode().fbWidth,
                          CGraphics::GetRenderMode().xfbHeight);
    CGraphics::SetScissor(0, 0, CGraphics::GetRenderMode().fbWidth,
                         CGraphics::GetRenderMode().xfbHeight);
    CCameraBlurPass::GetFbCopy(GX_TF_RGB565);
    CGraphics::SetIsBeginSceneClearFb(false);
  } else {
    CGraphics::SetViewport(0, 0, CGraphics::GetRenderMode().fbWidth,
                          CGraphics::GetRenderMode().xfbHeight);
    CGraphics::SetScissor(0, 0, CGraphics::GetRenderMode().fbWidth,
                         CGraphics::GetRenderMode().xfbHeight);
    const float amount = (8.f * (mMultiplayerEndFadeTime * mMultiplayerEndFadeTime) + 1.f) /
                         (CGraphics::Is50Hz() ? 50.f : 60.f);
    CColor color = CColor::White();
    color.SetAlpha(amount);
    CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Add,
                                  CCameraFilterPass::kFS_Fullscreen, color, nullptr, 1.f);
  }
}

void CMFGame::DrawTransitionFilter() const {
  if (mStateManager->GetShowSoftTransition()) {
    const bool fadeOut = mTransitionPhase == kTP_Load || mTransitionPhase == kTP_Complete;
    const float amount = fadeOut ? 1.f - mTransitionFadeTime : mTransitionFadeTime;
    CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Add,
                                  CCameraFilterPass::kFS_Fullscreen,
                                  CColor::Lerp(CColor::White(), CColor::Black(), amount), nullptr,
                                  1.f);
  }
}

void CMFGame::DrawGui(bool singleViewport) const {
  if (!singleViewport) {
    for (uint i = 0; i < uint(mStateManager->GetNumPlayers()); ++i) {
      mStateManager->SetupPlayerViewport(i);
      mGuiManager->PreDraw(*mStateManager, IsCameraActiveFlow());
      mGuiManager->Draw(*mStateManager, i);
      mStateManager->EndPlayerRender();
    }
  }
  CGraphics::SetViewport(0, 0, 640, 448);
  CGraphics::SetScissor(0, 0, 640, 448);
  if (mGuiManager->GetIsGameDraw() && mStateManager->IsMultiplayer() && !singleViewport) {
    mGuiManager->DrawMultiplayerGui();
  }
  if (mStateManager->mSaveGameScreen.get()) {
    mStateManager->mSaveGameScreen->Draw();
  }
}

void CMFGame::DrawWorld(bool singleViewport) const {
  const uint playerCount = singleViewport ? 1 : mStateManager->GetNumPlayers();
  if (mGuiManager->GetIsGameDraw()) {
    gpMain->SetGameFrameDrawn(true);
    bool usedViewports[4] = {false, false, false, false};
    for (uint i = 0; i < playerCount; ++i) {
      usedViewports[gpGameState->GetPlayerState(i)->GetPlayerSelection()] = true;
      mStateManager->PreRender(i);
      mGuiManager->PrepareScanDisplay(*mStateManager, i);
      mStateManager->DrawWorld(*mGuiManager);
      mStateManager->EndPlayerRender();
    }
    if (playerCount == 3) {
      for (int i = 0; i < 4; ++i) {
        if (!usedViewports[i]) {
          mStateManager->DrawUnusedViewport(i);
        }
      }
    }
  }
}

CIOWin::EMessageReturn CMFGame::OnMessage(const CArchitectureMessage& message,
                                          CArchitectureQueue& queue) {}

void CMFGame::SetFlowState(EFlowState state) {
  if (mFlowState == kFS_CinematicSkip) {
    gpMain->SetMaxSpeed(false);
    mGuiManager->StartFadeIn();
    mSkippedCineCam = kInvalidUniqueId;
    CSfxManager::SetMuted(false);
  }

  mFlowState = state;
  switch (state) {
  case kFS_CinematicSkip:
    CSfxManager::SetMuted(true);
    break;
  case kFS_PortalTransition:
    mStateManager->DeferStateTransition(kSMT_InGame);
    break;
  case kFS_MultiplayerEndFade:
    CSfxManager::SetChannel(CSfxManager::kSC_PauseScreen);
    CStreamAudioManager::FadeOutSoftwareAudio(CStreamAudioManager::kSC_Default, 0.5f);
    CStreamAudioManager::PlaySoftwareAudio(
        CStreamAudioManager::kSC_OneShot,
        rstl::string_l("/Audio/multi-defbgm-speed-doon32.dsp"), 0.01f, 0.01f, 90, true);
    break;
  default:
    break;
  }
}
