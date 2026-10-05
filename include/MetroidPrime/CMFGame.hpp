#ifndef _CMFGAME
#define _CMFGAME

#include "MetroidPrime/CIOWin.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/single_ptr.hpp"

class CInGameGuiManagerSet;
class CPortalTransition;
class CStateManager;

class CMFGame : public CIOWin {
public:
  enum EFlowState {
    kFS_Zero,
    kFS_InGame,
    kFS_Paused,
    kFS_PlayerDied,
    kFS_MultiplayerEndFade,
    kFS_MultiplayerResults,
    kFS_CinematicSkip,
    kFS_PortalTransition,
    kFS_State8
  };

  CMFGame(rstl::ncrc_ptr< CStateManager > stateManager,
          const rstl::ncrc_ptr< CInGameGuiManagerSet >& guiManager,
          CArchitectureQueue& architectureQueue);

  // CIOWin
  ~CMFGame() override;
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;
  void Draw() const override;

  static void ActivateMultiplayerGui();
  void EnterMapScreen();
  void PauseGame();
  void EnterLogBook();
  void SaveGame();
  void EnterPauseScreenState5();
  void EnterMessageScreen(float time);
  void UnpauseGame();
  void PlayerDied();
  bool IsCameraActiveFlow() const;

private:
  enum ETransitionPhase {
    kTP_None = -1,
    kTP_FadeOut,
    kTP_Load,
    kTP_Play,
    kTP_FadeIn,
    kTP_Complete
  };

  void SetFlowState(EFlowState state);
  void RecordMultiplayerResults() const;
  void FinishMultiplayerGame(CArchitectureQueue& queue);
  void EndGame(CArchitectureQueue& queue);
  void DrawWorld(bool singleViewport) const;
  void DrawGui(bool singleViewport) const;
  void DrawTransitionFilter() const;

  static bool mMultiplayerGuiActive;

  rstl::ncrc_ptr< CStateManager > mStateManager;
  rstl::ncrc_ptr< CInGameGuiManagerSet > mGuiManager;
  EFlowState mFlowState;
  float mFlowTime;
  float mMultiplayerEndFadeTime;
  TUniqueId mSkippedCineCam;
  rstl::single_ptr< CPortalTransition > mPortalTransition;
  float mPortalTransitionTime;
  ETransitionPhase mTransitionPhase;
  float mTransitionFadeTime;
  bool mInitialized : 1;
  bool mPlayerAlive : 1;
  mutable bool mEndGameFrameCaptured : 1;
  bool mTransitionFromDarkWorld : 1;
  bool x44_4 : 1;
};
CHECK_SIZEOF(CMFGame, 0x48)

#endif // _CMFGAME
