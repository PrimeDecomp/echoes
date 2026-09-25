#include "Kyoto/CMemoryCardSys.hpp"

#include "Kyoto/CCRC32.hpp"
#include "Kyoto/Graphics/CGraphicsPalette.hpp"
#include "Kyoto/Graphics/CTexture.hpp"

#include "dolphin/os/OSCache.h"

#include "Kyoto/MemoryCopy.hpp"

static uint RoundUp(uint value, uint align) { return (value + align - 1) & ~(align - 1); }

CMemoryCardSys::CCardFileInfo::Icon::Icon(CAssetId id, int speed, CSimplePool& pool)
: mId(id), mSpeed(speed), mTex(pool.GetObj(SObjectTag('TXTR', id))) {}

CMemoryCardSys::CCardFileInfo::SSaveSlot::SSaveSlot() : mGeneration(0), mCorrupt(false) {}

void CMemoryCardSys::CCardFileInfo::SSaveSlot::CheckCrc() {
  const uint* data = reinterpret_cast< const uint* >(mData.data());
  const uint crc = data[0];
  mCorrupt = CCRC32::Calculate(data + 1, mData.size() - 4, 0xFFFFFFFF) != crc;
  mGeneration = data[1];
}

CMemoryCardSys::CCardFileInfo::CCardFileInfo(EMemoryCardPort port, const rstl::string& name)
: mSlotBlocks(0)
, mSlotSize(0)
, mSlot(0)
, mGeneration(0)
, mStatus(kS_Standby)
, mNewFile(false)
, mFileName(name)
, x4c_(0)
, mBannerTex(kInvalidAssetId)
, mHeaderBuffer(0x2000, uchar())
, mSlots(2, SSaveSlot()) {
  mFileInfo.chan = port;
  mFileInfo.fileNo = -1;
}

CMemoryCardSys::CCardFileInfo::~CCardFileInfo() {
  if (mFileInfo.fileNo != -1) {
    CARDClose(&mFileInfo);
  }
}

ECardResult CMemoryCardSys::CCardFileInfo::Open() {
  const EMemoryCardPort port = GetCardPort();
  ECardResult result =
      static_cast< ECardResult >(CARDOpen(port, mFileName.data(), &mFileInfo));
  mFileInfo.chan = port;
  if (result == kCR_READY) {
    CardStat stat;
    ECardResult statResult = CMemoryCardSys::GetStatus(GetCardPort(), GetFileNo(), stat);
    if (statResult != kCR_READY) {
      return statResult;
    }
    const int blocks = stat.GetFileLength() / 0x2000;
    mSlotBlocks = (blocks - 1) / 2;
    mSlotSize = mSlotBlocks * 0x2000;
  }
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::CreateFile() {
  mNewFile = true;
  mSlotSize = RoundUp(mSaveBuffer.size(), 0x2000);
  mSlotBlocks = mSlotSize / 0x2000;
  const int size = GetFileBlocks() * 0x2000;
  return static_cast< ECardResult >(
      CARDCreateAsync(GetCardPort(), mFileName.data(), size, &mFileInfo, nullptr));
}

ECardResult CMemoryCardSys::CCardFileInfo::StartRead() {
  CardStat stat;
  ECardResult result = CMemoryCardSys::GetStatus(GetCardPort(), GetFileNo(), stat);
  if (result != kCR_READY) {
    return result;
  }
  const uint size = stat.GetFileLength();
  mLoadedData = rstl::vector< uchar >();
  mHeaderBuffer.assign(0x2000);
  mSlots[0].mData.assign(mSlotSize);
  mSlots[1].mData.assign(mSlotSize);
  result =
      static_cast< ECardResult >(CARDRead(&mFileInfo, mHeaderBuffer.data(), 0x2000, 0));
  if (result == kCR_READY) {
    mStatus = kS_ReadHeader;
  }
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::PumpCardRead() {
  if (mStatus == kS_Standby) {
    return kCR_READY;
  }
  ECardResult result = GetResultCode(GetCardPort());
  if (result != kCR_READY) {
    return result;
  }
  if (mStatus == kS_ReadHeader || mStatus == kS_RepairHeader) {
    if (mStatus == kS_ReadHeader && CheckHeaderCrc() != kCR_READY) {
      BuildHeaderBuffer();
      mStatus = kS_RepairHeader;
      void* data = mHeaderBuffer.data();
      DCStoreRange(data, 0x2000);
      result = static_cast< ECardResult >(CARDWriteAsync(&mFileInfo, data, 0x2000, 0, nullptr));
      if (result == kCR_READY) {
        return kCR_BUSY;
      }
      return result;
    } else {
      mHeaderBuffer = rstl::vector< uchar, rstl::aligned_allocator >();
      mStatus = kS_ReadSlotA;
      result = static_cast< ECardResult >(
          CARDRead(&mFileInfo, mSlots[0].mData.data(), mSlotSize, 0x2000));
      if (result == kCR_READY) {
        return kCR_BUSY;
      }
      return result;
    }
  } else if (mStatus == kS_ReadSlotA) {
    mStatus = kS_ReadSlotB;
    result = static_cast< ECardResult >(CARDRead(&mFileInfo, mSlots[1].mData.data(),
                                                 mSlotSize, mSlotSize + 0x2000));
    if (result == kCR_READY) {
      return kCR_BUSY;
    }
    return result;
  } else {
    return SelectSaveSlot();
  }
}

ECardResult CMemoryCardSys::CCardFileInfo::WriteSaveSlot(int slot) {
  void* data = mSlots[mSlot].mData.data();
  const int offset = slot * mSlotSize + 0x2000;
  DCStoreRange(data, mSlotSize);
  return static_cast< ECardResult >(
      CARDWriteAsync(&mFileInfo, data, mSlotSize, offset, nullptr));
}

ECardResult CMemoryCardSys::CCardFileInfo::WriteFile() {
  ECardResult result;
  BuildSaveSlot();
  if (mNewFile) {
    BuildHeaderBuffer();
    void* data = mHeaderBuffer.data();
    DCStoreRange(data, 0x2000);
    result = static_cast< ECardResult >(CARDWriteAsync(&mFileInfo, data, 0x2000, 0, nullptr));
    mStatus = kS_WriteHeader;
  } else {
    result = WriteSaveSlot(mSlot);
    mStatus = kS_WriteSlot;
  }
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::PumpCardTransfer() {
  if (mStatus == kS_Standby) {
    return kCR_READY;
  }
  ECardResult result = GetResultCode(GetCardPort());
  if (result != kCR_READY) {
    return result;
  }
  if (mStatus == kS_WriteHeader) {
    mStatus = mNewFile ? kS_WriteFirstSlot : kS_WriteSlot;
    result = WriteSaveSlot(mSlot);
    if (result == kCR_READY) {
      return kCR_BUSY;
    }
    return result;
  } else if (mStatus == kS_WriteFirstSlot) {
    mStatus = kS_WriteSlot;
    result = WriteSaveSlot(1 - mSlot);
    if (result == kCR_READY) {
      return kCR_BUSY;
    }
    return result;
  } else if (mStatus == kS_WriteSlot) {
    mStatus = kS_SetStatus;
    CardStat stat;
    result = GetStatus(stat);
    if (result != kCR_READY) {
      return result;
    }
    result = SetStatus(GetCardPort(), GetFileNo(), stat);
    if (result == kCR_READY) {
      return kCR_BUSY;
    }
    return result;
  } else {
    mStatus = kS_Standby;
    return kCR_READY;
  }
}

CMemoryCardSys::EMemoryCardPort CMemoryCardSys::CCardFileInfo::GetCardPort() {
  return static_cast< EMemoryCardPort >(mFileInfo.chan);
}

int CMemoryCardSys::CCardFileInfo::GetFileNo() { return mFileInfo.fileNo; }

int CMemoryCardSys::CCardFileInfo::GetFileBlocks() { return mSlotBlocks * 2 + 1; }

void CMemoryCardSys::CCardFileInfo::ResetHeaderInfo() {
  mComment = rstl::string();
  mBannerTex = kInvalidAssetId;
  mBannerTok = rstl::optional_object< TLockedToken< CTexture > >();
  mIconToks = rstl::reserved_vector< Icon, 8 >();
}

void CMemoryCardSys::CCardFileInfo::SetComment(const rstl::string& comment) {
  mComment = comment;
}

void CMemoryCardSys::CCardFileInfo::LockBannerToken(CAssetId bannerTxtr, CSimplePool& pool) {
  mBannerTex = bannerTxtr;
  mBannerTok = TLockedToken< CTexture >(pool.GetObj(SObjectTag('TXTR', mBannerTex)));
}

void CMemoryCardSys::CCardFileInfo::LockIconToken(CAssetId iconTxtr, int speed, CSimplePool& pool) {
  mIconToks.push_back(Icon(iconTxtr, speed, pool));
}

ECardResult CMemoryCardSys::CCardFileInfo::GetStatus(CardStat& stat) {
  ECardResult result = CMemoryCardSys::GetStatus(GetCardPort(), GetFileNo(), stat);
  if (result != kCR_READY) {
    return result;
  }
  stat.SetCommentAddr(4);
  stat.SetIconAddr(68);
  int bannerFormat;
  if (mBannerTex == kInvalidAssetId) {
    bannerFormat = CARD_STAT_BANNER_NONE;
  } else if ((*mBannerTok.data())->GetTexelFormat() == kTF_RGB5A3) {
    bannerFormat = CARD_STAT_BANNER_RGB5A3;
  } else {
    bannerFormat = CARD_STAT_BANNER_C8;
  }
  stat.SetBannerFormat(bannerFormat);
  const int count = mIconToks.size();
  int i = 0;
  for (; i < count; ++i) {
    int format = CARD_STAT_ICON_C8;
    if ((*mIconToks[i].mTex)->GetTexelFormat() == kTF_RGB5A3) {
      format = CARD_STAT_ICON_RGB5A3;
    }
    stat.SetIconFormat(format, i);
    stat.SetIconSpeed(mIconToks[i].mSpeed, i);
  }
  if (i < 8) {
    stat.SetIconFormat(CARD_STAT_ICON_NONE, i);
    stat.SetIconSpeed(CARD_STAT_SPEED_END, i);
  }
  return kCR_READY;
}

void CMemoryCardSys::CCardFileInfo::WriteBannerData(COutputStream& out) {
  if (mBannerTex != kInvalidAssetId) {
    const CTexture& texture = **mBannerTok.data();
    const ETexelFormat format = texture.GetTexelFormat();
    const void* data = texture.GetConstBitMapData(0);
    const uint size = format == kTF_RGB5A3 ? 6144 : 3072;
    out.Put(data, size);
    if (format == kTF_C8) {
      out.Put(texture.GetPalette()->GetPaletteData(), 512);
    }
  }
}

void CMemoryCardSys::CCardFileInfo::WriteIconData(COutputStream& out) {
  const void* palette = nullptr;
  const int count = mIconToks.size();
  for (int i = 0; i < count; ++i) {
    const CTexture& texture = **mIconToks[i].mTex;
    const ETexelFormat format = texture.GetTexelFormat();
    const void* data = texture.GetConstBitMapData(0);
    const uint size = format == kTF_RGB5A3 ? 2048 : 1024;
    out.Put(data, size);
    if (format == kTF_C8) {
      palette = texture.GetPalette()->GetPaletteData();
    }
  }
  if (palette != nullptr) {
    out.Put(palette, 512);
  }
}

void CMemoryCardSys::CCardFileInfo::BuildHeaderBuffer() {
  mHeaderBuffer.assign(0x2000);
  uint* data = reinterpret_cast< uint* >(mHeaderBuffer.data());
  {
    CMemoryStreamOut out(data, 0x2000);
    out.WriteInt32(0);
    char comment[64];
    strncpy(comment, mComment.data(), sizeof(comment));
    out.Put(comment, sizeof(comment));
    WriteBannerData(out);
    WriteIconData(out);
  }
  data[0] = CCRC32::Calculate(data + 1, 0x2000 - 4, 0xFFFFFFFF);
}

void CMemoryCardSys::CCardFileInfo::BuildSaveSlot() {
  SSaveSlot& slot = mSlots[mSlot];
  slot.mData.assign(mSlotSize);
  uint* data = reinterpret_cast< uint* >(slot.mData.data());
  (memcpy)(data + 2, mSaveBuffer.data(), mSaveBuffer.size());
  data[1] = mGeneration;
  data[0] = CCRC32::Calculate(data + 1, mSlotSize - 4, 0xFFFFFFFF);
  mSaveBuffer = rstl::vector< uchar >();
}

ECardResult CMemoryCardSys::CCardFileInfo::SelectSaveSlot() {
  SSaveSlot& slotA = mSlots[0];
  SSaveSlot& slotB = mSlots[1];
  slotA.CheckCrc();
  slotB.CheckCrc();

  ECardResult result = kCR_READY;
  int slot = -1;
  if (slotA.mCorrupt) {
    if (slotB.mCorrupt) {
      result = kCR_CRC_MISMATCH;
    } else {
      slot = 1;
      mGeneration = slotB.mGeneration;
    }
  } else if (slotB.mCorrupt) {
    slot = 0;
    mGeneration = 1 - slotA.mGeneration;
  } else {
    slot = slotA.mGeneration ^ slotB.mGeneration;
    mGeneration = slot != 0 ? slotB.mGeneration : 1 - slotA.mGeneration;
  }

  mLoadedData = rstl::vector< uchar >();
  if (slot != -1) {
    mSlot = 1 - slot;
    const int size = mSlotSize - 8;
    mLoadedData.resize(size);
    (memcpy)(mLoadedData.data(), mSlots[slot].mData.data() + 8, size);
  }
  slotA.mData = rstl::vector< uchar, rstl::aligned_allocator >();
  slotB.mData = rstl::vector< uchar, rstl::aligned_allocator >();
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::CheckHeaderCrc() {
  const uint* data = reinterpret_cast< const uint* >(mHeaderBuffer.data());
  const uint crc = data[0];
  return crc == CCRC32::Calculate(data + 1, 0x2000 - 4, 0xFFFFFFFF) ? kCR_READY
                                                                    : kCR_CRC_MISMATCH;
}

uint CardStat::GetFileLength() { return mStat.length; }

int CardStat::GetTime() const { return mStat.time; }

int CardStat::GetCommentAddr() const { return mStat.commentAddr; }

void CardStat::SetBannerFormat(int format) { CARDSetBannerFormat(&mStat, format); }

void CardStat::SetIconFormat(int format, int idx) { CARDSetIconFormat(&mStat, idx, format); }

void CardStat::SetIconSpeed(int speed, int idx) { CARDSetIconSpeed(&mStat, idx, speed); }

void CardStat::SetIconAddr(int addr) { CARDSetIconAddress(&mStat, addr); }

void CardStat::SetCommentAddr(int addr) { CARDSetCommentAddress(&mStat, addr); }

CMemoryCardSys::CMemoryCardSys() {
  Initialize();
  mIsCardSysExists = true;
}

void CMemoryCardSys::Initialize() {
  if (!mIsInitialized) {
    CARDInit();
    mIsInitialized = true;
  }
}

CMemoryCardSys::~CMemoryCardSys() {
  mIsCardSysExists = false;
  FreeCardWorkArea(kCS_SlotA);
  FreeCardWorkArea(kCS_SlotB);
}

ProbeResults CMemoryCardSys::IsMemoryCardInserted(EMemoryCardPort port) {
  ProbeResults result;
  result.mError =
      static_cast< ECardResult >(CARDProbeEx(port, &result.mCardSize, &result.mSectorSize));
  return result;
}

ECardResult CMemoryCardSys::GetResultCode(int port) {
  return static_cast< ECardResult >(CARDGetResultCode(port));
}

ECardResult CMemoryCardSys::MountCard(EMemoryCardPort port) {
  return static_cast< ECardResult >(
      CARDMountAsync(port, AllocCardWorkArea(port), nullptr, nullptr));
}

ECardResult CMemoryCardSys::UnmountCard(EMemoryCardPort port) {
  ECardResult result = static_cast< ECardResult >(CARDUnmount(port));
  FreeCardWorkArea(port);
  return result;
}

ECardResult CMemoryCardSys::FormatCard(EMemoryCardPort port) {
  return static_cast< ECardResult >(CARDFormatAsync(port, nullptr));
}

ECardResult CMemoryCardSys::GetNumFreeBytes(EMemoryCardPort port, uint& freeBytes,
                                            uint& freeFiles) {
  s32 bytes;
  s32 files;
  ECardResult result = static_cast< ECardResult >(CARDFreeBlocks(port, &bytes, &files));
  freeBytes = bytes;
  freeFiles = files;
  return result;
}

ECardResult CMemoryCardSys::FastDeleteFile(EMemoryCardPort port, int fileNo) {
  return static_cast< ECardResult >(CARDFastDeleteAsync(port, fileNo, nullptr));
}

ECardResult CMemoryCardSys::CheckCard(EMemoryCardPort port) {
  return static_cast< ECardResult >(CARDCheckAsync(port, nullptr));
}

ECardResult CMemoryCardSys::GetSerialNo(EMemoryCardPort port, long long& serialOut) {
  return static_cast< ECardResult >(CARDGetSerialNo(port, reinterpret_cast< u64* >(&serialOut)));
}

ECardResult CMemoryCardSys::GetStatus(EMemoryCardPort port, int fileNo, CardStat& statOut) {
  CARDStat stat;
  ECardResult result = static_cast< ECardResult >(CARDGetStatus(port, fileNo, &stat));
  memcpy(&statOut.mStat, &stat, sizeof(stat));
  return result;
}

ECardResult CMemoryCardSys::SetStatus(EMemoryCardPort port, int fileNo, const CardStat& stat) {
  return static_cast< ECardResult >(
      CARDSetStatusAsync(port, fileNo, const_cast< CARDStat* >(&stat.mStat), nullptr));
}

bool CMemoryCardSys::mIsInitialized;
bool CMemoryCardSys::mIsCardSysExists;
bool CMemoryCardSys::mIsCardBusy;
rstl::vector< char, rstl::aligned_allocator > CMemoryCardSys::mWorkAreaA;
rstl::vector< char, rstl::aligned_allocator > CMemoryCardSys::mWorkAreaB;

rstl::vector< char, rstl::aligned_allocator >&
CMemoryCardSys::WorkAreaVector(EMemoryCardPort port) {
  switch (port) {
  case kCS_SlotA:
    return mWorkAreaA;
  case kCS_SlotB:
    return mWorkAreaB;
  default:
    return mWorkAreaA;
  }
}

char* CMemoryCardSys::AllocCardWorkArea(EMemoryCardPort port) {
  rstl::vector< char, rstl::aligned_allocator >& area = WorkAreaVector(port);
  area.resize(0xa000);
  char* data = area.data();
  DCInvalidateRange(data, area.size());
  return data;
}

void CMemoryCardSys::FreeCardWorkArea(EMemoryCardPort port) {
  rstl::vector< char, rstl::aligned_allocator >& area = WorkAreaVector(port);
  area = rstl::vector< char, rstl::aligned_allocator >();
}
