#include "MetroidPrime/CSaveGameScreen.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiTableGroup.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CMemoryCard.hpp"
#include "MetroidPrime/CMemoryCardDriver.hpp"
#include "MetroidPrime/SFX/UIMemory.h"
#include "rstl/StringExtras.hpp"

static const char* const skSaveBanner = "TXTR_SaveBanner";
static const char* const skSaveIcon0 = "TXTR_SaveIcon0";
static const char* const skSaveIcon1 = "TXTR_SaveIcon1";
static const char* const skMemoryCardStrings = "STRG_MemoryCard";
static const char* const skGenericMenu = "FRME_GenericMenu";

CSaveGameScreen::EUIType CSaveGameScreen::SelectUIType() const {
  const EState state = mCardDriver->GetState();
  const CMemoryCardDriver::EError error = mCardDriver->GetError();
  if (state == kS_NoCard) {
    return kUIT_NoCardFound;
  }
  if (mUiType == kUIT_ProgressWillBeLost || mUiType == kUIT_AllDataWillBeLost ||
      mUiType == kUIT_NotOriginalCard) {
    return mUiType;
  }
  if (CMemoryCardDriver::IsCardBusy(state)) {
    if (mCardDriver->IsRepairingHeader()) {
      return kUIT_BusyWriting;
    }
    if (state == kS_FileWrite || state == kS_FileCreate) {
      return kUIT_BusyWritingInitial;
    }
    return CMemoryCardDriver::IsCardReading(state) ? kUIT_BusyReading : kUIT_BusyWriting;
  }
  if (state == kS_Ready) {
    return kUIT_SaveReady;
  }
  if (error == CMemoryCardDriver::kE_CardBroken) {
    return kUIT_NeedsFormatBroken;
  }
  if (error == CMemoryCardDriver::kE_CardWrongCharacterSet) {
    return kUIT_NeedsFormatEncoding;
  }
  if (error == CMemoryCardDriver::kE_CardWrongDevice) {
    return kUIT_WrongDevice;
  }
  if (error == CMemoryCardDriver::kE_CardFull) {
    return kUIT_InsufficientSpaceOKCheck;
  }
  if (error == CMemoryCardDriver::kE_CardNon8KSectors) {
    return kUIT_IncompatibleCard;
  }
  if (error == CMemoryCardDriver::kE_FileCorrupted) {
    return kUIT_SaveCorrupt;
  }
  if (error == CMemoryCardDriver::kE_CardIOError) {
    return kUIT_CardDamaged;
  }
  return kUIT_Empty;
}

void CSaveGameScreen::SetUIText() {
  mUiTextDirty = false;

  const CStringTable& strings = *mStrgMemoryCard.GetObject();
  const char* title = nullptr;
  const char* messageName = nullptr;
  const char* choices[4] = {nullptr, nullptr, nullptr, nullptr};

  switch (mUiType) {
  case kUIT_BusyWriting:
    messageName = "StatusWriting";
    break;
  case kUIT_BusyWritingInitial:
    messageName = "StatusWritingInitial";
    break;
  case kUIT_NoCardFound:
    messageName = "NoMemoryCard";
    choices[0] = "ChoiceRetry";
    choices[1] = "ChoiceContinueWithoutSave";
    break;
  case kUIT_NeedsFormatBroken:
    messageName = "CorruptedCard";
    choices[0] = "ChoiceRetry";
    choices[1] = "ChoiceContinueWithoutSave";
    choices[2] = "ChoiceFormatCard";
    break;
  case kUIT_NeedsFormatEncoding:
    messageName = "EncodingMismatch";
    choices[0] = "ChoiceRetry";
    choices[1] = "ChoiceContinueWithoutSave";
    choices[2] = "ChoiceFormatCard";
    break;
  case kUIT_CardDamaged:
    messageName = "DamagedCard";
    choices[0] = "ChoiceRetry";
    choices[1] = "ChoiceContinueWithoutSave";
    break;
  case kUIT_WrongDevice:
    messageName = "WrongDevice";
    choices[0] = "ChoiceRetry";
    choices[1] = "ChoiceContinueWithoutSave";
    break;
  case kUIT_InsufficientSpaceOKCheck:
    messageName = "InsufficientSpaceMain";
    choices[0] = "ChoiceRetry";
    choices[1] = "ChoiceContinueWithoutSave";
    choices[2] = "ChoiceManageMemoryCard";
    break;
  case kUIT_IncompatibleCard:
    messageName = "BadSectorSize";
    choices[0] = "ChoiceRetry";
    choices[1] = "ChoiceContinueWithoutSave";
    break;
  case kUIT_SaveCorrupt:
    messageName = "CorruptedFile";
    choices[0] = "ChoiceRetry";
    choices[1] = "ChoiceContinueWithoutSave";
    choices[2] = "ChoiceDeleteCorruptedFile";
    break;
  case kUIT_ProgressWillBeLost:
    title = "TitleWarning";
    messageName = "IPLWarning";
    choices[0] = "ChoiceCancel";
    choices[1] = "ChoiceContinueWithWarning";
    break;
  case kUIT_NotOriginalCard:
    title = "TitleWarning";
    messageName = "ConfirmOverwrite";
    choices[0] = mSaveCtx == kSC_InGame ? "ChoiceCancel" : "ChoiceContinueWithoutSave";
    choices[1] = "ChoiceContinueWithWarning";
    break;
  case kUIT_AllDataWillBeLost:
    title = "TitleWarning";
    messageName = "ConfirmFormat";
    choices[0] = "ChoiceCancel";
    choices[1] = "ChoiceContinueWithWarning";
    break;
  case kUIT_SaveReady:
    if (mSaveCtx == kSC_InGame) {
      messageName = "SaveFile";
      choices[0] = "ChoiceYes";
      choices[1] = "ChoiceNo";
    }
    break;
  default:
    break;
  }

  const rstl::wstring empty = rstl::wstring_l(L"");
  const rstl::wstring messageTitle =
      title == nullptr ? empty : rstl::wstring_l(strings.GetString(title));
  const rstl::wstring message =
      messageTitle +
      (messageName == nullptr ? empty : rstl::wstring_l(strings.GetString(messageName)));
  mTextpaneMessage->TextSupport().SetText(message);
  mTextpaneChoice0->TextSupport().SetText(
      choices[0] == nullptr ? empty : rstl::wstring_l(strings.GetString(choices[0])));
  mTextpaneChoice1->TextSupport().SetText(
      choices[1] == nullptr ? empty : rstl::wstring_l(strings.GetString(choices[1])));
  mTextpaneChoice2->TextSupport().SetText(
      choices[2] == nullptr ? empty : rstl::wstring_l(strings.GetString(choices[2])));
  mTextpaneChoice3->TextSupport().SetText(
      choices[3] == nullptr ? empty : rstl::wstring_l(strings.GetString(choices[3])));
  mTextpaneChoice0->SetIsSelectable(choices[0] != nullptr);
  mTextpaneChoice1->SetIsSelectable(choices[1] != nullptr);
  mTextpaneChoice2->SetIsSelectable(choices[2] != nullptr);
  mTextpaneChoice3->SetIsSelectable(choices[3] != nullptr);
  mTablegroupChoices->SetUserSelection(0);
  mTablegroupChoices->SetIsActive(choices[0] != nullptr || choices[1] != nullptr ||
                                  choices[2] != nullptr || choices[3] != nullptr);
  SetUIColors();
  mHasMessage = messageName != nullptr;
}

CMemoryCardDriver* CSaveGameScreen::ConstructCardDriver(bool importPersistent) {
  return rs_new CMemoryCardDriver(
      CMemoryCardSys::kCS_SlotA, gpResourceFactory->GetResourceIdByName(skSaveBanner)->GetId(),
      gpResourceFactory->GetResourceIdByName(skSaveIcon0)->GetId(),
      gpResourceFactory->GetResourceIdByName(skSaveIcon1)->GetId(), importPersistent);
}

CSaveGameScreen::CSaveGameScreen(ESaveContext saveContext, u64 cardSerial)
: mSaveCtx(saveContext)
, mSerial(cardSerial)
, mUiType(kUIT_Empty)
, mTxtrSaveBanner(gpSimplePool->GetObj(skSaveBanner))
, mTxtrSaveIcon0(gpSimplePool->GetObj(skSaveIcon0))
, mTxtrSaveIcon1(gpSimplePool->GetObj(skSaveIcon1))
, mStrgMemoryCard(gpSimplePool->GetObj(skMemoryCardStrings))
, mFrmeGenericMenu(gpSimplePool->GetObj(skGenericMenu))
, mLoadedFrame(nullptr)
, mCardDriver(nullptr)
, mIowRet(CIOWin::kMR_Normal)
, mNavConfirmSfx(SFXui_x_quitsel_00_oneshot)
, mNavMoveSfx(SFXui_x_quitaff_00_oneshot)
, mNavBackSfx(SFXui_x_quitsel_00_oneshot)
, mNeedsDriverReset(false)
, mUiTextDirty(false)
, mSavingDisabled(false)
, mInGame(mSaveCtx == kSC_InGame)
, mFrontEndSfx(mSaveCtx == kSC_FrontEnd)
, mHasMessage(false) {
  mTxtrSaveBanner.Lock();
  mTxtrSaveIcon0.Lock();
  mTxtrSaveIcon1.Lock();
  mStrgMemoryCard.Lock();
  mFrmeGenericMenu.Lock();

  const rstl::vector< CMemoryCard::MemoryWorld >& worlds = gpMemoryCard->GetMemoryWorlds();
  mSaveWorlds.reserve(worlds.size());
  for (rstl::vector< CMemoryCard::MemoryWorld >::const_iterator it = worlds.begin();
       it != worlds.end(); ++it) {
    TToken< CWorldSaveGameInfo > token =
        gpSimplePool->GetObj(SObjectTag('SAVW', it->second.GetSaveWorldAssetId()));
    token.Lock();
    mSaveWorlds.push_back_unsafe(token);
  }
}

CSaveGameScreen::~CSaveGameScreen() {}

void CSaveGameScreen::ResetCardDriver() {
  mSavingDisabled = false;
  mCardDriver = nullptr;
  mCardDriver = ConstructCardDriver(mSaveCtx == kSC_FrontEnd && !mNeedsDriverReset);
  mCardDriver->StartCardProbe();
  mUiType = kUIT_Empty;
  mIowRet = CIOWin::kMR_Normal;
  SetUIText();
}

bool CSaveGameScreen::PumpLoad() {
  if (mLoadedFrame != nullptr) {
    return true;
  }
  const TCachedToken< CTexture >& banner = mTxtrSaveBanner;
  const TCachedToken< CTexture >& icon0 = mTxtrSaveIcon0;
  const TCachedToken< CTexture >& icon1 = mTxtrSaveIcon1;
  if (!banner.IsLoaded() || !icon0.IsLoaded() || !icon1.IsLoaded() || !mStrgMemoryCard.TryCache()) {
    return false;
  }
  for (rstl::vector< TToken< CWorldSaveGameInfo > >::const_iterator it = mSaveWorlds.begin();
       it != mSaveWorlds.end(); ++it) {
    if (!it->IsLoaded()) {
      return false;
    }
  }
  if (mFrmeGenericMenu.TryCache()) {
    mLoadedFrame = mFrmeGenericMenu.GetObject();
    mTextpaneMessage = static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_message"));
    mTablegroupChoices =
        static_cast< CGuiTableGroup* >(mLoadedFrame->FindWidget("tablegroup_choices"));
    mTextpaneChoice0 = static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_choice0"));
    mTextpaneChoice1 = static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_choice1"));
    mTextpaneChoice2 = static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_choice2"));
    mTextpaneChoice3 = static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_choice3"));

    CGuiWidget* background = mLoadedFrame->FindWidget("model_messagebg");
    if (background != nullptr && mSaveCtx != kSC_FrontEnd) {
      background->SetVisibility(false, kTM_Children);
    }
    mTablegroupChoices->SetMenuAdvanceCallback(
        TFunctor1FromMethod< CSaveGameScreen, CGuiTableGroup* const >::Make(
            *this, &CSaveGameScreen::DoAdvance));
    mTablegroupChoices->SetMenuSelectionChangeCallback(
        TFunctor2FromMethod< CSaveGameScreen, CGuiTableGroup* const, const int >::Make(
            *this, &CSaveGameScreen::DoSelectionChange));
  } else {
    return false;
  }

  mCardDriver = ConstructCardDriver(mSaveCtx == kSC_FrontEnd);
  if (mSaveCtx == kSC_InGame) {
    mCardDriver->StartCardProbe();
  }
  mUiType = SelectUIType();
  SetUIText();
  return true;
}

CIOWin::EMessageReturn CSaveGameScreen::Update(float dt) {
  if (!PumpLoad()) {
    return CIOWin::kMR_Normal;
  }
  mLoadedFrame->Update(dt);
  mCardDriver->Update();

  const EState state = mCardDriver->GetState();
  const CMemoryCardDriver::EError error = mCardDriver->GetError();
  if (state == kS_DriverClosed) {
    if (mNeedsDriverReset) {
      ResetCardDriver();
      mNeedsDriverReset = false;
    } else {
      mIowRet = CIOWin::kMR_Exit;
    }
  } else if (state == kS_CardCheckDone && mUiType != kUIT_NotOriginalCard) {
    const u64 cardSerial = mCardDriver->GetCardSerial();
    if (cardSerial != 0 && cardSerial != mSerial) {
      if (mInGame) {
        mUiType = kUIT_NotOriginalCard;
        mUiTextDirty = true;
      } else {
        mSerial = mCardDriver->GetCardSerial();
        mCardDriver->IndexFiles();
      }
    } else {
      mCardDriver->IndexFiles();
    }
  } else if (state == kS_Ready) {
    if (mNeedsDriverReset) {
      mCardDriver->StartFileWriteTransactional();
    }
  }

  if (mIowRet != CIOWin::kMR_Normal) {
    return mIowRet;
  }
  EUIType oldType = mUiType;
  mUiType = SelectUIType();
  if (oldType != mUiType || mUiTextDirty) {
    SetUIText();
  }

  if (state == kS_NoCard) {
    const ProbeResults result = CMemoryCardSys::IsMemoryCardInserted(CMemoryCardSys::kCS_SlotA);
    if (result.mError == kCR_READY || result.mError == kCR_WRONGDEVICE) {
      ResetCardDriver();
    }
  } else if (state == kS_CardFormatted) {
    ResetCardDriver();
  } else if (state == kS_FileBad && error == CMemoryCardDriver::kE_FileMissing) {
    mCardDriver->StartFileCreate();
  }
  return CIOWin::kMR_Normal;
}

void CSaveGameScreen::ProcessUserInput(const CFinalInput& input) {
  if (mLoadedFrame != nullptr) {
    mLoadedFrame->ProcessUserInput(input);
  }
}

void CSaveGameScreen::ContinueWithoutSaving() {
  mIowRet = CIOWin::kMR_RemoveIOWin;
  gpGameState->SetCardSerial(0);
}

void CSaveGameScreen::Draw() const {
  if (mLoadedFrame != nullptr && mHasMessage) {
    CGraphics::SetDepthRange(0.f, 0.001f);
    mLoadedFrame->Draw(CGuiWidgetDrawParms::Default());
    CGraphics::SetDepthRange(0.f, 1.f);
  }
}

const CGameState::GameFileStateInfo* CSaveGameScreen::GetGameData(int idx) const {
  return mCardDriver->GetGameFileStateInfo(idx);
}

int CSaveGameScreen::GetSaveIdx() const { return mCardDriver->GetSaveIdx(); }

void CSaveGameScreen::EraseGame(int idx) {
  mCardDriver->EraseFileSlot(idx);
  mNeedsDriverReset = true;
  mCardDriver->StartFileWriteTransactional();
}

void CSaveGameScreen::CopyGame(int from, int to) {
  mCardDriver->CopyFileSlot(from, to);
  mNeedsDriverReset = true;
  mCardDriver->StartFileWriteTransactional();
}

void CSaveGameScreen::SaveChanges() {
  if (!mSavingDisabled) {
    mNeedsDriverReset = true;
    mSerial = mCardDriver->GetCardSerial();
    mCardDriver->StartFileWriteTransactional();
  }
}

void CSaveGameScreen::StartGame(int idx) {
  const bool newGame = mCardDriver->GetGameFileStateInfo(idx) == nullptr;
  gpGameState->SystemOptions().SetSaveIdx(idx);
  mCardDriver->ExportPersistentOptions();
  mCardDriver->ExportGameOptions();
  mCardDriver->BuildNewFileSlot(idx);
  if (newGame) {
    mCardDriver->StartFileWriteTransactional();
  } else {
    mIowRet = CIOWin::kMR_Exit;
  }
}

void CSaveGameScreen::DoAdvance(CGuiTableGroup* caller) {
  int userSel = mTablegroupChoices->GetUserSelection();
  int sfx = -1;

  switch (mUiType) {
  case kUIT_Empty:
  case kUIT_BusyReading:
  case kUIT_BusyWriting:
  case kUIT_BusyWritingInitial:
    break;
  case kUIT_NoCardFound:
  case kUIT_CardDamaged:
  case kUIT_WrongDevice:
  case kUIT_IncompatibleCard:
    if (userSel == 1) {
      if (mSaveCtx == kSC_InGame) {
        mIowRet = CIOWin::kMR_RemoveIOWinAndExit;
      } else {
        ContinueWithoutSaving();
      }
      sfx = mNavBackSfx;
    } else if (userSel == 0) {
      ResetCardDriver();
      sfx = mNavConfirmSfx;
    }
    break;
  case kUIT_NeedsFormatBroken:
  case kUIT_NeedsFormatEncoding:
    if (userSel == 1) {
      if (mSaveCtx == kSC_InGame) {
        mIowRet = CIOWin::kMR_RemoveIOWinAndExit;
      } else {
        ContinueWithoutSaving();
      }
      sfx = mNavBackSfx;
    } else if (userSel == 0) {
      ResetCardDriver();
      sfx = mNavConfirmSfx;
    } else if (userSel == 2) {
      mUiType = kUIT_AllDataWillBeLost;
      mUiTextDirty = true;
      sfx = mNavConfirmSfx;
    }
    break;
  case kUIT_InsufficientSpaceOKCheck:
    if (userSel == 1) {
      if (mSaveCtx == kSC_InGame) {
        mIowRet = CIOWin::kMR_RemoveIOWinAndExit;
      } else {
        ContinueWithoutSaving();
      }
      sfx = mNavBackSfx;
    } else if (userSel == 0) {
      ResetCardDriver();
      sfx = mNavConfirmSfx;
    } else if (userSel == 2) {
      if (mSaveCtx == kSC_InGame) {
        mUiType = kUIT_ProgressWillBeLost;
        mUiTextDirty = true;
        sfx = mNavConfirmSfx;
      } else {
        gpMain->SetManageCard(true);
      }
    }
    break;
  case kUIT_SaveCorrupt:
    if (userSel == 2) {
      mCardDriver->StartFileDeleteBad();
      sfx = mNavConfirmSfx;
    } else if (userSel == 1) {
      if (mSaveCtx == kSC_InGame) {
        mIowRet = CIOWin::kMR_RemoveIOWinAndExit;
      } else {
        ContinueWithoutSaving();
      }
      sfx = mNavBackSfx;
    } else if (userSel == 0) {
      ResetCardDriver();
      sfx = mNavConfirmSfx;
    }
    break;
  case kUIT_ProgressWillBeLost:
    if (userSel == 1) {
      gpMain->SetManageCard(true);
    } else if (userSel == 0) {
      mIowRet = CIOWin::kMR_RemoveIOWinAndExit;
      sfx = mNavBackSfx;
    }
    break;
  case kUIT_NotOriginalCard:
    if (userSel == 1) {
      mSerial = mCardDriver->GetCardSerial();
      mUiType = kUIT_Empty;
      mCardDriver->IndexFiles();
      sfx = mNavConfirmSfx;
    } else if (userSel == 0) {
      mIowRet = CIOWin::kMR_RemoveIOWinAndExit;
      sfx = mNavBackSfx;
    }
    break;
  case kUIT_AllDataWillBeLost:
    if (userSel == 1) {
      mCardDriver->StartCardFormat();
      mUiType = kUIT_Empty;
      sfx = mNavConfirmSfx;
    } else if (userSel == 0) {
      ResetCardDriver();
      sfx = mNavBackSfx;
    }
    break;
  case kUIT_SaveReady:
    if (mSaveCtx != kSC_FrontEnd) {
      if (userSel == 0) {
        mCardDriver->BuildExistingFileSlot(gpGameState->SystemOptions().GetSaveIdx());
        mCardDriver->StartFileWriteTransactional();
        sfx = mNavConfirmSfx;
      } else if (userSel == 1) {
        mIowRet = CIOWin::kMR_RemoveIOWinAndExit;
        sfx = mNavBackSfx;
      }
    }
    break;
  }

  if (sfx >= 0) {
    CSfxManager::SfxStart(sfx, 0x7f, 0x40, mFrontEndSfx ? 0 : CSfxManager::kAllAreas, mFrontEndSfx,
                          false, CSfxManager::kMedPriority);
  }
}

void CSaveGameScreen::DoSelectionChange(CGuiTableGroup* caller, int oldSelection) {
  SetUIColors();
  CSfxManager::SfxStart(mNavMoveSfx, 0x7f, 0x40, mFrontEndSfx ? 0 : CSfxManager::kAllAreas,
                        mFrontEndSfx, false, CSfxManager::kMedPriority);
}

void CSaveGameScreen::SetUIColors() {
  const CColor selected(0xffffffff);
  const CColor unselected(uchar(160), uchar(160), uchar(160), uchar(200));
  mTablegroupChoices->SetColors(selected, unselected);
}
