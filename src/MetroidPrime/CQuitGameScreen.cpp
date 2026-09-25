#include "MetroidPrime/CQuitGameScreen.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "Kyoto/CSimplePool.hpp"

// Structure-first scaffold; GUI callbacks and rendering are not implemented yet.

static const char* const skFrameNames[] = {"FRME_QuitScreen1", "FRME_QuitScreen2",
                                           "FRME_QuitScreen4"};

CQuitGameScreen::CQuitGameScreen(EQuitType type, int layout)
: mType(type)
, mFrame(gpSimplePool->GetObj(skFrameNames[layout]))
, mLoadedFrame(nullptr)
, mChoiceTable(nullptr)
, mAction(kQA_None)
, mTitle(nullptr)
, mYesChoice(nullptr)
, mNoChoice(nullptr) {
  mFrame.Lock();
}

CQuitGameScreen::~CQuitGameScreen() {}

void CQuitGameScreen::ProcessUserInput(const CFinalInput&) {
  // TODO: forward input to the loaded frame, handle cancellation and play its sound.
}

void CQuitGameScreen::Draw() const {
  // TODO: draw the type-specific backdrop and frame through shared GUI draw parameters.
}

EQuitAction CQuitGameScreen::Update() {
  if (mLoadedFrame == nullptr && mFrame.IsLoaded()) {
    FinishedLoading();
  }
  return mAction;
}

void CQuitGameScreen::DoAdvance(CGuiTableGroup*) {
  // TODO: translate the selected row into Yes/No and play the advance sound.
}

void CQuitGameScreen::DoSelectionChange(CGuiTableGroup*, int) {
  SetColors();
  // TODO: play the selection-change sound.
}

void CQuitGameScreen::FinishedLoading() {
  mLoadedFrame = mFrame.GetObject();
  // TODO: bind the table/text panes, localize the choices, install callbacks and set defaults.
}

void CQuitGameScreen::SetColors() {
  // TODO: set the selected row to light gray and the other row to dark gray.
}
