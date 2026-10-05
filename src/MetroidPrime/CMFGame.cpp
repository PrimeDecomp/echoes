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
#include "MetroidPrime/CGameGlobalObjects.hpp"
#include "MetroidPrime/CGameResultsScreen.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CWorldTransManager.hpp"
#include "MetroidPrime/Cameras/CCinematicCamera.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "Kyoto/Audio/CMidiManager.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/Input/IController.hpp"

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

void CMFGame::FinishMultiplayerGame(CArchitectureQueue& queue) {
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
                                          CArchitectureQueue& queue) {
  switch (message.GetType()) {
  case kAM_FrameBegin:
    mStateManager->FrameBegin(MakeMsg::GetParmFrameBegin(message).GetInt32());
    break;
  case kAM_TimerTick: {
    const bool wasInitialized = mInitialized;
    mInitialized = true;
    const float dt = MakeMsg::GetParmTimerTick(message).GetReal();
    if (!mStateManager->mSaveGameScreen.null() &&
        mStateManager->mSaveGameScreen->Update(dt) != kMR_Normal && mFlowState == kFS_Paused) {
      mStateManager->DeleteSaveGameScreen();
    }

    if (mTransitionPhase != kTP_None) {
      if (mStateManager->GetShowSoftTransition()) {
        mTransitionFadeTime = rstl::max_val(0.f, mTransitionFadeTime - dt);
      } else {
        mTransitionFadeTime = 0.f;
      }
      switch (mTransitionPhase) {
      case kTP_FadeOut:
        if (mTransitionFadeTime == 0.f) {
          mTransitionPhase = kTP_Load;
          mTransitionFadeTime = 1.f;
        }
        break;
      case kTP_Load:
        if (mPortalTransition.null() || mPortalTransition->IsReady()) {
          if (mTransitionFadeTime == 0.f) {
            mTransitionPhase = kTP_Play;
          }
        } else {
          mTransitionFadeTime = 1.f;
        }
        break;
      case kTP_FadeIn:
        if (mTransitionFadeTime != 0.f) {
          break;
        }
        mTransitionPhase = kTP_Complete;
        mTransitionFadeTime = 1.f;
        // Fall through.
      case kTP_Complete:
        if (mTransitionFadeTime == 0.f) {
          mTransitionPhase = kTP_None;
          mTransitionFadeTime = 1.f;
        }
        break;
      }
    }
    if (mFlowState == kFS_Zero) {
      SetFlowState(kFS_InGame);
    }

    switch (mFlowState) {
    case kFS_PortalTransition:
      for (uint i = 0; i < uint(mStateManager->GetNumPlayers()); ++i) {
        mGuiManager->Update(*mStateManager, dt, queue, IsCameraActiveFlow(), i);
      }
      mPortalTransitionTime += dt;
      if (mTransitionPhase > kTP_FadeOut && mTransitionPhase < kTP_Complete &&
          !mPortalTransition.null()) {
        mPortalTransition->Update(dt);
      }
      if (!mStateManager->IsFullyInitialized()) {
        mStateManager->InitializeState(mStateManager->GetWorld()->GetWorldAssetId(),
                                       mStateManager->mPendingDockArea, kInvalidAssetId);
        return kMR_Exit;
      }
      if (mPortalTransition.null() || mPortalTransition->IsFinished()) {
        if (mTransitionPhase == kTP_Play) {
          mTransitionPhase = kTP_FadeIn;
          mTransitionFadeTime = 1.f;
        }
        if (mTransitionPhase == kTP_Complete) {
          mStateManager->SetRandomAvailable(true);
          mStateManager->Update(dt, queue);
          mStateManager->SetRandomAvailable(false);
          mStateManager->mPendingDockArea = kInvalidAreaId;
          SetFlowState(kFS_InGame);
          mPortalTransition = nullptr;
        }
      }
      return kMR_Exit;
    case kFS_State8:
      mFlowTime += dt;
      mStateManager->UpdateDynamicLayers();
      if (!mStateManager->HasPendingLayerLoads() && mFlowTime >= 1.f / 60.f) {
        mStateManager->mUnkFlagB4 = false;
        SetFlowState(kFS_InGame);
      }
      return kMR_Exit;
    case kFS_CinematicSkip: {
      mFlowTime += dt;
      const CCinematicCamera* cineCam = TCastToConstPtr< CCinematicCamera >(
          *mStateManager->GetCameraManager(0)->GetCurrentCamera(*mStateManager, true));
      bool finished = true;
      if (cineCam) {
        finished = false;
        const bool canSkip = (cineCam->GetFlags() & 0x800) ||
                             ((cineCam->GetFlags() & 8) && cineCam->CanSkip(*mStateManager));
        if (canSkip && mFlowTime >= 1.f && mStateManager->SpecialSkipCinematic() == 1) {
          finished = true;
        }
        if ((cineCam->GetFlags() & 0x10) && mSkippedCineCam != cineCam->GetScriptCameraId()) {
          finished = true;
        }
      }
      if (finished) {
        SetFlowState(kFS_InGame);
        break;
      }
    }
    // Fall through.
    case kFS_InGame:
      if (mStateManager->mPendingDockArea != kInvalidAreaId) {
        if (mTransitionPhase == kTP_None) {
          mTransitionPhase = kTP_FadeOut;
          mTransitionFromDarkWorld = mStateManager->GetIsDarkWorld();
        }
        if (mTransitionPhase == kTP_Load) {
          mStateManager->SetRandomAvailable(true);
          if (mStateManager->PrepareAreaTransition(mStateManager->mPendingDockArea)) {
            SetFlowState(kFS_PortalTransition);
            mPortalTransition = mStateManager->TakePortalTransition();
            mPortalTransitionTime = 0.f;
            if (!mGuiManager.IsNull()) {
              mGuiManager->StopSounds(*mStateManager);
            }
          } else {
            mTransitionFadeTime = 1.f;
          }
          mStateManager->SetRandomAvailable(false);
        }
        break;
      }

      mStateManager->SetRandomAvailable(true);
      switch (mStateManager->mDeferredTransition) {
      case kSMT_InGame:
        mStateManager->Update(dt, queue);
        if (mStateManager->mQuitGame) {
          CGraphics::SetIsBeginSceneClearFb(false);
        }
        break;
      case kSMT_MapScreen:
        EnterMapScreen();
        break;
      case kSMT_PauseGame:
        PauseGame();
        break;
      case kSMT_Unk:
        EnterLogBook();
        break;
      case kSMT_LogBook:
        SaveGame();
        break;
      case kSMT_SaveGame:
        EnterPauseScreenState5();
        break;
      case kSMT_MessageScreen:
        EnterMessageScreen(mStateManager->mHudMessageTime);
        break;
      }
      if (gpGameState->GetGameMode().IsGameOver()) {
        if (mStateManager->IsMultiplayer()) {
          SetFlowState(kFS_MultiplayerEndFade);
        } else {
          EndGame(queue);
        }
      }
      if (!mStateManager->IsMultiplayer() && mPlayerAlive &&
          !mStateManager->GetPlayerState(0)->IsPlayerAlive()) {
        PlayerDied();
      }
      mStateManager->SetRandomAvailable(false);
      break;
    case kFS_Paused:
      if (!mGuiManager->IsInPausedState()) {
        UnpauseGame();
        if (mStateManager->GetPauseHUDMessage() != kInvalidAssetId) {
          mStateManager->IncrementHUDMessageFrameCounter();
        }
      }
      break;
    case kFS_PlayerDied:
      if (gpGameState->GetGameMode().IsGameOver()) {
        EndGame(queue);
      } else {
        mStateManager->SetRandomAvailable(true);
        mStateManager->Update(dt, queue);
        mStateManager->SetRandomAvailable(false);
      }
      break;
    case kFS_MultiplayerEndFade:
      if (mMultiplayerEndFadeTime < 1.f) {
        mMultiplayerEndFadeTime = rstl::min_val(mMultiplayerEndFadeTime + dt, 1.f);
        if (mMultiplayerEndFadeTime == 1.f) {
          FinishMultiplayerGame(queue);
        }
      }
      break;
    case kFS_MultiplayerResults:
      if (mEndGameFrameCaptured) {
        CIOWin* screen = rs_new CGameResultsScreen(*gpGameState);
        queue.Push(MakeMsg::CreateCreateIOWin(kAMT_IOWinManager, 9, 10, screen));
        queue.Push(MakeMsg::CreateQuitGameplay(kAMT_Game));
      }
      break;
    }
    for (uint i = 0; i < uint(mStateManager->GetNumPlayers()); ++i) {
      mGuiManager->Update(*mStateManager, dt, queue, IsCameraActiveFlow(), i);
    }
    if (IsCameraActiveFlow()) {
      mGuiManager->UpdateMultiplayerGui(dt, *mStateManager);
    }
    if (!wasInitialized) {
      gpGameState->WorldTransitionManager()->EndTransition();
    }
    return kMR_Exit;
  }
  case kAM_UserInput: {
    if (!mInitialized || (mTransitionPhase != kTP_None && mTransitionPhase != kTP_Complete)) {
      break;
    }
    CArchMsgParmUserInput parm = MakeMsg::GetParmUserInput(message);
    CFinalInput input = parm.GetUserInput();
    bool stopRumble = true;
    if (input.ControllerNumber() == 0 && !mStateManager->mSaveGameScreen.null()) {
      mStateManager->mSaveGameScreen->ProcessUserInput(input);
    }
    if (mFlowState == kFS_InGame) {
      if (mMultiplayerGuiActive) {
        queue.Push(MakeMsg::CreateQuitGameplay(kAMT_Game));
        mMultiplayerGuiActive = false;
      }
      if (mStateManager->mUnkFlagB4) {
        SetFlowState(kFS_State8);
        mFlowTime = 0.f;
        return kMR_Normal;
      }
      const CCinematicCamera* cineCam = TCastToConstPtr< CCinematicCamera >(
          *mStateManager->GetCameraManager(0)->GetCurrentCamera(*mStateManager, true));
      if (input.PStart() && !cineCam && gpGameState->GetGameMode().GetGameModeType() != 'FRND' &&
          gpGameState->GetGameMode().GetGameModeType() != 'SNGL') {
        for (uint i = 0; i < uint(mStateManager->GetNumPlayers()); ++i) {
          if (mStateManager->GetPlayerState(i)->GetPlayerSelection() == input.ControllerNumber()) {
            mGuiManager->GetPlayerGuiManager(i).PauseGame(*mStateManager, kIGGS_QuitGame);
            SetFlowState(kFS_Paused);
            break;
          }
        }
      }
      if (input.ControllerNumber() == 0 && input.PStart()) {
        if (cineCam) {
          const bool canSkip = (cineCam->GetFlags() & 0x800) ||
                               ((cineCam->GetFlags() & 8) && cineCam->CanSkip(*mStateManager));
          if (canSkip && !mStateManager->IsMultiplayer() &&
              gpGameState->GetGameMode().GetGameModeType() != 'FRND') {
            CMidiManager::StopAll();
            mSkippedCineCam = cineCam->GetScriptCameraId();
            SetFlowState(kFS_CinematicSkip);
            mFlowTime = 0.f;
            return kMR_Normal;
          }
        } else if (gpGameState->GetGameMode().GetGameModeType() != 'FRND') {
          mStateManager->DeferStateTransition(kSMT_Unk);
        }
      }
      mStateManager->SetRandomAvailable(true);
      mStateManager->ProcessInput(input);
      mStateManager->SetRandomAvailable(false);
      stopRumble = false;
    }
    mGuiManager->ProcessControllerInput(*mStateManager, input, queue);
    if (stopRumble) {
      gpController->SetMotorState(kIOP_Player1, kMS_Stop);
      gpController->SetMotorState(kIOP_Player2, kMS_Stop);
      gpController->SetMotorState(kIOP_Player3, kMS_Stop);
      gpController->SetMotorState(kIOP_Player4, kMS_Stop);
    }
    break;
  }
  case kAM_FrameEnd:
    mStateManager->FrameEnd();
    if (mStateManager->mQuitGame) {
      queue.Push(MakeMsg::CreateQuitGameplay(kAMT_Game));
    }
    break;
  case kAM_QuitGameplay:
    RecordMultiplayerResults();
    CFrameDelayedKiller::StallAndFlushAllAllocations();
    return kMR_RemoveIOWin;
  }
  return kMR_Normal;
}

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
