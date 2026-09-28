#include "MetroidPrime/CMainFlow.hpp"

#include "MetroidPrime/CArchMsgParmInt32.hpp"
#include "MetroidPrime/CArchitectureMessage.hpp"
#include "MetroidPrime/CArchitectureQueue.hpp"
#include "MetroidPrime/CAutoSave.hpp"
#include "MetroidPrime/CCredits.hpp"
#include "MetroidPrime/CMFGameLoader.hpp"
#include "MetroidPrime/CPlayMovie.hpp"
#include "MetroidPrime/CPreFrontEnd.hpp"
#include "MetroidPrime/Decode.hpp"
#include "MetroidPrime/Player/CGMSinglePlayer.hpp"
#include "MetroidPrime/Player/CGameMode.hpp"
#include "MetroidPrime/Player/CGameState.hpp"

#include "MetroidPrime/CMain.hpp"

#include "Kyoto/Alloc/CMemory.hpp"

CMainFlow::CMainFlow() : CIOWin(rstl::string_l("MainFlow")), mGameState(kCFS_Unspecified) {}

CIOWin::EMessageReturn CMainFlow::OnMessage(const CArchitectureMessage& msg,
                                            CArchitectureQueue& queue) {
  switch (msg.GetType()) {
  case kAM_TimerTick:
    AdvanceGameState(queue);
    break;
  case kAM_SetGameState:
    CArchMsgParmInt32 state = MakeMsg::GetParmNewGameflowState(msg);
    SetGameState(static_cast< EClientFlowStates >(state.GetInt32()), queue);
    return CIOWin::kMR_Exit;
  }

  return CIOWin::kMR_Normal;
}

bool CMainFlow::GetIsContinueDraw() const { return false; }

void CMainFlow::Draw() const {}

void CMainFlow::AdvanceGameState(CArchitectureQueue& queue) {
  switch (mGameState) {
  case kCFS_Game:
    SetGameState(kCFS_GameExit, queue);
    break;
  case kCFS_PreFrontEnd:
    SetGameState(kCFS_FrontEnd, queue);
    break;
  case kCFS_FrontEnd:
    SetGameState(kCFS_Game, queue);
    break;
  case kCFS_GameExit: {
    if (gpMain->GetRestartMode() != CMain::kRM_None &&
        gpMain->GetRestartMode() != CMain::kRM_StateSetter) {
      if (gpGameState->GetGameModeType() == 'SNGL') {
        gpMain->SetX30(true);
      } else {
        gpMain->ResetGameState();
      }
    }
    // Fall through.
  }
  case kCFS_Unspecified:
    SetGameState(kCFS_PreFrontEnd, queue);
    break;
  default:
    break;
  }
}

static inline bool IsEndGameMode(CMain::ERestartMode m) {
  return m >= CMain::kRM_Credits1 && m <= CMain::kRM_EndMovie2;
}

void CMainFlow::SetGameState(EClientFlowStates state, CArchitectureQueue& queue) {
  mGameState = state;

  switch (mGameState) {
  case kCFS_GameExit: {
    CMain::ERestartMode m = gpMain->GetRestartMode();
    if (IsEndGameMode(m)) {
      CIOWin* ioWin = nullptr;
      switch (m) {
      case CMain::kRM_EndMovie2:
        ioWin = rs_new CPlayMovie(6);
        break;
      case CMain::kRM_EndAutoSave:
        ioWin = rs_new CAutoSave();
        break;
      case CMain::kRM_EndMovie1:
        ioWin = rs_new CPlayMovie(4);
        break;
      default:
        ioWin = rs_new CCredits();
        break;
      }
      queue.Push(MakeMsg::CreateCreateIOWin(kAMT_IOWinManager, kFrontEndUIMsgPriority,
                                            kFrontEndUIDrawPriority, ioWin));
    }
    break;
  }
  case kCFS_PreFrontEnd: {
    if (gpMain->GetRestartMode() == CMain::kRM_None) {
      break;
    }

    CIOWin* preFrontEnd = rs_new CPreFrontEnd();
    queue.Push(MakeMsg::CreateCreateIOWin(kAMT_IOWinManager, kFrontEndUIMsgPriority,
                                          kFrontEndUIDrawPriority, preFrontEnd));
    break;
  }
  case kCFS_FrontEnd: {
    const CMain::ERestartMode mode = gpMain->GetRestartMode();
    if (gpGameState->GetGameMode().GetGameModeType() == 'FRND') {
      StartGameFromFrontEnd();
    } else if (mode != CMain::kRM_None) {
      if (gpMain->GetRestartMode() == CMain::kRM_StateSetter) {
        gpMain->SetRestartMode(CMain::kRM_Default);
        gpMain->StreamNewGameState(false);
        gpGameState->SetGameMode(rs_new CGMSinglePlayer());
        gpGameState->HintOptions().EnsureHintNextTime();
      } else {
        fn_80143E88();
      }
    }
    break;
  }
  case kCFS_Game: {
    gpGameState->GameOptions().EnsureOptions();
    CIOWin* const gameFlow = rs_new CMFGameLoader();
    gpMain->SetRestartMode(CMain::kRM_Default);
    queue.Push(MakeMsg::CreateCreateIOWin(kAMT_IOWinManager, kMFGameMsgPriority,
                                          kMFGameDrawPriority, gameFlow));
    break;
  }
  }
}

CMainFlow::~CMainFlow() {}
