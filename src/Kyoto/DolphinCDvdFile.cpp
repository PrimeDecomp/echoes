#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/CDvdRequest.hpp"

#include "Kyoto/CARAMManager.hpp"

#include "dolphin/os/OSCache.h"

#include "dolphin/arq.h"
#include "dolphin/dvd.h"
#include "dolphin/os.h"

#include "string.h"

static CDvdFile* sFirstARAM = nullptr;
// The original names of these two mode/activity flags are not yet known.
bool lbl_80419B9C = false;
bool lbl_80419B9D = false;

struct CDvdFileARAM {
  CDvdFileARAM()
  : mGotARAMInterrupt(false)
  , mGotDvdInterrupt(false)
  , mFileSize1(0)
  , mFileSize2(0)
  , mCurBufferLen(0)
  , mAramOffset(0)
  , mBufferLen(0)
  , mBufferIndex(0) {}
  ~CDvdFileARAM();

  ARQRequest mARQRequest;
  struct SDvdInfo {
    SDvdInfo() : mDvdFile(nullptr), mNextfile(nullptr) {}
    DVDFileInfo mDvdFileInfo;
    CDvdFile* mDvdFile;
    CDvdFile* mNextfile;
  } mInfo;
  rstl::reserved_vector< rstl::auto_ptr< uchar >, 2 > mBuffers;
  bool mGotARAMInterrupt;
  bool mGotDvdInterrupt;
  uint mFileSize1;
  uint mFileSize2;
  int mCurBufferLen;
  uint mAramOffset;
  int mBufferLen;
  uint mBufferIndex;
};

CHECK_SIZEOF(CDvdFileARAM, 0x94)

const char* DecodeARAMFile(const char* filename) {
  if (!strncmp(filename, "aram:", 5)) {
    return filename + 5;
  }

  return filename;
}

void CDvdFile::DVDARAMXferCallback(s32 result, DVDFileInfo* info) {
  CDvdFileARAM::SDvdInfo* ptr = reinterpret_cast< CDvdFileARAM::SDvdInfo* >(info);
  if (!lbl_80419B9C) {
    DVDClose(&ptr->mDvdFileInfo);
  }
  ptr->mDvdFile->HandleDVDInterrupt();
}

void CDvdFile::ARAMARAMXferCallback(u32 addr) {
  reinterpret_cast< CDvdFileARAM* >(addr)->mInfo.mDvdFile->HandleARAMInterrupt();
}

void CDvdFile::HandleARAMInterrupt() {
  BOOL enabled = OSDisableInterrupts();
  CDvdFileARAM* arFile = xc_aramFile.get();

  arFile->mGotARAMInterrupt = true;

  if (!lbl_80419B9C && arFile->mGotARAMInterrupt && arFile->mGotDvdInterrupt) {
    PingARAMTransfer();
  }

  OSRestoreInterrupts(enabled);
}

void CDvdFile::HandleDVDInterrupt() {
  BOOL enabled = OSDisableInterrupts();
  CDvdFileARAM* arFile = xc_aramFile.get();

  arFile->mGotDvdInterrupt = true;

  if (!lbl_80419B9C && arFile->mGotARAMInterrupt && arFile->mGotDvdInterrupt) {
    PingARAMTransfer();
  }

  OSRestoreInterrupts(enabled);
}

void CDvdFile::PingARAMTransfer() {
  CDvdFileARAM* aramFile = xc_aramFile.get();

  if (aramFile->mBufferLen == 0) {
    PopARAMFileLoad();
    return;
  }

  int length = aramFile->mBufferLen < 65536 ? aramFile->mBufferLen : 65536;
  ARQPostRequest(&aramFile->mARQRequest, 0, ARQ_TYPE_MRAM_TO_ARAM, ARQ_PRIORITY_HIGH,
                 reinterpret_cast< u32 >(aramFile->mBuffers[aramFile->mBufferIndex].get()),
                 reinterpret_cast< u32 >(x4_aramBuffer + aramFile->mAramOffset), length,
                 ARAMARAMXferCallback);

  aramFile->mBufferLen -= length;
  aramFile->mAramOffset += length;
  aramFile->mGotARAMInterrupt = false;
  aramFile->mBufferIndex ^= 1;

  if (aramFile->mCurBufferLen != 0) {
    int length2 = aramFile->mCurBufferLen < 65536 ? aramFile->mCurBufferLen : 65536;
    aramFile->mGotDvdInterrupt = false;
    DVDFastOpen(x0_fileEntry, &aramFile->mInfo.mDvdFileInfo);
    DVDReadAsync(&aramFile->mInfo.mDvdFileInfo, aramFile->mBuffers[aramFile->mBufferIndex].get(),
                 length2, aramFile->mFileSize2, DVDARAMXferCallback);
    lbl_80419B9D = true;
    aramFile->mFileSize2 += length2;
    aramFile->mCurBufferLen -= length2;
  }
}

void CDvdFile::TryARAMFile() {
  x4_aramBuffer = static_cast< uchar* >(CARAMManager::Alloc(x14_size));
  if (!CARAMManager::IsAllocValid(x4_aramBuffer)) {
    return;
  }
  xc_aramFile = rs_new CDvdFileARAM();
  CDvdFileARAM* arfile = xc_aramFile.get();
  arfile->mInfo.mDvdFile = this;
  arfile->mGotARAMInterrupt = true;
  arfile->mFileSize1 = arfile->mCurBufferLen = arfile->mBufferLen = GetFileSize();
  x8_aramAllocated = true;
  PushARAMFileLoad();
}

void CDvdFile::PushARAMFileLoad() {
  BOOL enabled = true;
  if (!lbl_80419B9C) {
    enabled = OSDisableInterrupts();
  }
  CDvdFile* file = sFirstARAM;
  if (file == NULL) {
    sFirstARAM = this;
    StartARAMFileLoad();
  } else {
    for (CDvdFile* p = file; p != nullptr; p = p->xc_aramFile->mInfo.mNextfile) {
      if (p->xc_aramFile->mInfo.mNextfile == nullptr) {
        p->xc_aramFile->mInfo.mNextfile = this;
        break;
      }
    }
  }
  if (!lbl_80419B9C) {
    OSRestoreInterrupts(enabled);
  }
}

void CDvdFile::PopARAMFileLoad() {
  BOOL enabled = true;
  if (!lbl_80419B9C) {
    enabled = OSDisableInterrupts();
  }
  CDvdFile* file = xc_aramFile->mInfo.mNextfile;
  x9_aramPopped = true;
  sFirstARAM = file;
  if (file != nullptr) {
    file->StartARAMFileLoad();
  }

  if (!lbl_80419B9C) {
    OSRestoreInterrupts(enabled);
  }
}

bool CDvdFile::IsARAMFileLoaded() {
  if (!x8_aramAllocated) {
    return true;
  }

  if (!x9_aramPopped) {
    if (lbl_80419B9C && xc_aramFile->mGotARAMInterrupt && xc_aramFile->mGotDvdInterrupt) {
      PingARAMTransfer();
    }
    return false;
  }

  xc_aramFile = nullptr;

  return true;
}

void CDvdFile::StartARAMFileLoad() {
  CDvdFileARAM* aramFile = xc_aramFile.get();
  aramFile->mBuffers.push_back(
      static_cast< uchar* >(CMemory::Alloc(0x10000, IAllocator::kHI_RoundUpLen)));
  aramFile->mBuffers.push_back(
      static_cast< uchar* >(CMemory::Alloc(0x10000, IAllocator::kHI_RoundUpLen)));

  int len = x14_size > 65536 ? 65536 : x14_size;
  aramFile->mCurBufferLen -= len;
  aramFile->mFileSize2 = len;
  if (!lbl_80419B9C) {
    DVDFastOpen(x0_fileEntry, &aramFile->mInfo.mDvdFileInfo);
  } else {
    DVDOpen(const_cast< char* >(DecodeARAMFile(x18_filename.data())),
            &aramFile->mInfo.mDvdFileInfo);
  }
  DVDReadAsync(&aramFile->mInfo.mDvdFileInfo, aramFile->mBuffers[0].get(), len, 0,
               DVDARAMXferCallback);
  lbl_80419B9D = true;
}

void CDvdFile::StallForARAMFile() {
  while (xc_aramFile.get() != nullptr) {
    OSYieldThread();
  }
}

CDvdFile::CDvdFile(const char* filename)
: x0_fileEntry(-1)
, x4_aramBuffer(0)
, x8_aramAllocated(false)
, x9_aramPopped(false)
, xc_aramFile(nullptr)
, x10_offset(0)
, x14_size(0)
, x18_filename(filename, -1) {
  const char* decodedName = DecodeARAMFile(filename);
  x0_fileEntry = DVDConvertPathToEntrynum(const_cast< char* >(decodedName));
  DVDFileInfo fileInfo;
  if (x0_fileEntry != -1) {
    DVDFastOpen(x0_fileEntry, &fileInfo);
  }

  x14_size = fileInfo.length;
  DVDClose(&fileInfo);

  if (filename != decodedName) {
    TryARAMFile();
  }
}

CDvdFileARAM::~CDvdFileARAM() {}

CDvdFile::~CDvdFile() { CloseFile(); }

CDvdRequest* CDvdFile::SyncRead(void* dest, uint len) {
  return AsyncSeekRead(dest, len, kSO_Cur, 0);
}

void CDvdFile::SyncSeekRead(void* dest, uint len, ESeekOrigin origin, int offset) {
  StallForARAMFile();
  CalcFileOffset(offset, origin);

  if (x8_aramAllocated) {
    uint roundedLen = (len + 31) & ~31;
    DCFlushRange(dest, roundedLen);
    CARAMManager::WaitForDMACompletion(CARAMManager::DMAToMRAM(
        x4_aramBuffer + x10_offset, dest, roundedLen, CARAMManager::kDMAPrio_One));
  } else {
    DVDFileInfo info;
    if (!lbl_80419B9C) {
      DVDFastOpen(x0_fileEntry, &info);
    } else {
      DVDOpen(const_cast< char* >(DecodeARAMFile(x18_filename.data())), &info);
    }
    DVDReadAsync(&info, dest, (len + 31) & ~31, x10_offset, internalCallback);
    lbl_80419B9D = true;
    while (DVDGetCommandBlockStatus(&info.cb) != DVD_STATE_END) {
    }
    DVDClose(&info);
  }

  UpdateFilePos(len);
}

CDvdRequest* CDvdFile::AsyncSeekRead(void* dest, uint len, ESeekOrigin origin, int offset) {
  StallForARAMFile();
  CalcFileOffset(offset, origin);
  CDvdRequest* request;
  if (x8_aramAllocated) {
    const int roundedLen = (len + 31) & ~31;
    DCFlushRange(dest, roundedLen);
    request = rs_new CARAMDvdRequest(CARAMManager::DMAToMRAM(
        x4_aramBuffer + x10_offset, dest, roundedLen, CARAMManager::kDMAPrio_One));
  } else {
    CRealDvdRequest* req = rs_new CRealDvdRequest();
    DVDFileInfo* info = &req->FileInfo();
    DVDFastOpen(x0_fileEntry, info);
    DVDReadAsync(info, dest, (len + 31) & ~31, x10_offset, internalCallback);
    lbl_80419B9D = true;
    request = req;
  }

  UpdateFilePos(len);

  return request;
}

void CDvdFile::CloseFile() {
  if (!x8_aramAllocated) {
    return;
  }

  StallForARAMFile();
  CARAMManager::Free(x4_aramBuffer);
}

bool CDvdFile::FileExists(const char* filename) {
  return DVDConvertPathToEntrynum(const_cast< char* >(DecodeARAMFile(filename))) != -1;
}

void CDvdFile::internalCallback(s32 res, DVDFileInfo* info) {
  if (res != DVD_STATE_CANCELED) {
    DCInvalidateRange(info->cb.addr, info->cb.length);
    lbl_80419B9D = true;
  }
}

void CDvdFile::CalcFileOffset(int offset, ESeekOrigin origin) {
  switch (origin) {
  case kSO_Set:
    x10_offset = offset;
    break;
  case kSO_Cur:
    x10_offset += offset;
    break;
  case kSO_End:
    x10_offset = offset + x14_size;
    break;
  }
}

void CDvdFile::UpdateFilePos(int pos) {
  x10_offset += (pos + 31) & ~31;
  int filesize = GetFileSize();
  if (x10_offset > filesize) {
    x10_offset = filesize;
  }
}
