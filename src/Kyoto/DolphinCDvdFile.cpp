#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/CDvdRequest.hpp"

#include "Kyoto/CARAMManager.hpp"

#include "rstl/math.hpp"

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
  CDvdFileARAM* arFile = mARAMFile.get();

  arFile->mGotARAMInterrupt = true;

  if (!lbl_80419B9C && arFile->mGotARAMInterrupt && arFile->mGotDvdInterrupt) {
    PingARAMTransfer();
  }

  OSRestoreInterrupts(enabled);
}

void CDvdFile::HandleDVDInterrupt() {
  BOOL enabled = OSDisableInterrupts();
  CDvdFileARAM* arFile = mARAMFile.get();

  arFile->mGotDvdInterrupt = true;

  if (!lbl_80419B9C && arFile->mGotARAMInterrupt && arFile->mGotDvdInterrupt) {
    PingARAMTransfer();
  }

  OSRestoreInterrupts(enabled);
}

void CDvdFile::PingARAMTransfer() {
  CDvdFileARAM* aramFile = mARAMFile.get();

  if (aramFile->mBufferLen == 0) {
    PopARAMFileLoad();
    return;
  }

  int length = rstl::min_val(65536, aramFile->mBufferLen);
  ARQPostRequest(&aramFile->mARQRequest, 0, ARQ_TYPE_MRAM_TO_ARAM, ARQ_PRIORITY_HIGH,
                 reinterpret_cast< u32 >(aramFile->mBuffers[aramFile->mBufferIndex].get()),
                 reinterpret_cast< u32 >(mARAMBuffer + aramFile->mAramOffset), length,
                 ARAMARAMXferCallback);

  aramFile->mBufferLen -= length;
  aramFile->mAramOffset += length;
  aramFile->mGotARAMInterrupt = false;
  aramFile->mBufferIndex ^= 1;

  if (aramFile->mCurBufferLen != 0) {
    int length2 = rstl::min_val(65536, aramFile->mCurBufferLen);
    aramFile->mGotDvdInterrupt = false;
    DVDFastOpen(mFileEntry, &aramFile->mInfo.mDvdFileInfo);
    DVDReadAsync(&aramFile->mInfo.mDvdFileInfo, aramFile->mBuffers[aramFile->mBufferIndex].get(),
                 length2, aramFile->mFileSize2, DVDARAMXferCallback);
    lbl_80419B9D = true;
    aramFile->mFileSize2 += length2;
    aramFile->mCurBufferLen -= length2;
  }
}

void CDvdFile::TryARAMFile() {
  mARAMBuffer = static_cast< uchar* >(CARAMManager::Alloc(mSize));
  if (!CARAMManager::IsAllocValid(mARAMBuffer)) {
    return;
  }
  mARAMFile = rs_new CDvdFileARAM();
  CDvdFileARAM* arfile = mARAMFile.get();
  arfile->mInfo.mDvdFile = this;
  arfile->mGotARAMInterrupt = true;
  arfile->mFileSize1 = arfile->mCurBufferLen = arfile->mBufferLen = GetFileSize();
  mARAMAllocated = true;
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
    for (CDvdFile* p = file; p != nullptr; p = p->mARAMFile->mInfo.mNextfile) {
      if (p->mARAMFile->mInfo.mNextfile == nullptr) {
        p->mARAMFile->mInfo.mNextfile = this;
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
  CDvdFile* file = mARAMFile->mInfo.mNextfile;
  mARAMPopped = true;
  sFirstARAM = file;
  if (file != nullptr) {
    file->StartARAMFileLoad();
  }

  if (!lbl_80419B9C) {
    OSRestoreInterrupts(enabled);
  }
}

bool CDvdFile::IsARAMFileLoaded() {
  if (!mARAMAllocated) {
    return true;
  }

  if (!mARAMPopped) {
    if (lbl_80419B9C && mARAMFile->mGotARAMInterrupt && mARAMFile->mGotDvdInterrupt) {
      PingARAMTransfer();
    }
    return false;
  }

  mARAMFile = nullptr;

  return true;
}

void CDvdFile::StartARAMFileLoad() {
  CDvdFileARAM* aramFile = mARAMFile.get();
  aramFile->mBuffers.push_back(
      static_cast< uchar* >(CMemory::Alloc(0x10000, IAllocator::kHI_RoundUpLen)));
  aramFile->mBuffers.push_back(
      static_cast< uchar* >(CMemory::Alloc(0x10000, IAllocator::kHI_RoundUpLen)));

  int len = rstl::min_val(mSize, 65536);
  aramFile->mCurBufferLen -= len;
  aramFile->mFileSize2 = len;
  if (!lbl_80419B9C) {
    DVDFastOpen(mFileEntry, &aramFile->mInfo.mDvdFileInfo);
  } else {
    DVDOpen(const_cast< char* >(DecodeARAMFile(mFilename.data())),
            &aramFile->mInfo.mDvdFileInfo);
  }
  DVDReadAsync(&aramFile->mInfo.mDvdFileInfo, aramFile->mBuffers[0].get(), len, 0,
               DVDARAMXferCallback);
  lbl_80419B9D = true;
}

void CDvdFile::StallForARAMFile() {
  while (mARAMFile.get() != nullptr) {
    OSYieldThread();
  }
}

CDvdFile::CDvdFile(const char* filename)
: mFileEntry(-1)
, mARAMBuffer(0)
, mARAMAllocated(false)
, mARAMPopped(false)
, mARAMFile(nullptr)
, mOffset(0)
, mSize(0)
, mFilename(filename, -1) {
  const char* decodedName = DecodeARAMFile(filename);
  mFileEntry = DVDConvertPathToEntrynum(const_cast< char* >(decodedName));
  DVDFileInfo fileInfo;
  if (mFileEntry != -1) {
    DVDFastOpen(mFileEntry, &fileInfo);
  }

  mSize = fileInfo.length;
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

  if (mARAMAllocated) {
    uint roundedLen = (len + 31) & ~31;
    DCFlushRange(dest, roundedLen);
    CARAMManager::WaitForDMACompletion(CARAMManager::DMAToMRAM(
        mARAMBuffer + mOffset, dest, roundedLen, CARAMManager::kDMAPrio_One));
  } else {
    DVDFileInfo info;
    if (!lbl_80419B9C) {
      DVDFastOpen(mFileEntry, &info);
    } else {
      DVDOpen(const_cast< char* >(DecodeARAMFile(mFilename.data())), &info);
    }
    DVDReadAsync(&info, dest, (len + 31) & ~31, mOffset, internalCallback);
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
  if (mARAMAllocated) {
    const int roundedLen = (len + 31) & ~31;
    DCFlushRange(dest, roundedLen);
    request = rs_new CARAMDvdRequest(CARAMManager::DMAToMRAM(
        mARAMBuffer + mOffset, dest, roundedLen, CARAMManager::kDMAPrio_One));
  } else {
    CRealDvdRequest* req = rs_new CRealDvdRequest();
    DVDFileInfo* info = &req->FileInfo();
    DVDFastOpen(mFileEntry, info);
    DVDReadAsync(info, dest, (len + 31) & ~31, mOffset, internalCallback);
    lbl_80419B9D = true;
    request = req;
  }

  UpdateFilePos(len);

  return request;
}

void CDvdFile::CloseFile() {
  if (!mARAMAllocated) {
    return;
  }

  StallForARAMFile();
  CARAMManager::Free(mARAMBuffer);
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
    mOffset = offset;
    break;
  case kSO_Cur:
    mOffset += offset;
    break;
  case kSO_End:
    mOffset = offset + mSize;
    break;
  }
}

void CDvdFile::UpdateFilePos(int pos) {
  mOffset += (pos + 31) & ~31;
  int filesize = GetFileSize();
  if (mOffset > filesize) {
    mOffset = filesize;
  }
}
