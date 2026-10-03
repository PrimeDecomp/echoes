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

// Guessed names, supported by the native QuitMPGame/MusicChoice widget bindings.
class CInGameQuitScreen {
public:
  explicit CInGameQuitScreen(int viewportLayout);
  ~CInGameQuitScreen();

private:
  struct SChoice {
    CGuiWidget* mLeftArrow;
    CGuiWidget* mRightArrow;
    CGuiTextPane* mTextPane;
    int mSelection;
    rstl::vector< rstl::wstring > mOptions;
  };

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
