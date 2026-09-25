#ifndef _CQUITGAMESCREEN
#define _CQUITGAMESCREEN

#include "Kyoto/TToken.hpp"

class CGuiFrame;
class CGuiTableGroup;
class CGuiTextPane;
class CFinalInput;

// Guessed names, based on the selected title strings.
enum EQuitType { kQT_QuitGame, kQT_ContinueFromLastSave, kQT_QuitMultiplayer };
enum EQuitAction { kQA_None, kQA_Yes, kQA_No };

class CQuitGameScreen {
public:
  CQuitGameScreen(EQuitType type, int layout);
  ~CQuitGameScreen();
  void SetColors();
  void FinishedLoading();
  void DoSelectionChange(CGuiTableGroup* caller, int oldSelection);
  void DoAdvance(CGuiTableGroup* caller);
  EQuitAction Update();
  void Draw() const;
  void ProcessUserInput(const CFinalInput& input);

private:
  EQuitType mType;
  TCachedToken< CGuiFrame > mFrame;
  CGuiFrame* mLoadedFrame;
  CGuiTableGroup* mChoiceTable;
  EQuitAction mAction;
  CGuiTextPane* mTitle;
  CGuiTextPane* mYesChoice;
  CGuiTextPane* mNoChoice;
};
CHECK_SIZEOF(CQuitGameScreen, 0x28)

#endif // _CQUITGAMESCREEN
