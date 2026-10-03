#ifndef _CINGAMEQUITSCREEN
#define _CINGAMEQUITSCREEN

#include "GuiSys/CRepeatState.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CQuitGameScreen.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CGuiFrame;
class CGuiTableGroup;
class CGuiTextPane;
class CGuiWidget;
class CFinalInput;
class CStateManager;

// Guessed names, supported by the native QuitMPGame/MusicChoice widget bindings.
class CInGameQuitScreen {
public:
  explicit CInGameQuitScreen(int viewportLayout);
  ~CInGameQuitScreen();

  EQuitAction Update(float dt, CStateManager& mgr);
  void ProcessUserInput(const CFinalInput& input);
  void Draw() const;

  // Guessed names for the native frame binding and selection handlers.
  void FinishedLoading();
  void DoSelectionChange(CGuiTableGroup* caller, int oldSelection);
  void SetColors();

private:
  // Guessed name; two rows contain quit and unlocked music choices.
  struct SChoice {
    SChoice(CGuiWidget* leftArrow, CGuiWidget* rightArrow, CGuiTextPane* textPane)
    : mLeftArrow(leftArrow), mRightArrow(rightArrow), mTextPane(textPane), mSelection(0) {}

    CGuiWidget* mLeftArrow;
    CGuiWidget* mRightArrow;
    CGuiTextPane* mTextPane;
    int mSelection;
    rstl::vector< rstl::wstring > mOptions;
  };

  void UpdateChoiceText();
  void ChangeChoice(int direction);
  void ApplyMusicSelection(CStateManager& mgr);

  TCachedToken< CGuiFrame > mFrame;
  CGuiFrame* mReadyFrame;
  int mViewportLayout;
  CGuiTableGroup* mChoiceTable;
  CGuiTextPane* mQuitChoiceText;
  CGuiTextPane* mMusicChoiceText;
  EQuitAction mAction;
  CRepeatState mLeftRepeat;
  CRepeatState mRightRepeat;
  int mPreviousMusicSelection;
  rstl::auto_ptr< CQuitGameScreen > mQuitConfirmation;
  rstl::reserved_vector< SChoice, 2 > mChoices;
};
CHECK_SIZEOF(CInGameQuitScreen, 0x7c)

#endif // _CINGAMEQUITSCREEN
