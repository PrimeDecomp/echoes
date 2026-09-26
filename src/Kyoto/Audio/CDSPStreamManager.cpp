#include "Kyoto/Audio/CDSPStreamManager.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CDvdFile.hpp"

#include "dolphin/os.h"

#include <string.h>

class CInterruptGuard {
public:
  CInterruptGuard() : mEnabled(OSDisableInterrupts()) {}
  ~CInterruptGuard() { OSRestoreInterrupts(mEnabled); }

private:
  bool mEnabled;
};

CDSPStreamManager sStreams[2];
rstl::reserved_vector< SDSPStreamVoice, 4 > sVoices;
static bool sDeferCallbacks;
static int sDeferredCallbacks;

CDSPStreamManager::CDSPStreamManager()
: mDataOffset(0)
, mFileCur(0)
, mReadsPending(0)
, mType(1)
, mState(kSS_Idle) {}

SDSPStreamVoice::SDSPStreamVoice()
: mBufferSize(0), mNumSamples(0), mType(1), mStreamId(-1), mUpperHalf(false), mFree(true) {}

void CDSPStreamManager::Initialize() {
  for (int i = 0; i < 4; ++i) {
    sVoices.push_back(SDSPStreamVoice());
  }

  sStreams[0].mType = 0;
  for (int i = 0; i < 2; ++i) {
    sVoices[i].mBuffer = rstl::auto_ptr< uchar >(
        static_cast< uchar* >(CMemory::Alloc(0x11E00, IAllocator::kHI_RoundUpLen)));
    sVoices[i].mType = 0;
  }
  CMemory::OffsetFakeStatics(0x23C00);
}

void CDSPStreamManager::Shutdown() {
  for (int i = 0; i < 2; ++i) {
    if (sStreams[i].mState != kSS_Idle) {
      sStreams[i].Deallocate();
    }
  }

  for (int i = 0; i < 4; ++i) {
    if (sVoices[i].mType == 0) {
      sVoices[i].mBuffer = rstl::auto_ptr< uchar >();
    }
  }
  sVoices.clear();
  CMemory::OffsetFakeStatics(-0x23C00);
}

int CDSPStreamManager::StartStreaming(const rstl::string& fileName, int volume, int type) {
  CInterruptGuard guard;
  int i;
  for (i = 0; i < 2; ++i) {
    CDSPStreamManager& stream = sStreams[i];
    if (stream.mState != kSS_Idle || stream.mType != type) {
      continue;
    }

    rstl::string path(fileName);
    if (strncmp("mem:", fileName.data(), strlen("mem:")) == 0) {
      path = fileName.substr(4, -1);
      if (CDvdFile::FileExists(path.data())) {
        stream.mPreload = CFilePreload(path);
        if (!stream.mPreload->IsReady()) {
          stream.mPreload = rstl::optional_object< CFilePreload >();
        }
      }
    }

    if (DVDOpen(const_cast< char* >(path.data()), &stream.mFiles[0])) {
      stream.mDataOffset = sizeof(SRSFHeader);
      stream.mState = kSS_ReadingHeader;
      stream.mFileCur = 0;
      stream.mVolume = volume;
      for (int j = 1; j < 10; ++j) {
        DVDOpen(const_cast< char* >(path.data()), &stream.mFiles[j]);
      }
      stream.mVoices.clear();
      stream.mReadsPending = 1;
      if (stream.mPreload) {
        stream.mPreload->Read(&stream.mHeader, 0, sizeof(SRSFHeader));
        DVDCallback(0, &stream.mFiles[0]);
      } else {
        DVDReadAsyncPrio(&stream.mFiles[0], &stream.mHeader, sizeof(SRSFHeader), 0, DVDCallback,
                         1);
      }
      break;
    }
  }

  return i < 2 ? i : -1;
}

int CDSPStreamManager::ActivateVoices() {
  CInterruptGuard guard;
  for (int* it = mVoices.begin(); it != mVoices.end(); ++it) {
    sndStreamActivate(sVoices[*it].mStreamId);
  }
  return 0;
}

int CDSPStreamManager::AllocateVoice(int volume, uchar pan, SND_ADPCMSTREAM_INFO* info,
                                     bool primary) {
  int idx = 0;
  CInterruptGuard guard;
  for (SDSPStreamVoice* it = sVoices.begin(); it != sVoices.end(); ++it, ++idx) {
    if (!it->mFree || mType != it->mType) {
      continue;
    }

    it->mFree = false;
    it->mUpperHalf = false;
    it->mBufferSize = 0x11E00;
    it->mPan = pan;
    it->mNumSamples = (it->mBufferSize / 8) * 14;
    if (it->mType != 0) {
      it->mBuffer = rstl::auto_ptr< uchar >(
          static_cast< uchar* >(CMemory::Alloc(it->mBufferSize, IAllocator::kHI_RoundUpLen)));
    }
    it->mStreamId = sndStreamAllocEx(0xFF, it->mBuffer.get(), it->mNumSamples, mHeader.mSampleRate,
                                     volume, it->mPan, 0, 0, 0, 0, 0x30001,
                                     primary ? UpdateStream : UpdateSecondaryStream,
                                     reinterpret_cast< u32 >(this), nullptr);
    sndStreamADPCMParameter(it->mStreamId, info);
    return idx;
  }
  return -1;
}

void CDSPStreamManager::DVDCallback(s32 result, DVDFileInfo* fileInfo) {
  static volatile bool sBusy = false;
  static volatile int sCount = 0;
  static volatile int sReadIdx = 0;
  static volatile int sWriteIdx = 0;
  static DVDFileInfo* volatile sQueue[8];

  {
    CInterruptGuard guard;
    sQueue[sWriteIdx] = fileInfo;
    sWriteIdx = (sWriteIdx + 1) & 7;
    ++sCount;
    if (sBusy) {
      return;
    }
    sBusy = true;
  }

  for (;;) {
    DVDFileInfo* next;
    {
      CInterruptGuard guard;
      if (sCount == 0) {
        sBusy = false;
        return;
      }
      --sCount;
      next = sQueue[sReadIdx];
      sReadIdx = (sReadIdx + 1) & 7;
    }
    ReadCompleted(result, fileInfo);
  }
}

void CDSPStreamManager::ReadCompleted(s32 result, DVDFileInfo* fileInfo) {
  int i;
  for (i = 0; i < 2; ++i) {
    int j;
    for (j = 0; j < 10; ++j) {
      if (&sStreams[i].mFiles[j] == fileInfo) {
        break;
      }
    }
    if (j != 10) {
      break;
    }
  }
  if (i >= 2) {
    return;
  }

  CDSPStreamManager& stream = sStreams[i];
  --stream.mReadsPending;
  if (stream.mReadsPending > 0) {
    return;
  }

  if (stream.mPreload || DVDGetCommandBlockStatus(&fileInfo->cb) == 0) {
    switch (stream.mState) {
    case kSS_ReadingHeader:
      DCInvalidateRange(&stream.mHeader, sizeof(SRSFHeader));
      if (stream.mHeader.mMagic != 'RS' || stream.mHeader.mVersion != 3) {
        stream.Deallocate();
        return;
      }
      if (stream.mHeader.mNumChannels == 1) {
        stream.mVoices.push_back(
            stream.AllocateVoice(stream.mVolume, 0x40, &stream.mHeader.mAdpcmInfo[0], true));
      } else if (stream.mHeader.mNumChannels == 2) {
        stream.mVoices.push_back(
            stream.AllocateVoice(stream.mVolume, 0, &stream.mHeader.mAdpcmInfo[0], true));
        stream.mVoices.push_back(
            stream.AllocateVoice(stream.mVolume, 0x7F, &stream.mHeader.mAdpcmInfo[1], false));
      } else {
        stream.Deallocate();
        return;
      }
      stream.mState = kSS_Priming;
      stream.BufferStream();
      break;
    case kSS_Priming:
      stream.UpdateARAM();
      stream.ActivateVoices();
      stream.BufferStream();
      stream.mState = kSS_Playing;
      break;
    case kSS_Playing:
      stream.UpdateARAM();
      break;
    case kSS_Stopping:
      stream.Deallocate();
      break;
    }
  } else {
    stream.Deallocate();
  }
}

void CDSPStreamManager::UpdateARAM() {
  for (int* it = mVoices.begin(); it != mVoices.end(); ++it) {
    SDSPStreamVoice& voice = sVoices[*it];
    sndStreamARAMUpdate(voice.mStreamId, voice.mUpperHalf ? voice.mNumSamples / 2 : 0,
                        voice.mNumSamples / 2, 0, 0);
    voice.mUpperHalf = !voice.mUpperHalf;
  }
}

void CDSPStreamManager::IssueRead(DVDFileInfo* file, void* dest, int length, int offset,
                                  const SDSPStreamVoice& voice, CDSPStreamManager& stream) {
  if (stream.mPreload) {
    stream.mPreload->Read(dest, offset, length);
    if (sDeferCallbacks) {
      ++sDeferredCallbacks;
    } else {
      DVDCallback(0, file);
    }
  } else {
    DVDReadAsyncPrio(file, dest, length, offset, DVDCallback, 1);
  }
}

void CDSPStreamManager::ReadData(uchar* left, uchar* right, uint length, int dataOffset,
                                 uint fileCur) {
  if (right != nullptr) {
    uint remaining = length;
    for (uint pos = fileCur; remaining != 0 && pos < length + fileCur;) {
      uint block = pos / 0x8F00;
      uint blockStart = block * 0x8F00;
      uint blockOffset = pos - blockStart;
      uint avail = 0x8F00 - blockOffset;
      uint chunk = remaining < avail ? remaining : avail;
      uint offset = dataOffset + (block * 0x11E00 + blockOffset);
      uint channelSize = 0x8F00;
      if ((block + 1) * 0x8F00 >= mHeader.mDataSize) {
        channelSize = mHeader.mDataSize - blockStart;
      }
      IssueRead(&mFiles[mReadsPending], left + pos - fileCur, chunk, offset, sVoices[mVoices[0]],
                *this);
      IssueRead(&mFiles[mReadsPending + 1], right + pos - fileCur, chunk, offset + channelSize,
                sVoices[mVoices[1]], *this);
      pos += chunk;
      remaining -= chunk;
      mReadsPending += 2;
    }
  } else {
    IssueRead(&mFiles[mReadsPending], left, length, dataOffset + fileCur, sVoices[mVoices[0]],
              *this);
    ++mReadsPending;
  }
}

void CDSPStreamManager::BufferStream() {
  int readLen = 0x8F00;
  bool stereo = mVoices.size() > 1;
  int end;
  if (mHeader.mLoopFlag != 0) {
    end = (mHeader.mLoopEnd + 31) & ~31;
  } else {
    end = mHeader.mDataSize;
  }

  int loopLen = 0;
  uchar* left = sVoices[mVoices[0]].GetCurrentBuffer();
  uchar* right = nullptr;
  if (stereo) {
    right = sVoices[mVoices[1]].GetCurrentBuffer();
  }

  if (mFileCur + 0x8F00 > end) {
    if (mFileCur < end) {
      readLen = end - mFileCur;
      if (mHeader.mLoopFlag != 0) {
        loopLen = 0x8F00 - readLen;
      } else {
        DCZeroRange(left + readLen, 0x8F00 - readLen);
        DCFlushRange(left + readLen, 0x8F00 - readLen);
        if (stereo) {
          DCZeroRange(right + readLen, 0x8F00 - readLen);
          DCFlushRange(right + readLen, 0x8F00 - readLen);
        }
      }
    } else if (mHeader.mLoopFlag != 0) {
      mFileCur = mHeader.mLoopStart;
    } else {
      readLen = 0;
      DCZeroRange(left, 0x8F00);
      DCFlushRange(left, 0x8F00);
      if (stereo) {
        DCZeroRange(right, 0x8F00);
        DCFlushRange(right, 0x8F00);
      }
      UpdateARAM();
    }
  }

  CInterruptGuard guard;
  sDeferCallbacks = true;
  if (readLen > 0) {
    ReadData(left, right, readLen, mDataOffset, mFileCur);
    if (loopLen != 0) {
      mFileCur = mHeader.mLoopStart;
      ReadData(left + readLen, stereo ? right + readLen : nullptr, loopLen, mDataOffset, mFileCur);
      mFileCur += loopLen;
    } else {
      mFileCur += readLen;
    }
  } else if (mState != kSS_Finishing) {
    mState = kSS_Finishing;
  } else if (mState != kSS_Stopping) {
    StopStream();
  }
  sDeferCallbacks = false;

  for (int i = 0; i < sDeferredCallbacks; ++i) {
    DVDCallback(0, &mFiles[0]);
  }
  sDeferredCallbacks = 0;
}

u32 CDSPStreamManager::UpdateStream(void* buf1, u32 len1, void* buf2, u32 len2, u32 user) {
  CDSPStreamManager* stream = reinterpret_cast< CDSPStreamManager* >(user);
  u32 half = sVoices[stream->mVoices[0]].mNumSamples / 2;
  if (len1 + len2 < half) {
    return 0;
  }
  if (stream->mReadsPending > 0) {
    return 0;
  }
  stream->BufferStream();
  return half;
}

u32 CDSPStreamManager::UpdateSecondaryStream(void* buf1, u32 len1, void* buf2, u32 len2,
                                             u32 user) {
  return 0;
}

void CDSPStreamManager::StopStream() {
  if (mReadsPending > 0) {
    mState = kSS_Stopping;
  } else {
    Deallocate();
  }
}

void CDSPStreamManager::Deallocate() {
  CInterruptGuard guard;
  for (int* it = mVoices.begin(); it != mVoices.end(); ++it) {
    SDSPStreamVoice& voice = sVoices[*it];
    sndStreamDeactivate(voice.mStreamId);
    sndStreamFree(voice.mStreamId);
    voice.mStreamId = -1;
    if (voice.mType != 0) {
      voice.mBuffer = rstl::auto_ptr< uchar >();
    }
    voice.mFree = true;
  }

  for (int i = 0; i < 10; ++i) {
    DVDClose(&mFiles[i]);
  }
  mVoices.clear();
  mPreload = rstl::optional_object< CFilePreload >();
  mState = kSS_Idle;
}

void CDSPStreamManager::StopStreaming(int handle) {
  if (handle >= 0 && handle < 2) {
    CInterruptGuard guard;
    if (sStreams[handle].mState != kSS_Idle) {
      sStreams[handle].StopStream();
    }
  }
}

void CDSPStreamManager::UpdateVolume(int handle, int volume) {
  if (handle >= 0 && handle < 2) {
    CInterruptGuard guard;
    CDSPStreamManager& stream = sStreams[handle];
    if (stream.mState == kSS_Playing) {
      for (int* it = stream.mVoices.begin(); it != stream.mVoices.end(); ++it) {
        SDSPStreamVoice& voice = sVoices[*it];
        sndStreamMixParameter(voice.mStreamId, volume, voice.mPan, 0, 0);
      }
    }
  }
}

bool CDSPStreamManager::IsStreamAvailable(int handle) {
  if (handle < 0 && handle >= 2) {
    return true;
  }
  CInterruptGuard guard;
  int state = sStreams[handle].mState;
  return state != kSS_ReadingHeader && state != kSS_Priming;
}

bool CDSPStreamManager::CanStop(int handle) {
  if (handle < 0 && handle >= 2) {
    return true;
  }
  CInterruptGuard guard;
  return sStreams[handle].mState == kSS_Idle;
}

int CDSPStreamManager::GetStreamState(int handle) {
  if (handle < 0 && handle >= 2) {
    return kS_Oneshot;
  }
  CInterruptGuard guard;
  if (sStreams[handle].mHeader.mLoopFlag != 0) {
    return kS_Looping;
  }
  return kS_Oneshot;
}
