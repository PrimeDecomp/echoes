#ifndef _CMEMORYCARDDRIVER
#define _CMEMORYCARDDRIVER

#include "Kyoto/CMemoryCardSys.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

struct SSaveHeader {
  uint mSignature;
  int mSaveIdx;
  bool mSavePresent[3];

  SSaveHeader(uint signature, int saveIdx);
  explicit SSaveHeader(CInputStream& in);
  void SetSavePresent(int idx, const bool present) { mSavePresent[idx] = present; }
  void PutTo(COutputStream& out) const;
};
CHECK_SIZEOF(SSaveHeader, 0xc)

struct SGameFileSlot {
  rstl::reserved_vector< uchar, 0xa38 > mSaveBuffer;
  CGameState::GameFileStateInfo mFileInfo;

  SGameFileSlot();
  explicit SGameFileSlot(CInputStream& in);
  void PutTo(COutputStream& out) const;
  void InitializeFromGameState();
};
CHECK_SIZEOF(SGameFileSlot, 0xa68)

enum EState {
  kS_Initial = 0,
  kS_Ready = 1,
  kS_NoCard = 2,
  kS_DriverClosed = 3,
  kS_CardFormatted = 4,
  kS_CardProbeDone = 5,
  kS_CardMountDone = 6,
  kS_CardCheckDone = 7,
  kS_FileCreateDone = 8,
  kS_CardProbeFailed = 10,
  kS_CardMountFailed = 11,
  kS_CardCheckFailed = 12,
  kS_FileDeleteBadFailed = 13,
  kS_FileBad = 14,
  kS_FileCreateFailed = 15,
  kS_FileWriteFailed = 16,
  kS_FileWriteTransactionalFailed = 17,
  kS_CardFormatFailed = 18,
  kS_CardProbe = 19,
  kS_CardMount = 20,
  kS_CardCheck = 21,
  kS_FileDeleteBad = 22,
  kS_FileRead = 23,
  kS_FileCreate = 24,
  kS_FileWrite = 25,
  kS_FileWriteTransactional = 26,
  kS_CardFormat = 27
};

class CMemoryCardDriver {
public:
  enum EError {
    kE_OK,
    kE_CardBroken,
    kE_CardWrongCharacterSet,
    kE_CardIOError,
    kE_CardWrongDevice,
    kE_CardFull,
    kE_CardNon8KSectors,
    kE_FileMissing,
    kE_FileCorrupted
  };

  CMemoryCardDriver(CMemoryCardSys::EMemoryCardPort cardPort, CAssetId saveBanner,
                    CAssetId saveIcon0, CAssetId saveIcon1, bool importPersistent);
  ~CMemoryCardDriver();

  static bool IsCardBusy(EState state);
  static bool IsCardReading(EState state);
  EState GetState() const { return mState; }
  EError GetError() const { return mError; }
  u64 GetCardSerial() const { return mCardSerial; }
  int GetSaveIdx() const { return mSaveIdx; }

  void InitializeFileInfo();
  void Update();
  void HandleCardError(ECardResult result, EState state);
  void UpdateMountCard(ECardResult result);
  void UpdateCardCheck(ECardResult result);
  void UpdateFileRead(ECardResult result);
  void UpdateFileDeleteBad(ECardResult result);
  void UpdateFileCreate(ECardResult result);
  void UpdateFileWrite(ECardResult result, EState successState, EState errorState);
  void WriteBackupBuf();
  void UpdateCardFormat(ECardResult result);
  void StartCardProbe();
  void UpdateCardProbe();
  void StartMountCard();
  void StartCardCheck();
  void NoCardFound();
  void IndexFiles();
  void StartFileDeleteBad();
  void StartFileRead();
  void StartFileCreate();
  void StartFileWrite();
  void StartFileWriteTransactional();
  void StartCardFormat();
  void BuildSaveBuffer(); // Guessed name
  void ReadFinished();
  void EraseFileSlot(int idx);
  void CopyFileSlot(int from, int to); // Guessed name
  void BuildNewFileSlot(int idx);
  void BuildExistingFileSlot(int idx);
  void ImportPersistentOptions();
  void ImportGameOptions(); // Guessed name
  void ExportPersistentOptions();
  void ExportGameOptions();       // Guessed name
  bool IsRepairingHeader() const; // Guessed name
  const CGameState::GameFileStateInfo* GetGameFileStateInfo(int idx);
  bool GetCardFreeBytes();

private:
  CMemoryCardSys::EMemoryCardPort mCardPort;
  CAssetId mSaveBanner;
  CAssetId mSaveIcon0;
  CAssetId mSaveIcon1;
  EState mState;
  EError mError;
  uint mCardFreeBytes;
  uint mCardFreeFiles;
  uint mFileTime;
  long long mCardSerial;
  rstl::reserved_vector< uchar, 192 > mSystemData;
  rstl::reserved_vector< rstl::auto_ptr< SGameFileSlot >, 3 > mFileSlots;
  int mSaveIdx;
  rstl::reserved_vector< rstl::reserved_vector< uchar, 32 >, 3 > mGameOptionsData;
  rstl::reserved_vector< uchar, 32 > mGlobalGameOptionsData; // Guessed name
  rstl::single_ptr< CMemoryCardSys::CCardFileInfo > mFileInfo;
  bool x1ac_;
  bool mImportPersistent;
};
CHECK_SIZEOF(CMemoryCardDriver, 0x1b0)

#endif // _CMEMORYCARDDRIVER
