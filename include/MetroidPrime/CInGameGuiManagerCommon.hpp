#ifndef _CINGAMEGUIMANAGERCOMMON
#define _CINGAMEGUIMANAGERCOMMON

// Names adapted from Prime; the final quit state is an Echoes addition.
enum EInGameGuiState {
  kIGGS_Zero,
  kIGGS_InGame,
  kIGGS_MapScreen,
  kIGGS_PauseGame,
  kIGGS_PauseLogBook,
  kIGGS_PauseSaveGame,
  kIGGS_PauseHUDMessage,
  kIGGS_QuitGame // Guessed name
};

namespace InGameGuiStates {
inline bool IsGameplayState(EInGameGuiState state) {
  return state >= kIGGS_Zero && state <= kIGGS_InGame;
}
} // namespace InGameGuiStates

#endif // _CINGAMEGUIMANAGERCOMMON
