#include "MetroidPrime/CMFGame.hpp"

#include "MetroidPrime/CInGameGuiManagerSet.hpp"
#include "MetroidPrime/CInGameGuiManager.hpp"
#include "MetroidPrime/CAutoMapper.hpp"
#include "MetroidPrime/CDecalManager.hpp"
#include "MetroidPrime/CPortalTransition.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CArchitectureQueue.hpp"
#include "MetroidPrime/Decode.hpp"
#include "MetroidPrime/Player/CGameMode.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"

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
, mPortalTransitionTime(0.f)
, mTransitionPhase(kTP_None)
, mTransitionFadeTime(1.f)
, mInitialized(false)
, mPlayerAlive(true)
, mEndGameFrameCaptured(false)
, mTransitionFromDarkWorld(false)
, x44_4(false) {
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

void CMFGame::RecordMultiplayerResults() const {}

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

void CMFGame::Draw() const {}

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

void CMFGame::DrawGui(bool singleViewport) const {}

void CMFGame::DrawWorld(bool singleViewport) const {}

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
