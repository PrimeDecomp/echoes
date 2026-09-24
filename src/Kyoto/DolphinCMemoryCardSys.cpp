#include "Kyoto/CMemoryCardSys.hpp"

#include "Kyoto/CCRC32.hpp"
#include "Kyoto/Graphics/CGraphicsPalette.hpp"
#include "Kyoto/Graphics/CTexture.hpp"

#include "dolphin/os/OSCache.h"

#include "Kyoto/MemoryCopy.hpp"

static uint RoundUp(uint value, uint align) { return (value + align - 1) & ~(align - 1); }

CMemoryCardSys::CCardFileInfo::Icon::Icon(CAssetId id, int speed, CSimplePool& pool)
: x0_id(id), x4_speed(speed), x8_tex(pool.GetObj(SObjectTag('TXTR', id))) {}

CMemoryCardSys::CCardFileInfo::SSaveSlot::SSaveSlot() : x0_generation(0), x4_corrupt(false) {}

void CMemoryCardSys::CCardFileInfo::SSaveSlot::CheckCrc() {
  const uint* data = reinterpret_cast< const uint* >(x8_data.data());
  const uint crc = data[0];
  x4_corrupt = CCRC32::Calculate(data + 1, x8_data.size() - 4, 0xFFFFFFFF) != crc;
  x0_generation = data[1];
}

CMemoryCardSys::CCardFileInfo::CCardFileInfo(EMemoryCardPort port, const rstl::string& name)
: x0_slotBlocks(0)
, x4_slotSize(0)
, x8_slot(0)
, xc_generation(0)
, x10_status(kS_Standby)
, x14_newFile(false)
, x2c_fileName(name)
, x4c_(0)
, x50_bannerTex(kInvalidAssetId)
, x108_headerBuffer(0x2000, uchar())
, x118_slots(2, SSaveSlot()) {
  x18_fileInfo.chan = port;
  x18_fileInfo.fileNo = -1;
}

CMemoryCardSys::CCardFileInfo::~CCardFileInfo() {
  if (x18_fileInfo.fileNo != -1) {
    CARDClose(&x18_fileInfo);
  }
}

ECardResult CMemoryCardSys::CCardFileInfo::Open() {
  const EMemoryCardPort port = GetCardPort();
  ECardResult result =
      static_cast< ECardResult >(CARDOpen(port, x2c_fileName.data(), &x18_fileInfo));
  x18_fileInfo.chan = port;
  if (result == kCR_READY) {
    CardStat stat;
    ECardResult statResult = CMemoryCardSys::GetStatus(GetCardPort(), GetFileNo(), stat);
    if (statResult != kCR_READY) {
      return statResult;
    }
    const int blocks = stat.GetFileLength() / 0x2000;
    x0_slotBlocks = (blocks - 1) / 2;
    x4_slotSize = x0_slotBlocks * 0x2000;
  }
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::CreateFile() {
  x14_newFile = true;
  x4_slotSize = RoundUp(x15c_saveBuffer.size(), 0x2000);
  x0_slotBlocks = x4_slotSize / 0x2000;
  const int size = GetFileBlocks() * 0x2000;
  return static_cast< ECardResult >(
      CARDCreateAsync(GetCardPort(), x2c_fileName.data(), size, &x18_fileInfo, nullptr));
}

ECardResult CMemoryCardSys::CCardFileInfo::StartRead() {
  CardStat stat;
  ECardResult result = CMemoryCardSys::GetStatus(GetCardPort(), GetFileNo(), stat);
  if (result != kCR_READY) {
    return result;
  }
  const uint size = stat.GetFileLength();
  x14c_loadedData = rstl::vector< uchar >();
  x108_headerBuffer.assign(0x2000);
  x118_slots[0].x8_data.assign(x4_slotSize);
  x118_slots[1].x8_data.assign(x4_slotSize);
  result =
      static_cast< ECardResult >(CARDRead(&x18_fileInfo, x108_headerBuffer.data(), 0x2000, 0));
  if (result == kCR_READY) {
    x10_status = kS_ReadHeader;
  }
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::PumpCardRead() {
  if (x10_status == kS_Standby) {
    return kCR_READY;
  }
  ECardResult result = GetResultCode(GetCardPort());
  if (result != kCR_READY) {
    return result;
  }
  if (x10_status == kS_ReadHeader || x10_status == kS_RepairHeader) {
    if (x10_status == kS_ReadHeader && CheckHeaderCrc() != kCR_READY) {
      BuildHeaderBuffer();
      x10_status = kS_RepairHeader;
      void* data = x108_headerBuffer.data();
      DCStoreRange(data, 0x2000);
      result = static_cast< ECardResult >(CARDWriteAsync(&x18_fileInfo, data, 0x2000, 0, nullptr));
      if (result == kCR_READY) {
        return kCR_BUSY;
      }
      return result;
    } else {
      x108_headerBuffer = rstl::vector< uchar, rstl::aligned_allocator >();
      x10_status = kS_ReadSlotA;
      result = static_cast< ECardResult >(
          CARDRead(&x18_fileInfo, x118_slots[0].x8_data.data(), x4_slotSize, 0x2000));
      if (result == kCR_READY) {
        return kCR_BUSY;
      }
      return result;
    }
  } else if (x10_status == kS_ReadSlotA) {
    x10_status = kS_ReadSlotB;
    result = static_cast< ECardResult >(CARDRead(&x18_fileInfo, x118_slots[1].x8_data.data(),
                                                 x4_slotSize, x4_slotSize + 0x2000));
    if (result == kCR_READY) {
      return kCR_BUSY;
    }
    return result;
  } else {
    return SelectSaveSlot();
  }
}

ECardResult CMemoryCardSys::CCardFileInfo::WriteSaveSlot(int slot) {
  void* data = x118_slots[x8_slot].x8_data.data();
  const int offset = slot * x4_slotSize + 0x2000;
  DCStoreRange(data, x4_slotSize);
  return static_cast< ECardResult >(
      CARDWriteAsync(&x18_fileInfo, data, x4_slotSize, offset, nullptr));
}

ECardResult CMemoryCardSys::CCardFileInfo::WriteFile() {
  ECardResult result;
  BuildSaveSlot();
  if (x14_newFile) {
    BuildHeaderBuffer();
    void* data = x108_headerBuffer.data();
    DCStoreRange(data, 0x2000);
    result = static_cast< ECardResult >(CARDWriteAsync(&x18_fileInfo, data, 0x2000, 0, nullptr));
    x10_status = kS_WriteHeader;
  } else {
    result = WriteSaveSlot(x8_slot);
    x10_status = kS_WriteSlot;
  }
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::PumpCardTransfer() {
  if (x10_status == kS_Standby) {
    return kCR_READY;
  }
  ECardResult result = GetResultCode(GetCardPort());
  if (result != kCR_READY) {
    return result;
  }
  if (x10_status == kS_WriteHeader) {
    x10_status = x14_newFile ? kS_WriteFirstSlot : kS_WriteSlot;
    result = WriteSaveSlot(x8_slot);
    if (result == kCR_READY) {
      return kCR_BUSY;
    }
    return result;
  } else if (x10_status == kS_WriteFirstSlot) {
    x10_status = kS_WriteSlot;
    result = WriteSaveSlot(1 - x8_slot);
    if (result == kCR_READY) {
      return kCR_BUSY;
    }
    return result;
  } else if (x10_status == kS_WriteSlot) {
    x10_status = kS_SetStatus;
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
    x10_status = kS_Standby;
    return kCR_READY;
  }
}

CMemoryCardSys::EMemoryCardPort CMemoryCardSys::CCardFileInfo::GetCardPort() {
  return static_cast< EMemoryCardPort >(x18_fileInfo.chan);
}

int CMemoryCardSys::CCardFileInfo::GetFileNo() { return x18_fileInfo.fileNo; }

int CMemoryCardSys::CCardFileInfo::GetFileBlocks() { return x0_slotBlocks * 2 + 1; }

void CMemoryCardSys::CCardFileInfo::ResetHeaderInfo() {
  x3c_comment = rstl::string();
  x50_bannerTex = kInvalidAssetId;
  x54_bannerTok = rstl::optional_object< TLockedToken< CTexture > >();
  x64_iconToks = rstl::reserved_vector< Icon, 8 >();
}

void CMemoryCardSys::CCardFileInfo::SetComment(const rstl::string& comment) {
  x3c_comment = comment;
}

void CMemoryCardSys::CCardFileInfo::LockBannerToken(CAssetId bannerTxtr, CSimplePool& pool) {
  x50_bannerTex = bannerTxtr;
  x54_bannerTok = TLockedToken< CTexture >(pool.GetObj(SObjectTag('TXTR', x50_bannerTex)));
}

void CMemoryCardSys::CCardFileInfo::LockIconToken(CAssetId iconTxtr, int speed, CSimplePool& pool) {
  x64_iconToks.push_back(Icon(iconTxtr, speed, pool));
}

ECardResult CMemoryCardSys::CCardFileInfo::GetStatus(CardStat& stat) {
  ECardResult result = CMemoryCardSys::GetStatus(GetCardPort(), GetFileNo(), stat);
  if (result != kCR_READY) {
    return result;
  }
  stat.SetCommentAddr(4);
  stat.SetIconAddr(68);
  int bannerFormat;
  if (x50_bannerTex == kInvalidAssetId) {
    bannerFormat = CARD_STAT_BANNER_NONE;
  } else if ((*x54_bannerTok.data())->GetTexelFormat() == kTF_RGB5A3) {
    bannerFormat = CARD_STAT_BANNER_RGB5A3;
  } else {
    bannerFormat = CARD_STAT_BANNER_C8;
  }
  stat.SetBannerFormat(bannerFormat);
  const int count = x64_iconToks.size();
  int i = 0;
  for (; i < count; ++i) {
    int format = CARD_STAT_ICON_C8;
    if ((*x64_iconToks[i].x8_tex)->GetTexelFormat() == kTF_RGB5A3) {
      format = CARD_STAT_ICON_RGB5A3;
    }
    stat.SetIconFormat(format, i);
    stat.SetIconSpeed(x64_iconToks[i].x4_speed, i);
  }
  if (i < 8) {
    stat.SetIconFormat(CARD_STAT_ICON_NONE, i);
    stat.SetIconSpeed(CARD_STAT_SPEED_END, i);
  }
  return kCR_READY;
}

void CMemoryCardSys::CCardFileInfo::WriteBannerData(COutputStream& out) {
  if (x50_bannerTex != kInvalidAssetId) {
    const CTexture& texture = **x54_bannerTok.data();
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
  const int count = x64_iconToks.size();
  for (int i = 0; i < count; ++i) {
    const CTexture& texture = **x64_iconToks[i].x8_tex;
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
  x108_headerBuffer.assign(0x2000);
  uint* data = reinterpret_cast< uint* >(x108_headerBuffer.data());
  {
    CMemoryStreamOut out(data, 0x2000);
    out.WriteInt32(0);
    char comment[64];
    strncpy(comment, x3c_comment.data(), sizeof(comment));
    out.Put(comment, sizeof(comment));
    WriteBannerData(out);
    WriteIconData(out);
  }
  data[0] = CCRC32::Calculate(data + 1, 0x2000 - 4, 0xFFFFFFFF);
}

void CMemoryCardSys::CCardFileInfo::BuildSaveSlot() {
  SSaveSlot& slot = x118_slots[x8_slot];
  slot.x8_data.assign(x4_slotSize);
  uint* data = reinterpret_cast< uint* >(slot.x8_data.data());
  (memcpy)(data + 2, x15c_saveBuffer.data(), x15c_saveBuffer.size());
  data[1] = xc_generation;
  data[0] = CCRC32::Calculate(data + 1, x4_slotSize - 4, 0xFFFFFFFF);
  x15c_saveBuffer = rstl::vector< uchar >();
}

ECardResult CMemoryCardSys::CCardFileInfo::SelectSaveSlot() {
  SSaveSlot& slotA = x118_slots[0];
  SSaveSlot& slotB = x118_slots[1];
  slotA.CheckCrc();
  slotB.CheckCrc();

  ECardResult result = kCR_READY;
  int slot = -1;
  if (slotA.x4_corrupt) {
    if (slotB.x4_corrupt) {
      result = kCR_CRC_MISMATCH;
    } else {
      slot = 1;
      xc_generation = slotB.x0_generation;
    }
  } else if (slotB.x4_corrupt) {
    slot = 0;
    xc_generation = 1 - slotA.x0_generation;
  } else {
    slot = slotA.x0_generation ^ slotB.x0_generation;
    xc_generation = slot != 0 ? slotB.x0_generation : 1 - slotA.x0_generation;
  }

  x14c_loadedData = rstl::vector< uchar >();
  if (slot != -1) {
    x8_slot = 1 - slot;
    const int size = x4_slotSize - 8;
    x14c_loadedData.resize(size);
    (memcpy)(x14c_loadedData.data(), x118_slots[slot].x8_data.data() + 8, size);
  }
  slotA.x8_data = rstl::vector< uchar, rstl::aligned_allocator >();
  slotB.x8_data = rstl::vector< uchar, rstl::aligned_allocator >();
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::CheckHeaderCrc() {
  const uint* data = reinterpret_cast< const uint* >(x108_headerBuffer.data());
  const uint crc = data[0];
  return crc == CCRC32::Calculate(data + 1, 0x2000 - 4, 0xFFFFFFFF) ? kCR_READY
                                                                    : kCR_CRC_MISMATCH;
}

uint CardStat::GetFileLength() { return x0_stat.length; }

int CardStat::GetTime() const { return x0_stat.time; }

int CardStat::GetCommentAddr() const { return x0_stat.commentAddr; }

void CardStat::SetBannerFormat(int format) { CARDSetBannerFormat(&x0_stat, format); }

void CardStat::SetIconFormat(int format, int idx) { CARDSetIconFormat(&x0_stat, idx, format); }

void CardStat::SetIconSpeed(int speed, int idx) { CARDSetIconSpeed(&x0_stat, idx, speed); }

void CardStat::SetIconAddr(int addr) { CARDSetIconAddress(&x0_stat, addr); }

void CardStat::SetCommentAddr(int addr) { CARDSetCommentAddress(&x0_stat, addr); }

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
  result.x0_error =
      static_cast< ECardResult >(CARDProbeEx(port, &result.x4_cardSize, &result.x8_sectorSize));
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
  memcpy(&statOut.x0_stat, &stat, sizeof(stat));
  return result;
}

ECardResult CMemoryCardSys::SetStatus(EMemoryCardPort port, int fileNo, const CardStat& stat) {
  return static_cast< ECardResult >(
      CARDSetStatusAsync(port, fileNo, const_cast< CARDStat* >(&stat.x0_stat), nullptr));
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
