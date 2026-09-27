#include "MetroidPrime/CMemoryCardDriver.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"

// This TU is a scaffold. Save serialization and option synchronization remain incomplete.
static bool sDriverExists; // Guessed name

// Guessed name
static uint GetSaveSignature() {
  // TODO: Cache the USA seed XOR the signature of each world's SAVW resource.
  return 0;
}

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
, mSaveIdx(0)
, mGameOptionsData(rstl::reserved_vector< uchar, 32 >(uchar(0)))
, mGlobalGameOptionsData(uchar(0))
, mFileInfo(nullptr)
, x1ac_(false)
, mImportPersistent(importPersistent) {
  sDriverExists = true;
  InitializeFileInfo();
  // TODO: Read the selected save index from persistent options and serialize the
  // system options and default CGameOptions into their respective bitstream buffers.
}

CMemoryCardDriver::~CMemoryCardDriver() {
  CMemoryCardSys::UnmountCard(mCardPort);
  sDriverExists = false;
  CMemoryCardSys::mIsCardBusy = false;
}

void CMemoryCardDriver::InitializeFileInfo() {
  // TODO: Create the card-file object, timestamp its comment, and prepare its
  // banner/icon header. Ownership is held by mFileInfo, not Prime's two-file array.
}

void CMemoryCardDriver::Update() {
  // TODO: Probe card removal, dispatch the active operation, and publish card-busy state.
}

void CMemoryCardDriver::HandleCardError(ECardResult result, EState state) {
  switch (result) {
  case kCR_ENCODING:
    mState = state;
    mError = kE_CardWrongCharacterSet;
    break;
  case kCR_IOERROR:
    mState = state;
    mError = kE_CardIOError;
    break;
  case kCR_WRONGDEVICE:
    mState = state;
    mError = kE_CardWrongDevice;
    break;
  case kCR_NOCARD:
    NoCardFound();
    break;
  default:
    break;
  }
}

void CMemoryCardDriver::UpdateMountCard(ECardResult result) {
  if (result == kCR_READY || result == kCR_BROKEN) {
    mState = kS_CardMountDone;
    if (result == kCR_BROKEN) {
      mError = kE_CardBroken;
    }
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
  if (result != kCR_READY) {
    HandleCardError(result, kS_FileBad);
    return;
  }

  result = mFileInfo->PumpCardRead();
  if (result == kCR_READY) {
    mState = kS_Ready;
    if (IsSaveSignatureInvalid(mFileInfo->LoadedData().data())) {
      mState = kS_FileBad;
      mError = kE_FileCorrupted;
    } else {
      ReadFinished();
    }
  } else if (result == kCR_CRC_MISMATCH) {
    mState = kS_FileBad;
    mError = kE_FileCorrupted;
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
  // TODO: Pump the card-file transfer, select the requested terminal state and
  // back up the active game when a transactional write completes.
}

void CMemoryCardDriver::WriteBackupBuf() {
  // TODO: Copy the selected slot to the in-memory game backup and record the card serial.
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
    } else {
      mState = kS_CardProbeDone;
      StartMountCard();
    }
  } else if (result.mError == kCR_WRONGDEVICE) {
    mState = kS_CardProbeFailed;
    mError = kE_CardWrongDevice;
  } else if (result.mError != kCR_BUSY) {
    NoCardFound();
  }
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
  mState = kS_CardCheck;
  mError = kE_OK;
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
  // TODO: Open the single Echoes save file, validate its comment header and start reading.
}

void CMemoryCardDriver::StartFileDeleteBad() {
  mState = kS_FileDeleteBad;
  mError = kE_OK;
  const ECardResult result = CMemoryCardSys::FastDeleteFile(mCardPort, mFileInfo->GetFileNo());
  if (result != kCR_READY) {
    UpdateFileDeleteBad(result);
  }
}

void CMemoryCardDriver::StartFileRead() {
  mState = kS_FileRead;
  mError = kE_OK;
  const ECardResult result = mFileInfo->StartRead();
  if (result != kCR_READY) {
    UpdateFileRead(result);
  }
}

void CMemoryCardDriver::StartFileCreate() {
  // TODO: Build the save buffer and create the file, handling both capacity errors.
}

void CMemoryCardDriver::StartFileWrite() {
  mState = kS_FileWrite;
  mError = kE_OK;
  const ECardResult result = mFileInfo->WriteFile();
  if (result != kCR_READY) {
    UpdateFileWrite(result, kS_Ready, kS_FileWriteFailed);
  }
}

void CMemoryCardDriver::StartFileWriteTransactional() {
  mState = kS_FileWriteTransactional;
  mError = kE_OK;
  BuildSaveBuffer();
  const ECardResult result = mFileInfo->WriteFile();
  if (result != kCR_READY) {
    UpdateFileWrite(result, kS_DriverClosed, kS_FileWriteTransactionalFailed);
  }
}

void CMemoryCardDriver::StartCardFormat() {
  mState = kS_CardFormat;
  mError = kE_OK;
  const ECardResult result = CMemoryCardSys::FormatCard(mCardPort);
  if (result != kCR_READY) {
    UpdateCardFormat(result);
  }
}

// Guessed name
void CMemoryCardDriver::BuildSaveBuffer() {
  // TODO: Export options, then write the save header, option buffers and occupied slots.
}

void CMemoryCardDriver::ReadFinished() {
  // TODO: Record file time and deserialize the header, option buffers and present
  // game slots; import global options when mImportPersistent is set.
}

void CMemoryCardDriver::EraseFileSlot(int idx) {
  // TODO: Release the slot, reset its game options and begin the appropriate card write.
}

// Guessed name
void CMemoryCardDriver::CopyFileSlot(int from, int to) {
  // TODO: Copy both slot contents and their game options, then start the card write.
}

void CMemoryCardDriver::BuildNewFileSlot(int idx) {
  // TODO: Allocate a clean game slot and serialize its current per-game options.
}

void CMemoryCardDriver::BuildExistingFileSlot(int idx) {
  // TODO: Refresh the selected slot from the current game and serialize its options.
}

void CMemoryCardDriver::ImportPersistentOptions() {
  // TODO: Decode mSystemData through CBitStreamReader and install the system options.
}

// Guessed name
void CMemoryCardDriver::ImportGameOptions() {
  // TODO: Decode the global game-options buffer and install it in the current game.
}

void CMemoryCardDriver::ExportPersistentOptions() {
  // TODO: Merge persistent state and serialize it to mSystemData through CBitStreamWriter.
}

// Guessed name
void CMemoryCardDriver::ExportGameOptions() {
  // TODO: Serialize the current global game options to mGlobalGameOptionsData.
}

// Guessed name
bool CMemoryCardDriver::IsRepairingHeader() const {
  // TODO: Expose the card-file status query through CCardFileInfo's typed interface.
  return false;
}

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
  // TODO: Serialize a clean game using the current hard-mode setting. The target
  // does not populate mFileInfo until the slot is read or refreshed.
}

SGameFileSlot::SGameFileSlot(CInputStream& in) : mSaveBuffer(uchar(0)) {
  in.Get(mSaveBuffer.data(), mSaveBuffer.size());
  mFileInfo = CGameState::LoadGameFileState(mSaveBuffer.data());
}

void SGameFileSlot::PutTo(COutputStream& out) const {
  out.Put(mSaveBuffer.data(), mSaveBuffer.size());
}

void SGameFileSlot::InitializeFromGameState() {
  // TODO: Serialize the current game through its bitstream interface, then refresh mFileInfo.
}

const CGameState::GameFileStateInfo* CMemoryCardDriver::GetGameFileStateInfo(int idx) {
  return mFileSlots[idx].null() ? nullptr : &mFileSlots[idx]->mFileInfo;
}

bool CMemoryCardDriver::GetCardFreeBytes() {
  const ECardResult result =
      CMemoryCardSys::GetNumFreeBytes(mCardPort, mCardFreeBytes, mCardFreeFiles);
  if (result != kCR_READY) {
    NoCardFound();
  }
  return result == kCR_READY;
}
