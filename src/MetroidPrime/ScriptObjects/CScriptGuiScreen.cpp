#include "MetroidPrime/ScriptObjects/CScriptGuiFrontEndScreen.hpp"

#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Input/IController.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Text/CGuiTextSupport.hpp"
#include "MetroidPrime/CArchitectureQueue.hpp"
#include "MetroidPrime/CGameGlobalObjects.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CMemoryCard.hpp"
#include "MetroidPrime/CSaveGameScreen.hpp"
#include "MetroidPrime/CSaveWorldMemory.hpp"
#include "MetroidPrime/CSlideShow.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Decode.hpp"
#include "MetroidPrime/Player/CEnvironmentVariable.hpp"
#include "MetroidPrime/Player/CFrontEndGameMode.hpp"
#include "MetroidPrime/Player/CGameOptions.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CWorldState.hpp"
#include "MetroidPrime/ScriptLoader/SLdrGuiScreen.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGuiMenu.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGuiSlider.hpp"
#include "MetroidPrime/ScriptObjects/CScriptRelay.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSound.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSwitch.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTextPane.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWorldTeleporter.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"

#include "rstl/StringExtras.hpp"

#include <math.h>
#include <stdio.h>

// Guessed names; IOWin priorities passed by reference to the slide show message.
static int sSlideShowMsgPriority = 16;
static int sSlideShowDrawPriority = 1001;

// Unresolved native cast for entity type 155.
extern "C" CEntity* fn_800977A4(CEntity* entity);

static inline float RoundToNearest(float value) {
  const float lower = floor(value);
  const float upper = CMath::CeilingF(value);
  return value - lower < upper - value ? lower : upper;
}

CScriptGuiFrontEndScreen::CScriptGuiFrontEndScreen(TUniqueId uid, const rstl::string& name,
                                                   const CEntityInfo& info, CAssetId stringTable)
: CScriptGuiScreen(uid, name, info)
, mStringTable(gpSimplePool->GetObj(SObjectTag('STRG', stringTable)))
, mDefaultTeleporter(nullptr)
, mDeathMatchSwitch(nullptr)
, mDeathMatchEntity(nullptr)
, mCoinSwitch(nullptr)
, mCoinEntity(nullptr)
, mBrightnessSlider(nullptr)
, mStretchSlider(nullptr)
, mPositionXSlider(nullptr)
, mPositionYSlider(nullptr)
, mHudAlphaSlider(nullptr)
, mHelmetAlphaSlider(nullptr)
, mHintSystemMenu(nullptr)
, mHudLagMenu(nullptr)
, mInvertYMenu(nullptr)
, mRumbleMenu(nullptr)
, mSfxVolumeSlider(nullptr)
, mMusicVolumeSlider(nullptr)
, mSurroundMenu(nullptr)
, mFragLimitMenu(nullptr)
, mDeathMatchTimeMenu(nullptr)
, mCoinTimeMenu(nullptr)
, mCoinLimitMenu(nullptr)
, mDeathMatchMusicMenu(nullptr)
, mCoinMusicMenu(nullptr)
, mCopySwitch(nullptr)
, mEraseSwitch(nullptr)
, mLoadSwitch(nullptr)
, mStartSwitch(nullptr)
, mGalleryEntity(nullptr)
, mSelectedSlot(0)
, x32c_(0)
, mOptionsPage(-1)
, mSavedBrightness(0x80000000)
, mSavedStretch(0x80000000)
, mSavedPositionX(0x80000000)
, mSavedPositionY(0x80000000)
, mSavedHudAlpha(0x80000000)
, mSavedHelmetAlpha(0x80000000)
, mSavedHintSystem(0x80000000)
, mSavedHudLag(0x80000000)
, mSavedInvertY(0x80000000)
, mSavedRumble(0x80000000)
, mSavedSfxVolume(0x80000000)
, mSavedMusicVolume(0x80000000)
, mSavedSurroundMode(0x80000000)
, mCardDriverReset(false)
, mSaveScreenBusy(false)
, mSaveScreenFailed(false)
, mGameStarted(false)
, mOptionsDirty(false)
, mMultipleControllers(false) {
  CGameOptions::SetFrontEndActive(true);
}

CScriptGuiFrontEndScreen::~CScriptGuiFrontEndScreen() { CGameOptions::SetFrontEndActive(false); }

void CScriptGuiFrontEndScreen::CollectWidgets(CStateManager& mgr) {
  TUniqueId defaultId = FindConnectedObject(mgr, kSS_XDamage, kSM_None);
  mDefaultTeleporter = TCastToPtr< CScriptWorldTeleporter >(mgr.ObjectById(defaultId));

  {
    rstl::vector< TUniqueId > ids = FindConnectedObjects(mgr, kSS_Dead, kSM_None);
    mTeleporters.reserve(ids.size());
    for (int i = 0; i < ids.size(); ++i) {
      mTeleporters.push_back_unsafe(TCastToPtr< CScriptWorldTeleporter >(mgr.ObjectById(ids[i])));
    }
  }

  {
    rstl::vector< TUniqueId > ids = FindConnectedObjects(mgr, kSS_AIS1, kSM_None);
    mDeathMatchSwitch = TCastToPtr< CScriptSwitch >(mgr.ObjectById(ids[0]));
    mDeathMatchEntity = fn_800977A4(mgr.ObjectById(ids[1]));
  }

  {
    rstl::vector< TUniqueId > ids = FindConnectedObjects(mgr, kSS_AIS2, kSM_None);
    mCoinSwitch = TCastToPtr< CScriptSwitch >(mgr.ObjectById(ids[0]));
    mCoinEntity = fn_800977A4(mgr.ObjectById(ids[1]));
  }

  {
    rstl::vector< TUniqueId > joinIds = FindConnectedObjects(mgr, kSS_Play, kSM_None);
    rstl::vector< TUniqueId > rumbleIds = FindConnectedObjects(mgr, kSS_DeathRattle, kSM_None);
    rstl::vector< TUniqueId > invertIds = FindConnectedObjects(mgr, kSS_Opened, kSM_None);
    for (int i = 0; i < 4; ++i) {
      SPlayerSetup setup;
      setup.mJoinedSwitch = TCastToPtr< CScriptSwitch >(mgr.ObjectById(joinIds[i * 2]));
      setup.mReadySwitch = TCastToPtr< CScriptSwitch >(mgr.ObjectById(joinIds[i * 2 + 1]));
      setup.mRumbleMenu = TCastToPtr< CScriptGuiMenu >(mgr.ObjectById(rumbleIds[i]));
      setup.mInvertMenu = TCastToPtr< CScriptGuiMenu >(mgr.ObjectById(invertIds[i]));
      mPlayerSetups.push_back(setup);
    }
  }

  {
    rstl::vector< TUniqueId > textIds = FindConnectedObjects(mgr, kSS_Modify, kSM_None);
    rstl::vector< TUniqueId > slotIds = FindConnectedObjects(mgr, kSS_Closed, kSM_None);
    rstl::vector< TUniqueId > usedIds = FindConnectedObjects(mgr, kSS_APRC, kSM_None);
    rstl::vector< TUniqueId > newIds = FindConnectedObjects(mgr, kSS_ScrewAttackDamage, kSM_None);
    rstl::vector< TUniqueId > difficultyIds = FindConnectedObjects(mgr, kSS_PhazonDamage, kSM_None);
    for (int i = 0; i < 3; ++i) {
      SSaveSlot slot;
      slot.mTitle = TCastToPtr< CScriptTextPane >(mgr.ObjectById(textIds[i * 3]));
      slot.mWorldName = TCastToPtr< CScriptTextPane >(mgr.ObjectById(textIds[i * 3 + 1]));
      slot.mPlayTime = TCastToPtr< CScriptTextPane >(mgr.ObjectById(textIds[i * 3 + 2]));
      slot.mSlotEntity = fn_800977A4(mgr.ObjectById(slotIds[i]));
      slot.mUsedSwitch = TCastToPtr< CScriptSwitch >(mgr.ObjectById(usedIds[i]));
      slot.mNewGameSwitch = TCastToPtr< CScriptSwitch >(mgr.ObjectById(newIds[i]));
      slot.mDifficultyMenu = TCastToPtr< CScriptGuiMenu >(mgr.ObjectById(difficultyIds[i]));
      mSaveSlots.push_back(slot);
    }
  }

  TUniqueId id = FindConnectedObject(mgr, kSS_InternalState00, kSM_None);
  mBrightnessSlider = TCastToPtr< CScriptGuiSlider >(mgr.ObjectById(id));
  id = FindConnectedObject(mgr, kSS_InternalState01, kSM_None);
  mStretchSlider = TCastToPtr< CScriptGuiSlider >(mgr.ObjectById(id));
  id = FindConnectedObject(mgr, kSS_InternalState02, kSM_None);
  mPositionXSlider = TCastToPtr< CScriptGuiSlider >(mgr.ObjectById(id));
  id = FindConnectedObject(mgr, kSS_InternalState03, kSM_None);
  mPositionYSlider = TCastToPtr< CScriptGuiSlider >(mgr.ObjectById(id));
  id = FindConnectedObject(mgr, kSS_InternalState04, kSM_None);
  mHudAlphaSlider = TCastToPtr< CScriptGuiSlider >(mgr.ObjectById(id));
  id = FindConnectedObject(mgr, kSS_InternalState05, kSM_None);
  mHelmetAlphaSlider = TCastToPtr< CScriptGuiSlider >(mgr.ObjectById(id));
  id = FindConnectedObject(mgr, kSS_InternalState06, kSM_None);
  mHintSystemMenu = TCastToPtr< CScriptGuiMenu >(mgr.ObjectById(id));
  id = FindConnectedObject(mgr, kSS_InternalState07, kSM_None);
  mHudLagMenu = TCastToPtr< CScriptGuiMenu >(mgr.ObjectById(id));
  id = FindConnectedObject(mgr, kSS_InternalState08, kSM_None);
  mInvertYMenu = TCastToPtr< CScriptGuiMenu >(mgr.ObjectById(id));
  id = FindConnectedObject(mgr, kSS_InternalState09, kSM_None);
  mRumbleMenu = TCastToPtr< CScriptGuiMenu >(mgr.ObjectById(id));
  id = FindConnectedObject(mgr, kSS_InternalState10, kSM_None);
  mSfxVolumeSlider = TCastToPtr< CScriptGuiSlider >(mgr.ObjectById(id));
  id = FindConnectedObject(mgr, kSS_InternalState11, kSM_None);
  mMusicVolumeSlider = TCastToPtr< CScriptGuiSlider >(mgr.ObjectById(id));
  id = FindConnectedObject(mgr, kSS_InternalState12, kSM_None);
  mSurroundMenu = TCastToPtr< CScriptGuiMenu >(mgr.ObjectById(id));

  mBrightnessSlider->SetSecondaryValue(mgr, 4.f, 0.f, 8.f);
  mStretchSlider->SetSecondaryValue(mgr, 0.f, -10.f, 10.f);
  mPositionXSlider->SetSecondaryValue(mgr, 0.f, -30.f, 30.f);
  mPositionYSlider->SetSecondaryValue(mgr, 0.f, -19.f, 19.f);
  mHudAlphaSlider->SetSecondaryValue(mgr, 255.f, 0.f, 255.f);
  mHelmetAlphaSlider->SetSecondaryValue(mgr, 255.f, 0.f, 255.f);
  mSfxVolumeSlider->SetSecondaryValue(mgr, 105.f, 0.f, 105.f);
  mMusicVolumeSlider->SetSecondaryValue(mgr, 79.f, 0.f, 105.f);

  {
    rstl::vector< TUniqueId > ids = FindConnectedObjects(mgr, kSS_InternalState13, kSM_None);
    mFragLimitMenu = TCastToPtr< CScriptGuiMenu >(mgr.ObjectById(ids[0]));
    for (int i = 1; i < ids.size(); ++i) {
      CScriptTextPane* pane = TCastToPtr< CScriptTextPane >(mgr.ObjectById(ids[i]));
      char buf[16];
      sprintf(buf, "%d", gpTweakGame->GetDeathMatchFragLimit(i - 1));
      pane->TextSupport().SetText(CStringExtras::ConvertToUNICODE(rstl::string_l(buf)));
    }
  }

  {
    rstl::vector< TUniqueId > ids = FindConnectedObjects(mgr, kSS_InternalState14, kSM_None);
    mDeathMatchTimeMenu = TCastToPtr< CScriptGuiMenu >(mgr.ObjectById(ids[0]));
    for (int i = 1; i < ids.size(); ++i) {
      CScriptTextPane* pane = TCastToPtr< CScriptTextPane >(mgr.ObjectById(ids[i]));
      char buf[16];
      sprintf(buf, "%02d", static_cast< int >(gpTweakGame->GetDeathMatchTimeLimit(i - 1)));
      pane->TextSupport().SetText(CStringExtras::ConvertToUNICODE(rstl::string_l(buf)));
    }
  }

  {
    rstl::vector< TUniqueId > ids = FindConnectedObjects(mgr, kSS_LightDamage, kSM_None);
    mCoinLimitMenu = TCastToPtr< CScriptGuiMenu >(mgr.ObjectById(ids[0]));
    for (int i = 1; i < ids.size(); ++i) {
      CScriptTextPane* pane = TCastToPtr< CScriptTextPane >(mgr.ObjectById(ids[i]));
      char buf[16];
      sprintf(buf, "%d", gpTweakGame->GetCoinGameCoinLimit(i - 1));
      pane->TextSupport().SetText(CStringExtras::ConvertToUNICODE(rstl::string_l(buf)));
    }
  }

  {
    rstl::vector< TUniqueId > ids = FindConnectedObjects(mgr, kSS_InternalState15, kSM_None);
    mCoinTimeMenu = TCastToPtr< CScriptGuiMenu >(mgr.ObjectById(ids[0]));
    for (int i = 1; i < ids.size(); ++i) {
      CScriptTextPane* pane = TCastToPtr< CScriptTextPane >(mgr.ObjectById(ids[i]));
      char buf[16];
      sprintf(buf, "%02d", static_cast< int >(gpTweakGame->GetCoinGameTimeLimit(i - 1)));
      pane->TextSupport().SetText(CStringExtras::ConvertToUNICODE(rstl::string_l(buf)));
    }
  }

  id = FindConnectedObject(mgr, kSS_InternalState19, kSM_None);
  mDeathMatchMusicMenu = TCastToPtr< CScriptGuiMenu >(mgr.ObjectById(id));
  id = FindConnectedObject(mgr, kSS_AIS3, kSM_None);
  mCoinMusicMenu = TCastToPtr< CScriptGuiMenu >(mgr.ObjectById(id));

  {
    rstl::vector< TUniqueId > ids = FindConnectedObjects(mgr, kSS_InternalState16, kSM_None);
    mCopySwitch = TCastToPtr< CScriptSwitch >(mgr.ObjectById(ids[0]));
    mEraseSwitch = TCastToPtr< CScriptSwitch >(mgr.ObjectById(ids[1]));
  }

  {
    rstl::vector< TUniqueId > ids = FindConnectedObjects(mgr, kSS_InternalState17, kSM_None);
    mLoadSwitch = TCastToPtr< CScriptSwitch >(mgr.ObjectById(ids[0]));
    mStartSwitch = TCastToPtr< CScriptSwitch >(mgr.ObjectById(ids[1]));
  }

  {
    rstl::vector< TUniqueId > ids = FindConnectedObjects(mgr, kSS_InternalState18, kSM_None);
    for (int i = 0; i < ids.size(); ++i) {
      mSlotCountRelays.push_back(TCastToPtr< CScriptRelay >(mgr.ObjectById(ids[i])));
    }
  }

  {
    rstl::vector< TUniqueId > ids = FindConnectedObjects(mgr, kSS_PowerDamage, kSM_None);
    for (int i = 0; i < ids.size(); ++i) {
      mOptionsPages.push_back(fn_800977A4(mgr.ObjectById(ids[i])));
    }
  }

  {
    rstl::vector< TUniqueId > ids = FindConnectedObjects(mgr, kSS_DarkDamage, kSM_None);
    for (int i = 0; i < ids.size(); ++i) {
      mResetPages.push_back(fn_800977A4(mgr.ObjectById(ids[i])));
    }
  }

  {
    rstl::vector< TUniqueId > ids = FindConnectedObjects(mgr, kSS_BombDamage, kSM_None);
    for (int i = 0; i < ids.size(); ++i) {
      mSlotNamePanes.push_back(TCastToPtr< CScriptTextPane >(mgr.ObjectById(ids[i])));
    }
  }

  {
    rstl::vector< TUniqueId > ids = FindConnectedObjects(mgr, kSS_MissileDamage, kSM_None);
    for (int i = 0; i < ids.size(); ++i) {
      mValuePanes.push_back(TCastToPtr< CScriptTextPane >(mgr.ObjectById(ids[i])));
    }
  }

  {
    rstl::vector< TUniqueId > ids = FindConnectedObjects(mgr, kSS_AnnihilatorDamage, kSM_None);
    mUnlockSwitches.reserve(ids.size());
    for (int i = 0; i < ids.size(); ++i) {
      mUnlockSwitches.push_back_unsafe(TCastToPtr< CScriptSwitch >(mgr.ObjectById(ids[i])));
    }
  }

  {
    rstl::vector< TUniqueId > ids = FindConnectedObjects(mgr, kSS_PowerBombDamage, kSM_None);
    mSounds.reserve(ids.size());
    for (int i = 0; i < ids.size(); ++i) {
      mSounds.push_back_unsafe(TCastToPtr< CScriptSound >(mgr.ObjectById(ids[i])));
    }
  }

  mSoundVolumes.reserve(mSounds.size());
  for (rstl::vector< CScriptSound* >::iterator it = mSounds.begin(); it != mSounds.end(); ++it) {
    mSoundVolumes.push_back_unsafe((*it)->GetVolume());
  }
  UpdateSoundVolumes();

  id = FindConnectedObject(mgr, kSS_BoostBallDamage, kSM_None);
  mGalleryEntity = fn_800977A4(mgr.ObjectById(id));

  CGameState* gameState = gpGameState;
  const uint gameMode = gameState->PreviousGameResults().mGameMode;
  if (gameMode == CFrontEndGameMode::kSGM_Coin || gameMode == CFrontEndGameMode::kSGM_DeathMatch) {
    rstl::vector< CScriptSwitch* > switches;
    {
      rstl::vector< TUniqueId > ids = FindConnectedObjects(mgr, kSS_CannonBallDamage, kSM_None);
      switches.reserve(ids.size());
      for (int i = 0; i < ids.size(); ++i) {
        switches.push_back_unsafe(TCastToPtr< CScriptSwitch >(mgr.ObjectById(ids[i])));
      }
    }
    for (rstl::vector< CScriptSwitch* >::iterator it = switches.begin(); it != switches.end();
         ++it) {
      mgr.SendScriptMsg(*it, GetUniqueId(), kSM_Open, GetUniqueId());
    }

    const CGameState::SPreviousGameResults& results = gameState->PreviousGameResults();
    for (int i = 0; i < results.mPlayerCount; ++i) {
      const CGameState::SPlayerResult& player = results.mPlayers[i];
      SPlayerSetup& setup = mPlayerSetups[player.mPlayerSelection];
      setup.mInvertMenu->SetSelection(player.mRumbleEnabled, mgr);
      setup.mRumbleMenu->SetSelection(player.xc_, mgr);
      mgr.SendScriptMsg(setup.mJoinedSwitch, GetUniqueId(), kSM_Open, GetUniqueId());
      mgr.SendScriptMsg(setup.mReadySwitch, GetUniqueId(), kSM_Open, GetUniqueId());
      if (gameState->PreviousGameResults().mGameMode == CFrontEndGameMode::kSGM_Coin) {
        mgr.SendScriptMsg(mCoinEntity, GetUniqueId(), kSM_InternalMessage02, GetUniqueId());
      }
    }
  }
}

void CScriptGuiFrontEndScreen::Think(float dt, CStateManager& mgr) {
  CActor::Think(dt, mgr);

  CSaveGameScreen* saveScreen = mgr.mSaveGameScreen.get();
  if (saveScreen != nullptr && saveScreen->PumpLoad()) {
    if (!mCardDriverReset) {
      saveScreen->ResetCardDriver();
      mCardDriverReset = true;
    }

    const CIOWin::EMessageReturn ret = saveScreen->GetMessageReturn();
    if (ret == CIOWin::kMR_Exit) {
      mgr.DeleteSaveGameScreen();
      mGameStarted = true;
      SendScriptMsgs(kSS_Arrived, mgr);
    } else if (ret == CIOWin::kMR_RemoveIOWin || ret == CIOWin::kMR_RemoveIOWinAndExit) {
      mSaveScreenFailed = true;
      SendScriptMsgs(kSS_DGNR, mgr);
      CloseSaveGameScreen(mgr);
    }

    if (!mSaveScreenFailed) {
      const bool busy = saveScreen->GetUIType() != CSaveGameScreen::kUIT_SaveReady;
      if (busy != mSaveScreenBusy) {
        mSaveScreenBusy = busy;
        if (busy) {
          SendScriptMsgs(kSS_Frozen, mgr);
        } else {
          SendScriptMsgs(kSS_UnFrozen, mgr);
          RefreshSaveSlots(mgr);
        }
      }
    }
  }
}

void CScriptGuiFrontEndScreen::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CActor::AcceptScriptMsg(mgr, msg);
  CEntity* sender = mgr.ObjectById(msg.GetSenderId());

  switch (msg.GetMessage()) {
  case kSM_SetToZero:
    StartMultiplayerGame(mgr);
    break;
  case kSM_AreaLoaded:
    CollectWidgets(mgr);
    UpdateUnlocks(mgr);
    break;
  case kSM_InternalMessage05:
    SelectSaveSlot(mgr, msg.GetSenderId());
    break;
  case kSM_Action:
    StartSelectedGame(mgr);
    break;
  case kSM_InternalMessage14:
    if (mSaveScreenFailed) {
      break;
    }
  case kSM_Unlock:
    OpenSaveGameScreen(mgr);
    break;
  case kSM_Lock:
    CloseSaveGameScreen(mgr);
    break;
  case kSM_InternalMessage00:
    StoreOptionWidget(mgr, sender);
    break;
  case kSM_InternalMessage01:
    CompareOptionWidget(mgr, sender);
    break;
  case kSM_InternalMessage02:
    RestoreOptionWidget(mgr, sender);
    break;
  case kSM_InternalMessage03:
    ResetOptionWidget(mgr, sender);
    break;
  case kSM_InternalMessage04:
    ApplyOptionWidget(mgr, sender);
    break;
  case kSM_InternalMessage06:
    CopySelectedGame(mgr);
    break;
  case kSM_InternalMessage07:
    EraseSelectedGame(mgr);
    break;
  case kSM_InternalMessage08:
    LoadOptions(mgr, sender);
    break;
  case kSM_InternalMessage09:
    RecordOptions(mgr);
    break;
  case kSM_InternalMessage10:
    SaveOptions(mgr);
    break;
  case kSM_InternalMessage11:
    HighlightSelectedSlot(mgr);
    break;
  case kSM_InternalMessage12:
    UpdateControllerCount(mgr);
    break;
  case kSM_InternalMessage13:
    ShowSlideShow(mgr);
    break;
  default:
    break;
  }
}

void CScriptGuiFrontEndScreen::PreRender(CStateManager& mgr) { mgr.RenderLastHUD(GetUniqueId()); }

void CScriptGuiFrontEndScreen::Render(const CStateManager& mgr) const {
  if (CSaveGameScreen* saveScreen = mgr.mSaveGameScreen.get()) {
    saveScreen->Draw();
  }
}

void CScriptGuiFrontEndScreen::UpdateControllerCount(CStateManager& mgr) {
  int count = 0;
  for (int i = 0; i < 4; ++i) {
    if (gpController->GetGamepadData(i).DeviceIsPresent()) {
      ++count;
    }
  }

  const bool multiple = count > 1;
  if (multiple != mMultipleControllers) {
    mMultipleControllers = multiple;
    SendScriptMsgs(multiple ? kSS_MaxReached : kSS_Zero, mgr);
  }
}

void CScriptGuiFrontEndScreen::StartMultiplayerGame(CStateManager& mgr) {
  CGameMode& gameMode = static_cast< const CGameState* >(gpGameState)->GetGameMode();
  if (gameMode.GetGameModeType() != CFrontEndGameMode::kSGM_FrontEnd) {
    return;
  }

  CFrontEndGameMode& frontEnd = static_cast< CFrontEndGameMode& >(gameMode);
  uint type = CFrontEndGameMode::kSGM_SinglePlayer;
  if (mDeathMatchSwitch->IsOpened()) {
    type = CFrontEndGameMode::kSGM_DeathMatch;
  } else if (mCoinSwitch->IsOpened()) {
    type = CFrontEndGameMode::kSGM_Coin;
  }
  frontEnd.SetNextGameType4CC(type);

  if (type == CFrontEndGameMode::kSGM_DeathMatch) {
    frontEnd.SetNextGameFragLimit(
        gpTweakGame->GetDeathMatchFragLimit(mFragLimitMenu->GetSelection()));
    frontEnd.SetNextGameTimeLimit(
        60.f * gpTweakGame->GetDeathMatchTimeLimit(mDeathMatchTimeMenu->GetSelection()));
    frontEnd.SetNextGameMusicIndex(mDeathMatchMusicMenu->GetSelection());
  } else {
    const int coinLimit = mCoinLimitMenu->GetSelection();
    frontEnd.SetNextGameCoinLimit(coinLimit != 0 ? gpTweakGame->GetCoinGameCoinLimit(coinLimit)
                                                 : -1);
    frontEnd.SetNextGameTimeLimit(60.f *
                                  gpTweakGame->GetCoinGameTimeLimit(mCoinTimeMenu->GetSelection()));
    frontEnd.SetNextGameMusicIndex(mCoinMusicMenu->GetSelection());
  }

  rstl::reserved_vector< CFrontEndPlayerData, 4 > players;
  for (int i = 0; i < 4; ++i) {
    const SPlayerSetup& setup = mPlayerSetups[i];
    if (setup.mJoinedSwitch->IsOpened()) {
      const bool rumble = setup.mRumbleMenu->GetSelection() == 1;
      const bool invert = setup.mInvertMenu->GetSelection() == 1;
      players.push_back(CFrontEndPlayerData(i, CPlayerOptions(rumble, invert)));
    }
  }

  CAssetId worldId = kInvalidAssetId;
  CAssetId areaId = kInvalidAssetId;
  if (players.size() == 0) {
    worldId = mDefaultTeleporter->GetWorldId();
    areaId = mDefaultTeleporter->GetAreaId();
  } else {
    for (rstl::vector< CScriptWorldTeleporter* >::iterator it = mTeleporters.begin();
         it != mTeleporters.end(); ++it) {
      if ((*it)->GetActive()) {
        worldId = (*it)->GetWorldId();
        areaId = (*it)->GetAreaId();
        break;
      }
    }
    frontEnd.SetPlayerData(players);
  }

  if (!mGameStarted) {
    CGameState state;
    state.SetDesiredWorldId(worldId);
    state.StateForWorld(worldId).SetDesiredAreaAssetId(areaId);
    gpGameState->RecordCompressedGameState(gpGameState->SystemOptions().GetSaveIdx(), state);
  }

  gpMain->SetRestartMode(CMain::kRM_None);
  mgr.QuitGame();
}

void CScriptGuiFrontEndScreen::SelectSaveSlot(CStateManager& mgr, TUniqueId slotId) {
  for (int i = 0; i < mSaveSlots.size(); ++i) {
    if (slotId == mSaveSlots[i].mSlotEntity->GetUniqueId()) {
      mSelectedSlot = i;
      const bool used = mgr.mSaveGameScreen->GetGameData(mSelectedSlot) != nullptr;
      const EScriptObjectMessage msg = used ? kSM_Open : kSM_Close;
      mgr.SendScriptMsg(mEraseSwitch, GetUniqueId(), msg);
      mgr.SendScriptMsg(mStartSwitch, GetUniqueId(), msg);
      return;
    }
  }
}

void CScriptGuiFrontEndScreen::StartSelectedGame(CStateManager& mgr) {
  CSaveGameScreen* saveScreen = mgr.mSaveGameScreen.get();
  if (saveScreen->GetGameData(mSelectedSlot) == nullptr) {
    gpGameState->SetHardMode(mSaveSlots[mSelectedSlot].mDifficultyMenu->GetSelection() > 0);
  }
  saveScreen->StartGame(mSelectedSlot);
  mgr.SendScriptMsg(mSaveSlots[mSelectedSlot].mSlotEntity, GetUniqueId(), kSM_Open);
}

void CScriptGuiFrontEndScreen::RefreshSaveSlots(CStateManager& mgr) {
  UpdateSaveSlots(mgr);
  UpdateUnlocks(mgr);
}

void CScriptGuiFrontEndScreen::UpdateSaveSlots(CStateManager& mgr) {
  CSaveGameScreen* saveScreen = mgr.mSaveGameScreen.get();
  const bool normalCompleted = gpGameState->SystemOptions()
                                   .EnvVars()
                                   .FindEnvironmentVariable("NormalModeCompleted")
                                   ->GetValue() > 0;
  int usedSlots = 0;
  for (int i = 0; i < 3; ++i) {
    const CGameState::GameFileStateInfo* data = saveScreen->GetGameData(i);
    rstl::wstring title;
    rstl::wstring worldName;
    rstl::wstring playTime;
    if (data != nullptr) {
      ++usedSlots;
      const char* key = data->mHardMode ? "Hard1" : "Slot1";
      const CStringTable* table = *mStringTable;
      title.assign(table->GetString(table->GetStringIndex(key) + i), -1);
      worldName.reserve(0x80);
      playTime.reserve(0x80);

      const CSaveWorldMemory& worldMemory = gpMemoryCard->GetSaveWorldMemory(data->mMlvlId);
      const wchar_t* name =
          data->x21_ ? worldMemory.GetDarkFrontEndName() : worldMemory.GetFrontEndName();
      if (name != nullptr) {
        worldName.assign(name, -1);
      }
      if (worldName.length() == 0) {
        worldName.assign(L"NO NAME WORLD", -1);
      }

      char buf[64];
      sprintf(buf, " %02d%%", data->mItemPercent);
      worldName.append(CStringExtras::ConvertToUNICODE(rstl::string_l(buf)));

      sprintf(buf, "%02d:%02d ", static_cast< int >(data->mPlayTime) / 3600,
              static_cast< int >(data->mPlayTime) % 3600 / 60);
      playTime = CStringExtras::ConvertToUNICODE(rstl::string_l(buf));
      playTime.append((*mStringTable)->GetString("TimeElapsed"), -1);
    } else {
      const CStringTable* table = *mStringTable;
      title.assign(table->GetString(table->GetStringIndex("New1") + i), -1);
    }

    mSaveSlots[i].mTitle->TextSupport().SetText(title);
    mSaveSlots[i].mWorldName->TextSupport().SetText(worldName);
    mSaveSlots[i].mPlayTime->TextSupport().SetText(playTime);
    mgr.SendScriptMsg(mSaveSlots[i].mUsedSwitch, GetUniqueId(),
                      data != nullptr ? kSM_Open : kSM_Close);
    mgr.SendScriptMsg(mSaveSlots[i].mNewGameSwitch, GetUniqueId(),
                      data == nullptr && normalCompleted ? kSM_Open : kSM_Close);
    mgr.SendScriptMsg(mSaveSlots[i].mNewGameSwitch, GetUniqueId(), kSM_Activate);
    mgr.SendScriptMsg(mEraseSwitch, GetUniqueId(), kSM_Open);
    mgr.SendScriptMsg(mCopySwitch, GetUniqueId(),
                      usedSlots > 0 && usedSlots < 3 ? kSM_Open : kSM_Close);
    mgr.SendScriptMsg(mStartSwitch, GetUniqueId(), kSM_Open);
    mgr.SendScriptMsg(mLoadSwitch, GetUniqueId(), usedSlots > 0 ? kSM_Open : kSM_Close);
    mgr.SendScriptMsg(mSlotCountRelays[0], GetUniqueId(),
                      usedSlots == 0 ? kSM_Activate : kSM_Deactivate);
    mgr.SendScriptMsg(mSlotCountRelays[1], GetUniqueId(),
                      usedSlots == 3 ? kSM_Activate : kSM_Deactivate);
    mSlotNamePanes[i]->TextSupport().SetText(title);
  }
}

void CScriptGuiFrontEndScreen::UpdateUnlocks(CStateManager& mgr) {
  for (int menu = 0; menu < 2; ++menu) {
    CScriptGuiMenu* musicMenu;
    if (menu == 0) {
      musicMenu = mCoinMusicMenu;
    } else {
      musicMenu = mDeathMatchMusicMenu;
    }
    musicMenu->BuildItemList(mgr);
    for (int i = 1; i < musicMenu->GetItems().size(); ++i) {
      CScriptGuiWidget* item =
          TCastToPtr< CScriptGuiWidget >(mgr.ObjectById(musicMenu->GetItem(i)));
      CEnvironmentVariable* var = gpGameState->SystemOptions().EnvVars().FindEnvironmentVariable(
          CBasics::Stringize("UnlockMusic%d", i));
      bool unlocked = true;
      if (var != nullptr) {
        unlocked = var->GetValue() == var->GetMaximum();
      }
      item->SetLocked(!unlocked, mgr);
    }
  }

  for (int i = 0; i < mUnlockSwitches.size(); ++i) {
    CScriptSwitch* unlockSwitch = mUnlockSwitches[i];
    CEnvironmentVariable* var = gpGameState->SystemOptions().EnvVars().FindEnvironmentVariable(
        CBasics::Stringize("UnlockMap%d", i));
    bool unlocked = true;
    if (var != nullptr) {
      unlocked = var->GetValue() == var->GetMaximum();
    }
    mgr.SendScriptMsg(unlockSwitch, GetUniqueId(), unlocked ? kSM_Open : kSM_Close);
  }

  const bool galleriesUnlocked = CSlideShow::GetGalleriesUnlocked() != 0;
  mgr.SendScriptMsg(mGalleryEntity, GetUniqueId(),
                    galleriesUnlocked ? kSM_Activate : kSM_Deactivate, GetUniqueId());
}

void CScriptGuiFrontEndScreen::OpenSaveGameScreen(CStateManager& mgr) {
  if (mgr.mSaveGameScreen.get() == nullptr) {
    mgr.CreateSaveGameScreen();
    mSaveScreenFailed = false;
  }
}

void CScriptGuiFrontEndScreen::CloseSaveGameScreen(CStateManager& mgr) {
  if (mgr.mSaveGameScreen.get() != nullptr) {
    mgr.DeleteSaveGameScreen();
    mCardDriverReset = false;
  }
  UpdateUnlocks(mgr);
}

void CScriptGuiFrontEndScreen::StoreOptionWidget(CStateManager& mgr, CEntity* widget) {
  CScriptGuiSlider* slider = TCastToPtr< CScriptGuiSlider >(widget);
  CScriptGuiMenu* menu = TCastToPtr< CScriptGuiMenu >(widget);
  CGameOptions& options = gpGameState->GameOptions();
  if (slider == mBrightnessSlider) {
    mSavedBrightness = options.GetScreenBrightness();
  } else if (slider == mStretchSlider) {
    mSavedStretch = options.GetScreenStretch();
  } else if (slider == mPositionXSlider) {
    mSavedPositionX = options.GetScreenPositionX();
  } else if (slider == mPositionYSlider) {
    mSavedPositionY = options.GetScreenPositionY();
  } else if (slider == mHudAlphaSlider) {
    mSavedHudAlpha = options.GetHudAlphaRaw();
  } else if (slider == mHelmetAlphaSlider) {
    mSavedHelmetAlpha = options.GetHelmetAlphaRaw();
  } else if (menu == mHintSystemMenu) {
    mSavedHintSystem = options.GetIsHintSystemEnabled() != 0;
  } else if (menu == mHudLagMenu) {
    mSavedHudLag = options.GetHUDLag() != 0;
  } else if (menu == mInvertYMenu) {
    mSavedInvertY = options.GetInvertYAxis() != 0;
  } else if (menu == mRumbleMenu) {
    mSavedRumble = options.GetIsRumbleEnabled() != 0;
  } else if (slider == mSfxVolumeSlider) {
    mSavedSfxVolume = options.GetSfxVolume();
  } else if (slider == mMusicVolumeSlider) {
    mSavedMusicVolume = options.GetMusicVolume();
  } else if (menu == mSurroundMenu) {
    mSavedSurroundMode = options.GetSurroundMode();
  }
  RestoreOptionWidget(mgr, widget);
}

void CScriptGuiFrontEndScreen::CompareOptionWidget(CStateManager& mgr, CEntity* widget) {
  CScriptGuiSlider* slider = TCastToPtr< CScriptGuiSlider >(widget);
  CScriptGuiMenu* menu = TCastToPtr< CScriptGuiMenu >(widget);
  CGameOptions& options = gpGameState->GameOptions();
  if (slider == mBrightnessSlider) {
    SendScriptMsgs(mSavedBrightness == options.GetScreenBrightness() ? kSS_Left : kSS_Right, mgr);
    mSavedBrightness = 0x80000000;
  } else if (slider == mStretchSlider) {
    SendScriptMsgs(mSavedStretch == options.GetScreenStretch() ? kSS_Left : kSS_Right, mgr);
    mSavedStretch = 0x80000000;
  } else if (slider == mPositionXSlider) {
    SendScriptMsgs(mSavedPositionX == options.GetScreenPositionX() ? kSS_Left : kSS_Right, mgr);
    mSavedPositionX = 0x80000000;
  } else if (slider == mPositionYSlider) {
    SendScriptMsgs(mSavedPositionY == options.GetScreenPositionY() ? kSS_Left : kSS_Right, mgr);
    mSavedPositionY = 0x80000000;
  } else if (slider == mHudAlphaSlider) {
    SendScriptMsgs(mSavedHudAlpha == options.GetHudAlphaRaw() ? kSS_Left : kSS_Right, mgr);
    mSavedHudAlpha = 0x80000000;
  } else if (slider == mHelmetAlphaSlider) {
    SendScriptMsgs(mSavedHelmetAlpha == options.GetHelmetAlphaRaw() ? kSS_Left : kSS_Right, mgr);
    mSavedHelmetAlpha = 0x80000000;
  } else if (menu == mHintSystemMenu) {
    SendScriptMsgs(
        (mSavedHintSystem > 0) == options.GetIsHintSystemEnabled() ? kSS_Left : kSS_Right, mgr);
    mSavedHintSystem = 0x80000000;
  } else if (menu == mHudLagMenu) {
    SendScriptMsgs((mSavedHudLag > 0) == options.GetHUDLag() ? kSS_Left : kSS_Right, mgr);
    mSavedHudLag = 0x80000000;
  } else if (menu == mInvertYMenu) {
    SendScriptMsgs((mSavedInvertY > 0) == options.GetInvertYAxis() ? kSS_Left : kSS_Right, mgr);
    mSavedInvertY = 0x80000000;
  } else if (menu == mRumbleMenu) {
    SendScriptMsgs((mSavedRumble > 0) == options.GetIsRumbleEnabled() ? kSS_Left : kSS_Right, mgr);
    mSavedRumble = 0x80000000;
  } else if (slider == mSfxVolumeSlider) {
    SendScriptMsgs(
        mSavedSfxVolume == static_cast< int >(options.GetSfxVolume()) ? kSS_Left : kSS_Right, mgr);
    mSavedSfxVolume = 0x80000000;
  } else if (slider == mMusicVolumeSlider) {
    SendScriptMsgs(mSavedMusicVolume == static_cast< int >(options.GetMusicVolume()) ? kSS_Left
                                                                                     : kSS_Right,
                   mgr);
    mSavedMusicVolume = 0x80000000;
  } else if (menu == mSurroundMenu) {
    SendScriptMsgs(mSavedSurroundMode == options.GetSurroundMode() ? kSS_Left : kSS_Right, mgr);
    mSavedSurroundMode = 0x80000000;
  }
}

void CScriptGuiFrontEndScreen::RestoreOptionWidget(CStateManager& mgr, CEntity* widget) {
  CScriptGuiSlider* slider = TCastToPtr< CScriptGuiSlider >(widget);
  CScriptGuiMenu* menu = TCastToPtr< CScriptGuiMenu >(widget);
  if (slider == mBrightnessSlider) {
    slider->SetValue(mgr, mSavedBrightness, 0.f, 8.f);
  } else if (slider == mStretchSlider) {
    slider->SetValue(mgr, mSavedStretch, -10.f, 10.f);
  } else if (slider == mPositionXSlider) {
    slider->SetValue(mgr, mSavedPositionX, -30.f, 30.f);
  } else if (slider == mPositionYSlider) {
    slider->SetValue(mgr, mSavedPositionY, -19.f, 19.f);
  } else if (slider == mHudAlphaSlider) {
    slider->SetValue(mgr, mSavedHudAlpha, 0.f, 255.f);
  } else if (slider == mHelmetAlphaSlider) {
    slider->SetValue(mgr, mSavedHelmetAlpha, 0.f, 255.f);
  } else if (menu == mHintSystemMenu) {
    menu->SetSelection(mSavedHintSystem, mgr);
  } else if (menu == mHudLagMenu) {
    menu->SetSelection(mSavedHudLag, mgr);
  } else if (menu == mInvertYMenu) {
    menu->SetSelection(mSavedInvertY, mgr);
  } else if (menu == mRumbleMenu) {
    menu->SetSelection(mSavedRumble, mgr);
  } else if (slider == mSfxVolumeSlider) {
    slider->SetValue(mgr, mSavedSfxVolume, 0.f, 105.f);
  } else if (slider == mMusicVolumeSlider) {
    slider->SetValue(mgr, mSavedMusicVolume, 0.f, 105.f);
  } else if (menu == mSurroundMenu) {
    menu->SetSelection(mSavedSurroundMode, mgr);
  }
  widget->SendScriptMsgs(kSS_Modify, mgr);
}

void CScriptGuiFrontEndScreen::ResetOptionWidget(CStateManager& mgr, CEntity* widget) {
  CScriptGuiSlider* slider = TCastToPtr< CScriptGuiSlider >(widget);
  CScriptGuiMenu* menu = TCastToPtr< CScriptGuiMenu >(widget);
  if (slider == mBrightnessSlider) {
    slider->SetValue(mgr, 4.f, 0.f, 8.f);
  } else if (slider == mStretchSlider) {
    slider->SetValue(mgr, 0.f, -10.f, 10.f);
  } else if (slider == mPositionXSlider) {
    slider->SetValue(mgr, 0.f, -30.f, 30.f);
  } else if (slider == mPositionYSlider) {
    slider->SetValue(mgr, 0.f, -19.f, 19.f);
  } else if (slider == mHudAlphaSlider) {
    slider->SetValue(mgr, 255.f, 0.f, 255.f);
  } else if (slider == mHelmetAlphaSlider) {
    slider->SetValue(mgr, 255.f, 0.f, 255.f);
  } else if (menu == mHintSystemMenu) {
    menu->SetSelection(CGameOptions::kDefaultHintSystem ? 1 : 0, mgr);
  } else if (menu == mHudLagMenu) {
    menu->SetSelection(CGameOptions::kDefaultHUDLag ? 1 : 0, mgr);
  } else if (menu == mInvertYMenu) {
    menu->SetSelection(CGameOptions::kDefaultInvertYAxis ? 1 : 0, mgr);
  } else if (menu == mRumbleMenu) {
    menu->SetSelection(CGameOptions::kDefaultRumble ? 1 : 0, mgr);
  } else if (slider == mSfxVolumeSlider) {
    slider->SetValue(mgr, 105.f, 0.f, 105.f);
  } else if (slider == mMusicVolumeSlider) {
    slider->SetValue(mgr, 79.f, 0.f, 105.f);
  } else if (menu == mSurroundMenu) {
    menu->SetSelection(1, mgr);
  } else {
    ResetOptionPage(mgr, widget);
    return;
  }
  widget->SendScriptMsgs(kSS_Modify, mgr);
}

void CScriptGuiFrontEndScreen::ApplyOptionWidget(CStateManager& mgr, CEntity* widget) {
  CScriptGuiSlider* slider = TCastToPtr< CScriptGuiSlider >(widget);
  CScriptGuiMenu* menu = TCastToPtr< CScriptGuiMenu >(widget);
  CGameOptions& options = gpGameState->GameOptions();
  const float value = slider != nullptr ? slider->GetValue(0.f, 1.f) : 0.f;
  if (slider == mBrightnessSlider) {
    const int brightness = slider->GetRoundedValue(0.f, 8.f);
    if (brightness != options.GetScreenBrightness()) {
      mOptionsDirty = true;
      options.SetScreenBrightness(brightness, true);
    }
    SetPercentText(mValuePanes[0], value);
  } else if (slider == mStretchSlider) {
    const int stretch = slider->GetRoundedValue(-10.f, 10.f);
    if (stretch != options.GetScreenStretch()) {
      mOptionsDirty = true;
      options.SetScreenStretch(stretch, true);
    }
    SetPercentText(mValuePanes[1], value);
  } else if (slider == mPositionXSlider) {
    const int x = slider->GetRoundedValue(-30.f, 30.f);
    if (x != options.GetScreenPositionX()) {
      mOptionsDirty = true;
      options.SetScreenPositionX(x, true);
    }
    SetPercentText(mValuePanes[2], value);
  } else if (slider == mPositionYSlider) {
    const int y = slider->GetRoundedValue(-19.f, 19.f);
    if (y != options.GetScreenPositionY()) {
      mOptionsDirty = true;
      options.SetScreenPositionY(y, true);
    }
    SetPercentText(mValuePanes[3], value);
  } else if (slider == mHudAlphaSlider) {
    const int alpha = slider->GetRoundedValue(0.f, 255.f);
    if (alpha != options.GetHudAlphaRaw()) {
      mOptionsDirty = true;
      options.SetHudAlpha(alpha);
    }
    SetPercentText(mValuePanes[4], value);
  } else if (slider == mHelmetAlphaSlider) {
    const int alpha = slider->GetRoundedValue(0.f, 255.f);
    if (alpha != options.GetHelmetAlphaRaw()) {
      mOptionsDirty = true;
      options.SetHelmetAlpha(alpha);
    }
    SetPercentText(mValuePanes[5], value);
  } else if (menu == mHintSystemMenu) {
    const bool enabled = menu->GetSelection() > 0;
    if (enabled != options.GetIsHintSystemEnabled()) {
      mOptionsDirty = true;
      options.SetIsHintSystemEnabled(enabled);
    }
  } else if (menu == mHudLagMenu) {
    const bool enabled = menu->GetSelection() > 0;
    if (enabled != options.GetHUDLag()) {
      mOptionsDirty = true;
      options.SetHUDLag(enabled);
    }
  } else if (menu == mInvertYMenu) {
    const bool enabled = menu->GetSelection() > 0;
    if (enabled != options.GetInvertYAxis()) {
      mOptionsDirty = true;
      options.SetInvertYAxis(enabled);
    }
  } else if (menu == mRumbleMenu) {
    const bool enabled = menu->GetSelection() > 0;
    if (enabled != options.GetIsRumbleEnabled()) {
      mOptionsDirty = true;
      options.SetIsRumbleEnabled(enabled);
    }
  } else if (slider == mSfxVolumeSlider) {
    const int volume = slider->GetRoundedValue(0.f, 105.f);
    if (volume != static_cast< int >(options.GetSfxVolume())) {
      mOptionsDirty = true;
      options.SetSfxVolume(volume, true);
    }
    SetPercentText(mValuePanes[6], value);
  } else if (slider == mMusicVolumeSlider) {
    const int volume = slider->GetRoundedValue(0.f, 105.f);
    if (volume != static_cast< int >(options.GetMusicVolume())) {
      mOptionsDirty = true;
      options.SetMusicVolume(volume, true);
      UpdateSoundVolumes();
    }
    SetPercentText(mValuePanes[7], value);
  } else if (menu == mSurroundMenu) {
    const int mode = menu->GetSelection();
    if (mode != options.GetSurroundMode()) {
      mOptionsDirty = true;
      options.SetSurroundMode(static_cast< CAudioSys::ESurroundModes >(mode), true);
    }
  }
}

void CScriptGuiFrontEndScreen::ResetOptionPage(CStateManager& mgr, CEntity* widget) {
  if (CEntity* page = fn_800977A4(widget)) {
    for (int i = 0; i < mResetPages.size(); ++i) {
      if (page == mResetPages[i]) {
        mOptionsPage = i;
        return;
      }
    }
    mOptionsPage = -1;
    return;
  }

  mOptionsDirty = true;
  CGameOptions& options = gpGameState->GameOptions();
  switch (mOptionsPage) {
  case 0:
    options.SetScreenBrightness(4, true);
    options.SetScreenStretch(0, true);
    options.SetScreenPositionX(0, true);
    options.SetScreenPositionY(0, true);
    break;
  case 1:
    options.SetHudAlpha(255);
    options.SetHelmetAlpha(255);
    options.SetIsHintSystemEnabled(CGameOptions::kDefaultHintSystem);
    options.SetHUDLag(CGameOptions::kDefaultHUDLag);
    break;
  case 2:
    options.SetInvertYAxis(CGameOptions::kDefaultInvertYAxis);
    options.SetIsRumbleEnabled(CGameOptions::kDefaultRumble);
    break;
  case 3:
    options.SetSfxVolume(105, true);
    options.SetMusicVolume(79, true);
    options.SetSurroundMode(CAudioSys::kSM_Stereo, true);
    UpdateSoundVolumes();
    break;
  case -1:
    options.ResetToDefaults();
    UpdateSoundVolumes();
    break;
  default:
    break;
  }
}

void CScriptGuiFrontEndScreen::CopySelectedGame(CStateManager& mgr) {
  CSaveGameScreen* saveScreen = mgr.mSaveGameScreen.get();
  int target;
  for (target = 0; target < 3; ++target) {
    if (saveScreen->GetGameData(target) == nullptr) {
      break;
    }
  }
  saveScreen->CopyGame(mSelectedSlot, target);
}

void CScriptGuiFrontEndScreen::EraseSelectedGame(CStateManager& mgr) {
  mgr.mSaveGameScreen->EraseGame(mSelectedSlot);
}

void CScriptGuiFrontEndScreen::LoadOptions(CStateManager& mgr, CEntity* page) {
  for (int i = 0; i < mOptionsPages.size(); ++i) {
    if (page == mOptionsPages[i]) {
      switch (i) {
      case 0:
      case 1:
      case 2:
        gpGameState->LoadCompressedGameOptions(i);
        break;
      case 3:
        gpGameState->LoadCompressedMultiplayerOptions();
        break;
      case 4:
        gpGameState->LoadCompressedGameOptions(0);
        break;
      default:
        break;
      }
      gpGameState->GameOptions().EnsureOptions();
      mOptionsPage = i;
      return;
    }
  }
}

void CScriptGuiFrontEndScreen::RecordOptions(CStateManager& mgr) {
  switch (mOptionsPage) {
  case 0:
  case 1:
  case 2:
    gpGameState->RecordCompressedGameOptions(mOptionsPage);
    break;
  case 3:
    gpGameState->RecordCompressedMultiplayerOptions();
    break;
  case 4:
    gpGameState->RecordCompressedGameOptions(0);
    break;
  default:
    break;
  }
}

void CScriptGuiFrontEndScreen::SaveOptions(CStateManager& mgr) {
  if (mOptionsDirty) {
    mOptionsDirty = false;
    if (CSaveGameScreen* saveScreen = mgr.mSaveGameScreen.get()) {
      saveScreen->SaveChanges();
    }
  }
}

void CScriptGuiFrontEndScreen::HighlightSelectedSlot(CStateManager& mgr) {
  if (CSaveGameScreen* saveScreen = mgr.mSaveGameScreen.get()) {
    CEntity* slotEntity = mSaveSlots[saveScreen->GetSaveIdx()].mSlotEntity;
    mgr.SendScriptMsg(slotEntity, GetUniqueId(), kSM_InternalMessage02);
  }
}

void CScriptGuiFrontEndScreen::UpdateSoundVolumes() {
  for (int i = 0; i < mSounds.size(); ++i) {
    mSounds[i]->SetMaxVolume(mSoundVolumes[i] *
                             static_cast< int >(gpGameState->GameOptions().GetMusicVolume()) / 79);
  }
}

void CScriptGuiFrontEndScreen::SetPercentText(CScriptTextPane* pane, float value) {
  char buf[16];
  sprintf(buf, "%02d", static_cast< int >(RoundToNearest(100.f * value)));
  pane->TextSupport().SetText(rstl::string(buf));
}

void CScriptGuiFrontEndScreen::ShowSlideShow(CStateManager& mgr) {
  CArchitectureQueue& queue = mgr.ArchQueue();
  queue.Push(MakeMsg::CreateCreateIOWin(kAMT_IOWinManager, sSlideShowMsgPriority,
                                        sSlideShowDrawPriority, rs_new CSlideShow()));
}

CScriptGuiScreen::CScriptGuiScreen(TUniqueId uid, const rstl::string& name, const CEntityInfo& info)
: CActor(uid, name, info, 0, CTransform4f::Identity(), CModelData::CModelDataNull(),
         CMaterialList(), CActorParameters::None(), kInvalidUniqueId) {}

CEntity* LoadGuiScreen(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrGuiScreen sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrGuiScreen.inc"

  switch (sldrThis.whichScreen) {
  case 1:
    return rs_new CScriptGuiFrontEndScreen(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                                           LdrToEntityInfo(info, sldrThis.editorProperties),
                                           sldrThis.stringTable);
  }
  return nullptr;
}
