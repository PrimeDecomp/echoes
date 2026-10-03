#ifndef _CMFGAME
#define _CMFGAME

#include "MetroidPrime/CIOWin.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/single_ptr.hpp"

class CInGameGuiManagerSet;
class CPortalTransition;
class CStateManager;

// Names reconstructed from G2ME01 behavior and Prime correspondence, except the
// Wii-exported mMultiplayerGuiActive. See research/CMFGame-G2ME01.md in the workspace docs.
class CMFGame : public CIOWin {
public:
  // Guessed names; Echoes adds multiplayer and portal states to Prime's flow.
  enum EFlowState {
    kFS_Zero,
    kFS_InGame,
    kFS_Paused,
    kFS_PlayerDied,
    kFS_MultiplayerEndFade,
    kFS_MultiplayerResults,
    kFS_CinematicSkip,
    kFS_PortalTransition,
    kFS_State8 // Area update and timed screen fade; exact purpose unresolved.
  };

  CMFGame(rstl::ncrc_ptr< CStateManager > stateManager,
          const rstl::ncrc_ptr< CInGameGuiManagerSet >& guiManager,
          CArchitectureQueue& architectureQueue);

  // CIOWin
  ~CMFGame() override;
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;
  void Draw() const override;

  static void ActivateMultiplayerGui(); // Guessed name; sets the exported static flag.
  void EnterMapScreen();
  void PauseGame();
  void EnterLogBook();
  void SaveGame();
  void EnterPauseScreenState5(); // Deferred-transition 5; screen identity unresolved.
  void EnterMessageScreen(float time);
  void UnpauseGame();
  void PlayerDied();
  bool IsCameraActiveFlow() const;

private:
  // Guessed names for the observed -1,0..4 transition sequence.
  enum ETransitionPhase {
    kTP_None = -1,
    kTP_FadeOut,
    kTP_Load,
    kTP_Play,
    kTP_FadeIn,
    kTP_Complete
  };

  void SetFlowState(EFlowState state);
  void RecordMultiplayerResults() const;     // Guessed name.
  void FinishMultiplayerGame();              // Guessed name.
  void EndGame(CArchitectureQueue& queue);   // Guessed name.
  void DrawWorld(bool singleViewport) const; // Guessed names for the two render passes.
  void DrawGui(bool singleViewport) const;
  void DrawTransitionFilter() const; // Guessed name.

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
  bool mEndGameFrameCaptured : 1;
  bool mTransitionFromDarkWorld : 1;
  bool x44_4 : 1; // Mask 0x08 in IsCameraActiveFlow; writer/meaning unresolved.
};
CHECK_SIZEOF(CMFGame, 0x48)

#endif // _CMFGAME
