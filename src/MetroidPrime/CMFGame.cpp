#include "MetroidPrime/CMFGame.hpp"

#include "MetroidPrime/CInGameGuiManagerSet.hpp"
#include "MetroidPrime/CPortalTransition.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CGameState.hpp"

bool CMFGame::mMultiplayerGuiActive;

CMFGame::CMFGame(rstl::ncrc_ptr< CStateManager > stateManager,
                 const rstl::ncrc_ptr< CInGameGuiManagerSet >& guiManager,
                 CArchitectureQueue& architectureQueue)
: CIOWin(rstl::string_l("CMFGame")), mSkippedCineCam(kInvalidUniqueId) {}

CMFGame::~CMFGame() {}

void CMFGame::ActivateMultiplayerGui() {}

void CMFGame::RecordMultiplayerResults() const {}

CGameState::SPreviousGameResults::SPreviousGameResults(
    uint gameMode, bool showResults, int modeResult, int playerCount,
    const rstl::reserved_vector< SPlayerResult, 4 >& players) {}

void CMFGame::FinishMultiplayerGame() {}

void CMFGame::EndGame(CArchitectureQueue& queue) {}

bool CMFGame::IsCameraActiveFlow() const {}

void CMFGame::PlayerDied() {}

void CMFGame::UnpauseGame() {}

void CMFGame::EnterMessageScreen(float time) {}

void CMFGame::EnterPauseScreenState5() {}

void CMFGame::SaveGame() {}

void CMFGame::EnterLogBook() {}

void CMFGame::PauseGame() {}

void CMFGame::EnterMapScreen() {}

void CMFGame::Draw() const {}

void CMFGame::DrawTransitionFilter() const {}

void CMFGame::DrawGui(bool singleViewport) const {}

void CMFGame::DrawWorld(bool singleViewport) const {}

CIOWin::EMessageReturn CMFGame::OnMessage(const CArchitectureMessage& message,
                                          CArchitectureQueue& queue) {}

void CMFGame::SetFlowState(EFlowState state) {}
