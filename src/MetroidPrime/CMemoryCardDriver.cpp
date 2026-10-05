#include "MetroidPrime/CMemoryCardDriver.hpp"

#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Streams/CBitStreamReader.hpp"
#include "Kyoto/Streams/CBitStreamWriter.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "Kyoto/Streams/CMemoryStreamOut.hpp"
#include "Kyoto/Streams/COutputStream.hpp"
#include "MetroidPrime/CMemoryCard.hpp"
#include "MetroidPrime/Player/CGameOptions.hpp"
#include "MetroidPrime/Player/CPersistentOptions.hpp"
#include <dolphin/os.h>
#include <stdio.h>

static const char* const skSaveFileName = "MetroidPrime2";

// Diagnostic strings retained in the retail pool; table structure is inferred from Prime.
static const char* const skStateNames[] = {
    "NotLoaded",
    "Loaded",
    "NoCard",
    "Saved",
    "Formatted",
    "Probed",
    "Mounted",
    "CheckedCard",
    "CreatedInitial",
    "WroteCopy",
    "FailedProbe",
    "FailedMount",
    "FailedCheck",
    "FailedDeleteCorruptedFile",
    "FailedLoad",
    "FailedCreateInitial",
    "FailedWriteInitial",
    "FailedWriteCopy",
    "FailedFormat",
    "Probing",
    "Mounting",
    "CheckingCard",
    "DeletingCorruptedFile",
    "Reading",
    "CreatingInitial",
    "WritingInitial",
    "WritingCopy",
    "Formatting",
};

static const char* const skErrorNames[] = {
    "NoError",
    "CorruptedFile",
    "EncodingMismatch",
    "Damaged",
    "WrongDevice",
    "InsufficientSpace",
    "BadSectorSize",
    "NoFile",
};

// Guessed name
static uint GetSaveSignature() {
  static uint signature = 0xffffffff;
  if (signature != 0xffffffff) {
    return signature;
  }
  signature = 0x5553413e;
  const rstl::vector< CMemoryCard::MemoryWorld >& worlds = gpMemoryCard->GetMemoryWorlds();
  for (rstl::vector< CMemoryCard::MemoryWorld >::const_iterator it = worlds.begin();
       it != worlds.end(); ++it) {
    const CAssetId saveId = it->second.GetSaveWorldAssetId();
    TLockedToken< CWorldSaveGameInfo > save(gpSimplePool->GetObj(SObjectTag('SAVW', saveId)));
    signature ^= save->CalculateHash();
  }
  return signature;
}

static bool sDriverExists; // Guessed name

// Guessed name
static bool IsSaveSignatureInvalid(const void* data) {
  return *static_cast< const uint* >(data) != GetSaveSignature();
}

bool CMemoryCardDriver::IsCardBusy(EState state) {
  return state >= kS_CardMount && state <= kS_CardFormat;
}

bool CMemoryCardDriver::IsCardReading(EState state) {
  return state == kS_CardProbe || state == kS_CardMount || state == kS_CardCheck ||
         state == kS_FileRead;
}

CMemoryCardDriver::CMemoryCardDriver(CMemoryCardSys::EMemoryCardPort cardPort, CAssetId saveBanner,
                                     CAssetId saveIcon0, CAssetId saveIcon1, bool importPersistent)
: mCardPort(cardPort)
, mSaveBanner(saveBanner)
, mSaveIcon0(saveIcon0)
, mSaveIcon1(saveIcon1)
, mState(kS_Initial)
, mError(kE_OK)
, mCardFreeBytes(0)
, mCardFreeFiles(0)
, mFileTime(0)
, mCardSerial(0)
, mSystemData(uchar(0))
, mFileSlots(rstl::auto_ptr< SGameFileSlot >())
, mSaveIdx(gpGameState->SystemOptions().GetSaveIdx())
, mGameOptionsData(3, rstl::reserved_vector< uchar, 32 >(uchar(0)))
, mGlobalGameOptionsData(uchar(0))
, mFileInfo(nullptr)
, x1ac_(false)
, mImportPersistent(importPersistent) {
  sDriverExists = true;
  InitializeFileInfo();
  {
    CMemoryStreamOut output(mSystemData.data(), mSystemData.capacity());
    CBitStreamWriter writer(output);
    gpGameState->SystemOptions().PutTo(writer);
  }
  CGameOptions defaults;
  for (int i = 0; i < 3; ++i) {
    CMemoryStreamOut output(mGameOptionsData[i].data(), mGameOptionsData[i].capacity());
    CBitStreamWriter writer(output);
    defaults.PutTo(writer);
  }
  {
    CMemoryStreamOut output(mGlobalGameOptionsData.data(), mGlobalGameOptionsData.capacity());
    CBitStreamWriter writer(output);
    defaults.PutTo(writer);
  }
}

CMemoryCardDriver::~CMemoryCardDriver() {
  CMemoryCardSys::UnmountCard(mCardPort);
  sDriverExists = false;
  CMemoryCardSys::mIsCardBusy = false;
}

void CMemoryCardDriver::InitializeFileInfo() {
  mFileInfo = rs_new CMemoryCardSys::CCardFileInfo(mCardPort, rstl::string_l(skSaveFileName));
  CMemoryCardSys::CCardFileInfo& file = *mFileInfo;
  file.ResetHeaderInfo();

  char comment[] = "Metroid Prime 2 Echoes          ";
  OSCalendarTime date;
  OSTicksToCalendarTime(OSGetTime(), &date);
  char timestamp[36];
  sprintf(timestamp, "%02d.%02d.%02d  %02d:%02d", date.mon + 1, date.mday, date.year % 100,
          date.hour, date.min);
  file.SetComment(rstl::string_l(comment) + timestamp);
  file.LockBannerToken(mSaveBanner, *gpSimplePool);
  file.LockIconToken(mSaveIcon0, 2, *gpSimplePool);
  file.BuildHeaderBuffer();
}

void CMemoryCardDriver::Update() {
  const ProbeResults probe = CMemoryCardSys::IsMemoryCardInserted(mCardPort);
  if (probe.mError == kCR_NOCARD) {
    if (mState != kS_NoCard) {
      NoCardFound();
    }
    CMemoryCardSys::mIsCardBusy = false;
    return;
  }
  if (mState == kS_CardProbe) {
    UpdateCardProbe();
    CMemoryCardSys::mIsCardBusy = false;
    return;
  }

  const ECardResult result = CMemoryCardSys::GetResultCode(mCardPort);
  bool busy = false;
  if (IsCardBusy(mState)) {
    busy = true;
    switch (mState) {
    case kS_CardProbe:
      break;
    case kS_CardMount:
      UpdateMountCard(result);
      break;
    case kS_CardCheck:
      UpdateCardCheck(result);
      break;
    case kS_FileDeleteBad:
      UpdateFileDeleteBad(result);
      break;
    case kS_FileRead:
      UpdateFileRead(result);
      break;
    case kS_FileCreate:
      UpdateFileCreate(result);
      break;
    case kS_FileWrite:
      UpdateFileWrite(result, kS_Ready, kS_FileWriteFailed);
      break;
    case kS_FileWriteTransactional:
      UpdateFileWrite(result, kS_DriverClosed, kS_FileWriteTransactionalFailed);
      break;
    case kS_CardFormat:
      UpdateCardFormat(result);
      break;
    default:
      break;
    }
  }
  CMemoryCardSys::mIsCardBusy = busy;
}

void CMemoryCardDriver::HandleCardError(ECardResult result, EState state) {
  switch (result) {
  case kCR_BUSY:
    break;
  case kCR_WRONGDEVICE:
    mState = state;
    mError = kE_CardWrongDevice;
    break;
  case kCR_NOCARD:
    NoCardFound();
    break;
  case kCR_IOERROR:
    mState = state;
    mError = kE_CardIOError;
    break;
  case kCR_ENCODING:
    mState = state;
    mError = kE_CardWrongCharacterSet;
    break;
  }
}

void CMemoryCardDriver::UpdateMountCard(ECardResult result) {
  if (result == kCR_READY) {
    mState = kS_CardMountDone;
    StartCardCheck();
  } else if (result == kCR_BROKEN) {
    mState = kS_CardMountDone;
    mError = kE_CardBroken;
    StartCardCheck();
  } else {
    HandleCardError(result, kS_CardMountFailed);
  }
}

void CMemoryCardDriver::UpdateCardCheck(ECardResult result) {
  if (result == kCR_READY) {
    mState = kS_CardCheckDone;
    if (GetCardFreeBytes() && CMemoryCardSys::GetSerialNo(mCardPort, mCardSerial) != kCR_READY) {
      NoCardFound();
    }
  } else if (result == kCR_BROKEN) {
    mState = kS_CardCheckFailed;
    mError = kE_CardBroken;
  } else {
    HandleCardError(result, kS_CardCheckFailed);
  }
}

void CMemoryCardDriver::UpdateFileRead(ECardResult result) {
  if (result == kCR_READY) {
    const ECardResult readResult = mFileInfo->PumpCardRead();
    if (readResult == kCR_READY) {
      mState = kS_Ready;
      if (IsSaveSignatureInvalid(mFileInfo->LoadedData().data())) {
        mState = kS_FileBad;
        mError = kE_FileCorrupted;
      } else {
        ReadFinished();
      }
    } else if (readResult == kCR_BUSY) {
      return;
    } else if (readResult == kCR_CRC_MISMATCH) {
      mState = kS_FileBad;
      mError = kE_FileCorrupted;
    }
  } else {
    HandleCardError(result, kS_FileBad);
  }
}

void CMemoryCardDriver::UpdateFileDeleteBad(ECardResult result) {
  if (result == kCR_READY) {
    mState = kS_CardCheckDone;
    if (GetCardFreeBytes()) {
      IndexFiles();
    }
  } else {
    HandleCardError(result, kS_FileDeleteBadFailed);
  }
}

void CMemoryCardDriver::UpdateFileCreate(ECardResult result) {
  if (result == kCR_READY) {
    mState = kS_FileCreateDone;
    StartFileWrite();
  } else {
    HandleCardError(result, kS_FileCreateFailed);
  }
}

void CMemoryCardDriver::UpdateFileWrite(ECardResult result, EState successState,
                                        EState errorState) {
  if (result == kCR_READY) {
    const ECardResult writeResult = mFileInfo->PumpCardTransfer();
    if (writeResult == kCR_READY) {
      mState = successState;
      if (successState == kS_DriverClosed) {
        WriteBackupBuf();
      }
    } else if (writeResult != kCR_BUSY) {
      if (writeResult == kCR_IOERROR) {
        mState = kS_FileWriteFailed;
        mError = kE_CardIOError;
      } else {
        NoCardFound();
      }
    }
  } else {
    HandleCardError(result, errorState);
  }
}

void CMemoryCardDriver::WriteBackupBuf() {
  const int idx = gpGameState->SystemOptions().GetSaveIdx();
  if (!mFileSlots[idx].null()) {
    gpGameState->CopyCompressedGameState(idx, mFileSlots[idx]->mSaveBuffer.data());
  }
  gpGameState->SetCardSerial(mCardSerial);
}

void CMemoryCardDriver::UpdateCardFormat(ECardResult result) {
  if (result == kCR_READY) {
    mState = kS_CardFormatted;
  } else if (result == kCR_BROKEN) {
    mState = kS_CardFormatFailed;
    mError = kE_CardIOError;
  } else {
    HandleCardError(result, kS_CardFormatFailed);
  }
}

void CMemoryCardDriver::StartCardProbe() {
  mState = kS_CardProbe;
  mError = kE_OK;
  UpdateCardProbe();
}

void CMemoryCardDriver::UpdateCardProbe() {
  const ProbeResults result = CMemoryCardSys::IsMemoryCardInserted(mCardPort);
  if (result.mError == kCR_READY) {
    if (result.mSectorSize != 0x2000) {
      mState = kS_CardProbeFailed;
      mError = kE_CardNon8KSectors;
      return;
    }
  } else {
    if (result.mError == kCR_BUSY) {
      return;
    }
    if (result.mError == kCR_WRONGDEVICE) {
      mState = kS_CardProbeFailed;
      mError = kE_CardWrongDevice;
    } else {
      NoCardFound();
    }
    return;
  }
  mState = kS_CardProbeDone;
  StartMountCard();
}

void CMemoryCardDriver::StartMountCard() {
  mState = kS_CardMount;
  mError = kE_OK;
  const ECardResult result = CMemoryCardSys::MountCard(mCardPort);
  if (result != kCR_READY) {
    UpdateMountCard(result);
  }
}

void CMemoryCardDriver::StartCardCheck() {
  mError = kE_OK;
  mState = kS_CardCheck;
  const ECardResult result = CMemoryCardSys::CheckCard(mCardPort);
  if (result != kCR_READY) {
    UpdateCardCheck(result);
  }
}

void CMemoryCardDriver::NoCardFound() {
  mState = kS_NoCard;
  CMemoryCardSys::mIsCardBusy = false;
}

void CMemoryCardDriver::IndexFiles() {
  mError = kE_OK;
  const ECardResult result = mFileInfo->Open();
  if (result == kCR_NOFILE) {
    mError = kE_FileMissing;
    mState = kS_FileBad;
  } else if (result == kCR_READY) {
    CardStat stat;
    if (CMemoryCardSys::GetStatus(mCardPort, mFileInfo->GetFileNo(), stat) == kCR_READY) {
      if (stat.GetCommentAddr() == -1) {
        mError = kE_FileCorrupted;
        mState = kS_FileBad;
      } else {
        StartFileRead();
      }
    } else {
      NoCardFound();
    }
  } else {
    NoCardFound();
  }
}

void CMemoryCardDriver::StartFileDeleteBad() {
  mError = kE_OK;
  mState = kS_FileDeleteBad;
  const ECardResult result = CMemoryCardSys::FastDeleteFile(mCardPort, mFileInfo->GetFileNo());
  if (result != kCR_READY) {
    UpdateFileDeleteBad(result);
  }
}

void CMemoryCardDriver::StartFileRead() {
  mError = kE_OK;
  mState = kS_FileRead;
  const ECardResult result = mFileInfo->StartRead();
  if (result != kCR_READY) {
    UpdateFileRead(result);
  }
}

void CMemoryCardDriver::StartFileCreate() {
  mError = kE_OK;
  mState = kS_FileCreate;
  BuildSaveBuffer();
  const ECardResult result = mFileInfo->CreateFile();
  if (result != kCR_READY) {
    if (result == kCR_INSSPACE || result == kCR_NOENT) {
      mState = kS_FileCreateFailed;
      mError = kE_CardFull;
    } else {
      UpdateFileCreate(result);
    }
  }
}

void CMemoryCardDriver::StartFileWrite() {
  mError = kE_OK;
  mState = kS_FileWrite;
  const ECardResult result = mFileInfo->WriteFile();
  if (result != kCR_READY) {
    UpdateFileWrite(result, kS_Ready, kS_FileWriteFailed);
  }
}

void CMemoryCardDriver::StartFileWriteTransactional() {
  mError = kE_OK;
  mState = kS_FileWriteTransactional;
  BuildSaveBuffer();
  const ECardResult result = mFileInfo->WriteFile();
  if (result != kCR_READY) {
    UpdateFileWrite(result, kS_DriverClosed, kS_FileWriteTransactionalFailed);
  }
}

void CMemoryCardDriver::StartCardFormat() {
  mError = kE_OK;
  mState = kS_CardFormat;
  const ECardResult result = CMemoryCardSys::FormatCard(mCardPort);
  if (result != kCR_READY) {
    UpdateCardFormat(result);
  }
}

// Guessed name
void CMemoryCardDriver::BuildSaveBuffer() {
  ExportPersistentOptions();
  ExportGameOptions();

  rstl::vector< uchar >& buffer = mFileInfo->SaveBuffer();
  buffer.resize(0x1ff8, uchar(0));
  CMemoryStreamOut output(buffer.data(), 0x1ff8);
  SSaveHeader header(GetSaveSignature(), mSaveIdx);
  for (int i = 0; i < 3; ++i) {
    header.SetSavePresent(i, !mFileSlots[i].null());
  }
  header.PutTo(output);
  output.Put(mSystemData.data(), mSystemData.capacity());
  for (int i = 0; i < 3; ++i) {
    output.Put(mGameOptionsData[i].data(), mGameOptionsData[i].capacity());
  }
  output.Put(mGlobalGameOptionsData.data(), mGlobalGameOptionsData.capacity());
  for (rstl::reserved_vector< rstl::auto_ptr< SGameFileSlot >, 3 >::const_iterator it =
           mFileSlots.begin();
       it != mFileSlots.end(); ++it) {
    if (!it->null()) {
      (*it)->PutTo(output);
    }
  }
}

void CMemoryCardDriver::ReadFinished() {
  CardStat stat;
  if (CMemoryCardSys::GetStatus(mCardPort, mFileInfo->GetFileNo(), stat) != kCR_READY) {
    NoCardFound();
    return;
  }
  mFileTime = stat.GetTime();
  CMemoryInStream input(mFileInfo->LoadedData().data(), 0x1ff8);
  SSaveHeader header(input);
  mSaveIdx = header.mSaveIdx;
  input.Get(mSystemData.data(), mSystemData.capacity());
  for (int i = 0; i < 3; ++i) {
    input.Get(mGameOptionsData[i].data(), mGameOptionsData[i].capacity());
  }
  input.Get(mGlobalGameOptionsData.data(), mGlobalGameOptionsData.capacity());
  for (int i = 0; i < 3; ++i) {
    rstl::auto_ptr< SGameFileSlot >& slot = mFileSlots[i];
    if (header.mSavePresent[i]) {
      slot = rstl::auto_ptr< SGameFileSlot >(rs_new SGameFileSlot(input));
    } else {
      slot = rstl::auto_ptr< SGameFileSlot >();
    }
  }
  if (mImportPersistent) {
    ImportPersistentOptions();
    ImportGameOptions();
  }
}

void CMemoryCardDriver::EraseFileSlot(int idx) {
  mFileSlots[idx] = rstl::auto_ptr< SGameFileSlot >();
  CGameOptions options;
  {
    CMemoryStreamOut output(mGameOptionsData[idx].data(), mGameOptionsData[idx].capacity());
    CBitStreamWriter writer(output);
    options.PutTo(writer);
  }
  gpGameState->CopyCompressedGameOptions(idx, mGameOptionsData[idx].data());
  if (idx == gpGameState->SystemOptions().GetSaveIdx()) {
    gpGameState->GameOptions() = options;
  }
}

// Guessed name
void CMemoryCardDriver::CopyFileSlot(int from, int to) {
  {
    CMemoryInStream input(mFileSlots[from]->mSaveBuffer.data(),
                          mFileSlots[from]->mSaveBuffer.capacity());
    mFileSlots[to] = rstl::auto_ptr< SGameFileSlot >(rs_new SGameFileSlot(input));
  }
  gpGameState->CopyCompressedGameOptions(to, gpGameState->GetCompressedGameOptions(from).data());
  mGameOptionsData[to] = mGameOptionsData[from];
}

void CMemoryCardDriver::BuildNewFileSlot(int idx) {
  if (mFileSlots[idx].null()) {
    mFileSlots[idx] = rstl::auto_ptr< SGameFileSlot >(rs_new SGameFileSlot());
  }
  for (int i = 0; i < 3; ++i) {
    if (!mFileSlots[i].null()) {
      gpGameState->CopyCompressedGameState(i, mFileSlots[i]->mSaveBuffer.data());
    } else {
      gpGameState->ClearCompressedGameState(i);
    }
  }
  {
    CMemoryInStream input(mSystemData.data(), mSystemData.capacity());
    gpGameState->ReadSystemOptions(input);
  }
  gpGameState->SystemOptions().SetSaveIdx(idx);
  ImportPersistentOptions();
  ImportGameOptions();
  gpGameState->SetCardSerial(mCardSerial);
}

void CMemoryCardDriver::BuildExistingFileSlot(int idx) {
  for (int i = 0; i < 3; ++i) {
    const rstl::vector< uchar >& state = gpGameState->GetCompressedGameStates()[i];
    if (!state.empty()) {
      CMemoryInStream input(state.data(), 0xa38);
      mFileSlots[i] = rstl::auto_ptr< SGameFileSlot >(rs_new SGameFileSlot(input));
    } else {
      mFileSlots[i] = rstl::auto_ptr< SGameFileSlot >();
    }
  }
  ExportGameOptions();
  gpGameState->SystemOptions().SetSaveIdx(idx);
  rstl::auto_ptr< SGameFileSlot >& slot = mFileSlots[idx];
  if (slot.null()) {
    slot = rstl::auto_ptr< SGameFileSlot >(rs_new SGameFileSlot());
  } else {
    slot->InitializeFromGameState();
  }
  CMemoryStreamOut output(mSystemData.data(), mSystemData.capacity());
  gpGameState->WriteSystemOptions(output);
  mSaveIdx = gpGameState->SystemOptions().GetSaveIdx();
}

void CMemoryCardDriver::ImportPersistentOptions() {
  CMemoryInStream stream(mSystemData.data(), mSystemData.capacity());
  CBitStreamReader reader(stream);
  CPersistentOptions options(reader);
  gpGameState->SetSystemOptions(options);
}

// Guessed name
void CMemoryCardDriver::ImportGameOptions() {
  for (int i = 0; i < 3; ++i) {
    gpGameState->CopyCompressedGameOptions(i, mGameOptionsData[i].data());
  }
  gpGameState->CopyCompressedMultiplayerOptions(mGlobalGameOptionsData.data());
}

void CMemoryCardDriver::ExportPersistentOptions() {
  CMemoryInStream input(mSystemData.data(), mSystemData.capacity());
  CBitStreamReader reader(input);
  CPersistentOptions options(reader);
  gpGameState->ExportPersistentOptions(options);
  mSaveIdx = options.GetSaveIdx();

  CMemoryStreamOut output(mSystemData.data(), mSystemData.capacity());
  CBitStreamWriter writer(output);
  options.PutTo(writer);
}

// Guessed name
void CMemoryCardDriver::ExportGameOptions() {
  for (int i = 0; i < 3; ++i) {
    CMemoryStreamOut output(mGameOptionsData[i].data(), mGameOptionsData[i].capacity());
    const rstl::vector< uchar >& gameOptions = gpGameState->GetCompressedGameOptions()[i];
    output.Put(gameOptions.data(), gameOptions.size());
  }

  CMemoryStreamOut output(mGlobalGameOptionsData.data(), mGlobalGameOptionsData.capacity());
  const rstl::vector< uchar >& multiplayerOptions = gpGameState->GetCompressedMultiplayerOptions();
  output.Put(multiplayerOptions.data(), multiplayerOptions.size());
}

// Guessed name
bool CMemoryCardDriver::IsRepairingHeader() const { return mFileInfo->IsRepairingHeader(); }

SSaveHeader::SSaveHeader(uint signature, int saveIdx) : mSignature(signature), mSaveIdx(saveIdx) {}

SSaveHeader::SSaveHeader(CInputStream& in) : mSignature(in.ReadInt32()), mSaveIdx(in.ReadInt32()) {
  for (int i = 0; i < 3; ++i) {
    mSavePresent[i] = in.ReadBool();
  }
  in.ReadInt32(); // Trailing SAVH marker.
}

void SSaveHeader::PutTo(COutputStream& out) const {
  out.WriteUint32(mSignature);
  out.WriteInt32(mSaveIdx);
  for (int i = 0; i < 3; ++i) {
    out.WriteBool(mSavePresent[i]);
  }
  out.WriteUint32('SAVH');
}

SGameFileSlot::SGameFileSlot() : mSaveBuffer(uchar(0)) {
  CMemoryStreamOut stream(mSaveBuffer.data(), mSaveBuffer.capacity());
  CBitStreamWriter writer(stream);
  CGameState::SerializeNewForCleanSlot(writer, gpGameState->GetHardModeEnabled());
}

SGameFileSlot::SGameFileSlot(CInputStream& in) : mSaveBuffer(uchar(0)) {
  in.Get(mSaveBuffer.data(), mSaveBuffer.capacity());
  mFileInfo = CGameState::LoadGameFileState(mSaveBuffer.data());
}

void SGameFileSlot::PutTo(COutputStream& out) const {
  out.Put(mSaveBuffer.data(), mSaveBuffer.capacity());
}

void SGameFileSlot::InitializeFromGameState() {
  {
    CMemoryStreamOut stream(mSaveBuffer.data(), mSaveBuffer.capacity());
    CBitStreamWriter writer(stream);
    gpGameState->PutTo(writer);
  }
  mFileInfo = CGameState::LoadGameFileState(mSaveBuffer.data());
}

const CGameState::GameFileStateInfo* CMemoryCardDriver::GetGameFileStateInfo(int idx) {
  return mFileSlots[idx].null() ? nullptr : &mFileSlots[idx]->mFileInfo;
}

bool CMemoryCardDriver::GetCardFreeBytes() {
  const ECardResult result =
      CMemoryCardSys::GetNumFreeBytes(mCardPort, mCardFreeBytes, mCardFreeFiles);
  if (result != kCR_READY) {
    NoCardFound();
    return false;
  }
  return true;
}
