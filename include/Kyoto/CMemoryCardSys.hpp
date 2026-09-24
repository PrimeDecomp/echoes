#ifndef _CMEMORYCARDSYS
#define _CMEMORYCARDSYS

#include "string.h"
#include "types.h"

#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Streams/CMemoryStreamOut.hpp"
#include "Kyoto/TToken.hpp"

#include "dolphin/card.h"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CTexture;

// Dolphin SDK result codes, with Retro's CRC mismatch extension.
enum ECardResult {
  kCR_CRC_MISMATCH = -1003, /* Extension enum for Retro's CRC check */
  kCR_FATAL_ERROR = -128,
  kCR_ENCODING = -13,
  kCR_NAMETOOLONG = -12,
  kCR_INSSPACE = -9,
  kCR_NOENT = -8,
  kCR_EXIST = -7,
  kCR_BROKEN = -6,
  kCR_IOERROR = -5,
  kCR_NOFILE = -4,
  kCR_NOCARD = -3,
  kCR_WRONGDEVICE = -2,
  kCR_BUSY = -1,
  kCR_READY = 0
};

struct ProbeResults {
  ECardResult x0_error;
  s32 x4_cardSize;   /* in megabits */
  s32 x8_sectorSize; /* in bytes */
};

struct CardStat {
  CARDStat x0_stat;

  CardStat() { memset(this, 0, sizeof(CardStat)); }

  uint GetFileLength();
  int GetTime() const;
  int GetCommentAddr() const;
  void SetBannerFormat(int format);
  void SetIconFormat(int format, int idx);
  void SetIconSpeed(int speed, int idx);
  void SetIconAddr(int addr);
  void SetCommentAddr(int addr);
};
CHECK_SIZEOF(CardStat, 0x6C);

class CMemoryCardSys {
public:
  enum EMemoryCardPort { kCS_SlotA, kCS_SlotB };

  // Echoes stores a 0x2000-byte header block followed by two alternating copies of the save data.
  class CCardFileInfo {
    struct Icon {
      CAssetId x0_id;
      int x4_speed;
      TLockedToken< CTexture > x8_tex;

      Icon(CAssetId id, int speed, CSimplePool& pool);
    };

    struct SSaveSlot {
      int x0_generation;
      bool x4_corrupt;
      rstl::vector< uchar, rstl::aligned_allocator > x8_data;

      SSaveSlot();
      void CheckCrc();
    };

    enum EStatus {
      kS_Standby,
      kS_ReadHeader,
      kS_RepairHeader,
      kS_ReadSlotA,
      kS_ReadSlotB,
      kS_WriteHeader,
      kS_WriteSlot,
      kS_WriteFirstSlot,
      kS_SetStatus,
    };

    int x0_slotBlocks;
    int x4_slotSize;
    int x8_slot;
    int xc_generation;
    EStatus x10_status;
    bool x14_newFile;
    CARDFileInfo x18_fileInfo;
    rstl::string x2c_fileName;
    rstl::string x3c_comment;
    int x4c_;
    CAssetId x50_bannerTex;
    rstl::optional_object< TLockedToken< CTexture > > x54_bannerTok;
    rstl::reserved_vector< Icon, 8 > x64_iconToks;
    rstl::vector< uchar, rstl::aligned_allocator > x108_headerBuffer;
    rstl::reserved_vector< SSaveSlot, 2 > x118_slots;
    rstl::vector< uchar > x14c_loadedData;
    rstl::vector< uchar > x15c_saveBuffer;

  public:
    CCardFileInfo(EMemoryCardPort port, const rstl::string& name);
    ~CCardFileInfo();

    void SetComment(const rstl::string& comment);
    void ResetHeaderInfo();
    void LockBannerToken(CAssetId bannerTxtr, CSimplePool& pool);
    void LockIconToken(CAssetId iconTxtr, int speed, CSimplePool& pool);

    ECardResult Open();
    ECardResult CreateFile();
    ECardResult StartRead();
    ECardResult PumpCardRead();
    ECardResult WriteFile();
    ECardResult PumpCardTransfer();
    ECardResult GetStatus(CardStat& stat);
    EMemoryCardPort GetCardPort();
    int GetFileNo();
    int GetFileBlocks();

    rstl::vector< uchar >& LoadedData() { return x14c_loadedData; }
    rstl::vector< uchar >& SaveBuffer() { return x15c_saveBuffer; }

  private:
    ECardResult CheckHeaderCrc();
    ECardResult SelectSaveSlot();
    void BuildSaveSlot();
    void BuildHeaderBuffer();
    void WriteBannerData(COutputStream& out);
    void WriteIconData(COutputStream& out);
    ECardResult WriteSaveSlot(int slot);
  };

  CMemoryCardSys();
  ~CMemoryCardSys();

  static ECardResult GetResultCode(int);
  static ECardResult MountCard(EMemoryCardPort port);
  static ECardResult CheckCard(EMemoryCardPort port);
  static ECardResult GetStatus(EMemoryCardPort port, int fileNo, CardStat& statOut);
  static ECardResult SetStatus(EMemoryCardPort port, int fileNo, const CardStat& stat);
  static ECardResult FastDeleteFile(EMemoryCardPort port, int fileNo);
  static ECardResult FormatCard(EMemoryCardPort port);

  static ProbeResults IsMemoryCardInserted(EMemoryCardPort);
  static ECardResult GetSerialNo(EMemoryCardPort port, long long& serialOut);
  static ECardResult UnmountCard(EMemoryCardPort);
  static ECardResult GetNumFreeBytes(EMemoryCardPort port, uint& freeBytes, uint& freeFiles);
  static rstl::vector< char, rstl::aligned_allocator >& WorkAreaVector(EMemoryCardPort port);
  static char* AllocCardWorkArea(EMemoryCardPort port);
  static void FreeCardWorkArea(EMemoryCardPort port);
  inline void Initialize();

  static bool mIsCardBusy;

private:
  static bool mIsInitialized;
  static bool mIsCardSysExists;
  static rstl::vector< char, rstl::aligned_allocator > mWorkAreaA;
  static rstl::vector< char, rstl::aligned_allocator > mWorkAreaB;
};

NESTED_CHECK_SIZEOF(CMemoryCardSys, CCardFileInfo, 0x16C)

#endif // _CMEMORYCARDSYS
