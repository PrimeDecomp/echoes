#include "MetroidPrime/CInGameQuitScreen.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiTableGroup.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/Player/CFrontEndGameMode.hpp"
#include "MetroidPrime/Player/CGMMultiplayer.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpecialFunction.hpp"
#include "MetroidPrime/ScriptObjects/CScriptStreamedMusic.hpp"
#include "MetroidPrime/TCastTo.hpp"

static const char* const skFrameNames[] = {"FRME_MultiplayerPause4", "FRME_MultiplayerPause2",
                                           "FRME_MultiplayerPause4"};

CInGameQuitScreen::CInGameQuitScreen(int viewportLayout)
: mFrame(gpSimplePool->GetObj(skFrameNames[viewportLayout]))
, mReadyFrame(nullptr)
, mViewportLayout(viewportLayout)
, mChoiceTable(nullptr)
, mQuitChoiceText(nullptr)
, mMusicChoiceText(nullptr)
, mAction(kQA_None)
, mPreviousMusicSelection(0) {
  mFrame.Lock();
  CSfxManager::SetChannel(CSfxManager::kSC_PauseScreen);
}

void CInGameQuitScreen::ProcessUserInput(const CFinalInput& input) {
  if (!mQuitConfirmation.null()) {
    mQuitConfirmation->ProcessUserInput(input);
    return;
  }
  if (mReadyFrame == nullptr) {
    return;
  }

  mReadyFrame->ProcessUserInput(input);
  if (input.PA() || input.PStart()) {
    if (mChoices[0].mSelection == 1 && mChoiceTable->GetUserSelection() == 0) {
      mQuitConfirmation = rs_new CQuitGameScreen(kQT_QuitMultiplayer, mViewportLayout);
    } else {
      CSfxManager::SfxStart(0x5e1, 127, 64);
      mAction = kQA_No;
    }
  } else if (input.PB()) {
    CSfxManager::SfxStart(0x5e1, 127, 64);
    mAction = kQA_No;
  } else {
    const bool left = input.DLALeft() || input.DDPLeft();
    const bool right = input.DLARight() || input.DDPRight();
    if (mLeftRepeat.Update(input.DeltaTime(), left) && left) {
      ChangeChoice(-1);
    } else if (!left && mRightRepeat.Update(input.DeltaTime(), right) && right) {
      ChangeChoice(1);
    }
  }
}

EQuitAction CInGameQuitScreen::Update(float dt, CStateManager& mgr) {
  if (mReadyFrame == nullptr) {
    if (mFrame.IsLoaded()) {
      FinishedLoading();
    }
  } else if (!mQuitConfirmation.null()) {
    const EQuitAction action = mQuitConfirmation->Update(dt);
    if (action == kQA_No) {
      mQuitConfirmation = rstl::auto_ptr< CQuitGameScreen >();
    } else if (action == kQA_Yes) {
      mAction = kQA_Yes;
      mQuitConfirmation = rstl::auto_ptr< CQuitGameScreen >();
    }
  } else {
    mReadyFrame->Update(dt);
    if (mPreviousMusicSelection != mChoices[1].mSelection) {
      mPreviousMusicSelection = mChoices[1].mSelection;
      ApplyMusicSelection(mgr);
    }
  }

  if (mAction != kQA_None) {
    CSfxManager::SetChannel(CSfxManager::kSC_Game);
  }
  return mAction;
}

void CInGameQuitScreen::Draw() const {
  CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen,
                                CColor::Black().WithAlphaOf(0.7f), nullptr, 1.f);
  if (mReadyFrame != nullptr) {
    mReadyFrame->Draw(CGuiWidgetDrawParms::Default());
  }
  if (!mQuitConfirmation.null()) {
    mQuitConfirmation->Draw();
  }
}

void CInGameQuitScreen::DoSelectionChange(CGuiTableGroup*, int) {
  SetColors();
  CSfxManager::SfxStart(0x5e1, 127, 64);
}

void CInGameQuitScreen::FinishedLoading() {
  mReadyFrame = mFrame.GetObject();
  mChoiceTable = static_cast< CGuiTableGroup* >(mReadyFrame->FindWidget("tablegroup_choice"));
  mChoiceTable->SetVertical(true);
  mChoiceTable->SetMenuSelectionChangeCallback(
      TFunctor2FromMethod< CInGameQuitScreen, CGuiTableGroup* const, const int >::Make(
          *this, &CInGameQuitScreen::DoSelectionChange));
  mChoiceTable->SetIsActive(true);
  mChoiceTable->SetUserSelection(0);

  CGuiTextPane* quitTitle =
      static_cast< CGuiTextPane* >(mReadyFrame->FindWidget("textpane_quit_title"));
  quitTitle->TextSupport().SetText(rstl::wstring_l(gpStringTable->GetString("QuitMPGame")), false);
  CGuiTextPane* musicTitle =
      static_cast< CGuiTextPane* >(mReadyFrame->FindWidget("textpane_song_title"));
  musicTitle->TextSupport().SetText(rstl::wstring_l(gpStringTable->GetString("MusicChoice")),
                                    false);

  mQuitChoiceText = static_cast< CGuiTextPane* >(mReadyFrame->FindWidget("textpane_quit_choice"));
  SChoice quitChoice(mReadyFrame->FindWidget("model_quit_left"),
                     mReadyFrame->FindWidget("model_quit_right"), mQuitChoiceText);
  quitChoice.mOptions.reserve(2);
  quitChoice.mOptions.push_back(rstl::wstring(gpStringTable->GetString("No")));
  quitChoice.mOptions.push_back(rstl::wstring(gpStringTable->GetString("Yes")));
  mChoices.push_back(quitChoice);

  mMusicChoiceText = static_cast< CGuiTextPane* >(mReadyFrame->FindWidget("textpane_song_choice"));
  SChoice musicChoice(mReadyFrame->FindWidget("model_song_left"),
                      mReadyFrame->FindWidget("model_song_right"), mMusicChoiceText);
  musicChoice.mOptions.reserve(7);

  CGameMode& mode = gpGameState->GetGameMode();
  int currentTrack = 0;
  if (mode.GetGameModeType() == CFrontEndGameMode::kSGM_DeathMatch ||
      mode.GetGameModeType() == CFrontEndGameMode::kSGM_Coin) {
    currentTrack = static_cast< CGMMultiplayer& >(mode).GetMusicIndex();
  }
  for (int track = 0; track < 7; ++track) {
    bool unlocked = track == 0;
    const CEnvironmentVariable* variable = gpGameState->SystemOptions().FindEnvironmentVariable(
        CBasics::Stringize("UnlockMusic%d", track));
    if (variable != nullptr) {
      unlocked = variable->GetMaximum() == variable->GetValue();
    }
    if (unlocked) {
      if (track == currentTrack) {
        musicChoice.mSelection = musicChoice.mOptions.size();
        mPreviousMusicSelection = musicChoice.mOptions.size();
      }
      musicChoice.mOptions.push_back(
          rstl::wstring(gpStringTable->GetString(CBasics::Stringize("MusicSelection%d", track))));
    }
  }
  mChoices.push_back(musicChoice);

  SetColors();
  UpdateChoiceText();
}

void CInGameQuitScreen::SetColors() {
  const CColor selected(0xc8c8c8ff);
  const CColor unselected(0x323232ff);
  for (int i = 0; i < 2; ++i) {
    mChoiceTable->GetWorkerWidget(i)->SetColor(i == mChoiceTable->GetUserSelection() ? selected
                                                                                     : unselected);
  }
}

void CInGameQuitScreen::UpdateChoiceText() {
  const CColor enabled(0xffffffff);
  const CColor disabled(0x323232ff);
  for (int i = 0; i < mChoices.size(); ++i) {
    SChoice& choice = mChoices[i];
    choice.mLeftArrow->SetColor(choice.mSelection == 0 ? disabled : enabled);
    choice.mRightArrow->SetColor(choice.mSelection == choice.mOptions.size() - 1 ? disabled
                                                                                 : enabled);
    choice.mTextPane->TextSupport().SetText(choice.mOptions[choice.mSelection], false);
  }
}

void CInGameQuitScreen::ChangeChoice(int direction) {
  SChoice& choice = mChoices[mChoiceTable->GetUserSelection()];
  const int previous = choice.mSelection;
  choice.mSelection = CMath::Clamp(0, previous + direction, choice.mOptions.size() - 1);
  UpdateChoiceText();
  if (previous != choice.mSelection) {
    CSfxManager::SfxStart(0x5e3, 127, 64);
  }
}

void CInGameQuitScreen::ApplyMusicSelection(CStateManager& mgr) {
  int selectedTrack = 0;
  int unlockedIndex = 0;
  for (int track = 0; track < 7; ++track) {
    bool unlocked = track == 0;
    const CEnvironmentVariable* variable = gpGameState->SystemOptions().FindEnvironmentVariable(
        CBasics::Stringize("UnlockMusic%d", track));
    if (variable != nullptr) {
      unlocked = variable->GetMaximum() == variable->GetValue();
    }
    if (unlocked) {
      if (unlockedIndex == mPreviousMusicSelection) {
        selectedTrack = track;
      }
      ++unlockedIndex;
    }
  }

  CGameMode& mode = gpGameState->GetGameMode();
  if (mode.GetGameModeType() == CFrontEndGameMode::kSGM_DeathMatch ||
      mode.GetGameModeType() == CFrontEndGameMode::kSGM_Coin) {
    static_cast< CGMMultiplayer& >(mode).SetMusicIndex(selectedTrack);
  }

  CObjectList& actors = mgr.ObjectListById(kOL_Actor);
  for (int index = actors.GetFirstObjectIndex(); index != -1;
       index = actors.GetNextObjectIndex(index)) {
    const CScriptSpecialFunction* function = TCastToPtr< CScriptSpecialFunction >(actors[index]);
    if (function == nullptr ||
        function->GetFunction() != CScriptSpecialFunction::kSF_MultiplayerMusic) {
      continue;
    }

    const rstl::vector< SConnection >& connections = function->GetConnectionList();
    if (selectedTrack < 0 || selectedTrack >= connections.size()) {
      continue;
    }
    const SConnection& connection = connections[selectedTrack];
    if (connection.state != kSS_Zero) {
      continue;
    }

    const CStateManager::TIdListResult targets = mgr.GetIdListForScript(connection.objId);
    for (CStateManager::TIdList::const_iterator it = targets.first; it != targets.second; ++it) {
      CScriptStreamedMusic* music =
          TCastToPtr< CScriptStreamedMusic >(const_cast< CEntity* >(mgr.GetObjectById(it->second)));
      if (music != nullptr && music->GetActive()) {
        music->PlayAudio();
        break;
      }
    }
  }
}
