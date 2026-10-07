#include "MetroidPrime/CQuitGameScreen.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiTableGroup.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"

static const char* const skTitleNames[] = {"QuitGame", "Continue", "QuitMPConfirmation"};

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

void CQuitGameScreen::ProcessUserInput(const CFinalInput& input) {
  if (mLoadedFrame != nullptr) {
    mLoadedFrame->ProcessUserInput(input);
    if (input.PB() && mType != kQT_ContinueFromLastSave) {
      mAction = kQA_No;
      CSfxManager::SfxStart(0xbfc, 127, 63, CSfxManager::kAllAreas, false, false,
                            CSfxManager::kMedPriority);
    }
  }
}

void CQuitGameScreen::Draw() const {
  if (mType == kQT_QuitGame || mType == kQT_QuitMultiplayer) {
    CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen,
                                  CColor::Black().WithAlphaOf(0.7f), nullptr, 1.f);
  }

  const float cameraOffsets[] = {0.f, 0.92f, 0.f};
  if (mLoadedFrame != nullptr) {
    mLoadedFrame->Draw(CGuiWidgetDrawParms(1.f, CVector3f(0.f, 0.f, cameraOffsets[mType])));
  }
}

EQuitAction CQuitGameScreen::Update(float dt) {
  if (mLoadedFrame == nullptr && mFrame.IsLoaded()) {
    FinishedLoading();
  }
  return mAction;
}

void CQuitGameScreen::DoAdvance(CGuiTableGroup* caller) {
  if (caller->GetUserSelection() == 0) {
    CSfxManager::SfxStart(0x5e3, 127, 64, CSfxManager::kAllAreas, false, false,
                          CSfxManager::kMedPriority);
    mAction = kQA_Yes;
  } else {
    CSfxManager::SfxStart(0x5e3, 127, 64, CSfxManager::kAllAreas, false, false,
                          CSfxManager::kMedPriority);
    mAction = kQA_No;
  }
}

void CQuitGameScreen::DoSelectionChange(CGuiTableGroup*, int) {
  SetColors();
  CSfxManager::SfxStart(0x5e1, 127, 64, CSfxManager::kAllAreas, false, false,
                        CSfxManager::kMedPriority);
}

void CQuitGameScreen::FinishedLoading() {
  mLoadedFrame = mFrame.GetObject();
  mChoiceTable = static_cast< CGuiTableGroup* >(mLoadedFrame->FindWidget("tablegroup_choice"));
  mChoiceTable->SetVertical(false);
  mChoiceTable->SetMenuAdvanceCallback(
      TFunctor1FromMethod< CQuitGameScreen, CGuiTableGroup* const >::Make(
          *this, &CQuitGameScreen::DoAdvance));
  mChoiceTable->SetMenuSelectionChangeCallback(
      TFunctor2FromMethod< CQuitGameScreen, CGuiTableGroup* const, const int >::Make(
          *this, &CQuitGameScreen::DoSelectionChange));

  mTitle = static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_title"));
  mTitle->TextSupport().SetText(rstl::wstring_l(gpStringTable->GetString(skTitleNames[mType])));
  mYesChoice = static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_choice0"));
  mYesChoice->TextSupport().SetText(rstl::wstring_l(gpStringTable->GetString("Yes")));
  mNoChoice = static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_choice1"));
  mNoChoice->TextSupport().SetText(rstl::wstring_l(gpStringTable->GetString("No")));

  const int initialSelections[] = {1, 0, 1};
  if (mType == kQT_ContinueFromLastSave) {
    CGuiWidget* background = mLoadedFrame->FindWidget("basewidget_message");
    if (background != nullptr) {
      background->SetVisibility(false, kTM_Children);
    }
  }
  mChoiceTable->SetIsActive(true);
  mChoiceTable->SetUserSelection(initialSelections[mType]);
  SetColors();
}

void CQuitGameScreen::SetColors() {
  const CColor selected(uchar(200), uchar(200), uchar(200), uchar(255));
  const CColor unselected(uchar(50), uchar(50), uchar(50), uchar(255));
  const int selection = mChoiceTable->GetUserSelection();
  for (int row = 0; row < 2; ++row) {
    CGuiWidget* widget = mChoiceTable->GetWorkerWidget(row);
    widget->SetColor(row == selection ? selected : unselected);
  }
}
