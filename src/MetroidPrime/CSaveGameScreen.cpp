#include "MetroidPrime/CSaveGameScreen.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "MetroidPrime/CMemoryCard.hpp"
#include "MetroidPrime/CMemoryCardDriver.hpp"

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
  switch (error) {
  case CMemoryCardDriver::kE_CardBroken:
    return kUIT_NeedsFormatBroken;
  case CMemoryCardDriver::kE_CardWrongCharacterSet:
    return kUIT_NeedsFormatEncoding;
  case CMemoryCardDriver::kE_CardWrongDevice:
    return kUIT_WrongDevice;
  case CMemoryCardDriver::kE_CardFull:
    return kUIT_InsufficientSpaceOKCheck;
  case CMemoryCardDriver::kE_CardNon8KSectors:
    return kUIT_IncompatibleCard;
  case CMemoryCardDriver::kE_FileCorrupted:
    return kUIT_SaveCorrupt;
  case CMemoryCardDriver::kE_CardIOError:
    return kUIT_CardDamaged;
  default:
    return kUIT_Empty;
  }
}

void CSaveGameScreen::SetUIText() {
  mUiTextDirty = false;
  // TODO: Select Echoes's named strings and menu choices, then set mHasMessage.
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
, mNavConfirmSfx(0x5e3)
, mNavMoveSfx(0x5e1)
, mNavBackSfx(0x5e3)
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
    mSaveWorlds.push_back(token);
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
  if (!mTxtrSaveBanner.GetToken().IsLoaded() || !mTxtrSaveIcon0.GetToken().IsLoaded() ||
      !mTxtrSaveIcon1.GetToken().IsLoaded() || !mStrgMemoryCard.IsLoaded()) {
    return false;
  }
  for (rstl::vector< TToken< CWorldSaveGameInfo > >::const_iterator it = mSaveWorlds.begin();
       it != mSaveWorlds.end(); ++it) {
    if (!it->IsLoaded()) {
      return false;
    }
  }
  // TODO: Bind widgets and CGuiTableGroup callbacks, then construct the card driver.
  // Keep loading incomplete until that initialization is implemented.
  return false;
}

CIOWin::EMessageReturn CSaveGameScreen::Update(float dt) {
  if (!PumpLoad()) {
    return CIOWin::kMR_Normal;
  }
  // TODO: Advance the frame/card driver and handle card, serial and UI transitions.
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
    mLoadedFrame->Draw(CGuiWidgetDrawParms(1.f, CVector3f::Zero()));
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
  // TODO: Dispatch the selected Echoes menu choice and play the corresponding sound.
}

void CSaveGameScreen::DoSelectionChange(CGuiTableGroup* caller, int oldSelection) {
  SetUIColors();
  CSfxManager::SfxStart(mNavMoveSfx, 0x7f, 0x40, mFrontEndSfx ? 0 : CSfxManager::kAllAreas,
                        mFrontEndSfx, false, CSfxManager::kMedPriority);
}

void CSaveGameScreen::SetUIColors() {
  // TODO: Set CGuiTableGroup's selected white and unselected (160,160,160,200) colors.
}
