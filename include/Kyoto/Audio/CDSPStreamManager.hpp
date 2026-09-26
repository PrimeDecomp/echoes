#ifndef _CDSPSTREAMMANAGER
#define _CDSPSTREAMMANAGER

#include "types.h"

#include "Kyoto/Streams/CFilePreload.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/string.hpp"

#include "dolphin/dvd.h"
#include "musyx/musyx.h"

// Header of an Echoes .rsf stream: "RS" version 3, one or two ADPCM channels.
struct SRSFHeader {
  ushort mMagic;
  ushort mVersion;
  uint mNumChannels;
  uint x8_;
  uint mSampleRate;
  uint mDataSize;
  ushort mLoopFlag;
  uint mLoopStart;
  uint mLoopEnd;
  SND_ADPCMSTREAM_INFO mAdpcmInfo[2];
};
CHECK_SIZEOF(SRSFHeader, 0x60)

// One sndStream voice with its ARAM staging buffer.
struct SDSPStreamVoice {
  SDSPStreamVoice();

  uchar* GetCurrentBuffer() const {
    return mBuffer.get() + (mUpperHalf ? mBufferSize / 2 : 0);
  }

  rstl::auto_ptr< uchar > mBuffer;
  uint mBufferSize;
  uint mNumSamples;
  uchar mPan;
  int mType;
  SND_STREAMID mStreamId;
  bool mUpperHalf : 1;
  bool mFree : 1;
};
CHECK_SIZEOF(SDSPStreamVoice, 0x20)

class CDSPStreamManager {
public:
  enum EState { kS_Looping, kS_Oneshot };

  CDSPStreamManager();

  static void Initialize();
  static void Shutdown();

  static int StartStreaming(const rstl::string& fileName, int volume, int type);
  static void StopStreaming(int handle);
  static void UpdateVolume(int handle, int volume);
  static bool IsStreamAvailable(int handle);
  static bool CanStop(int handle);
  static int GetStreamState(int handle);

private:
  enum EStreamState {
    kSS_Idle,
    kSS_ReadingHeader,
    kSS_Priming,
    kSS_Playing,
    kSS_Stopping,
    kSS_Finishing,
  };

  static void DVDCallback(s32 result, DVDFileInfo* fileInfo);
  static void ReadCompleted(s32 result, DVDFileInfo* fileInfo);
  static void IssueRead(DVDFileInfo* file, void* dest, int length, int offset,
                        const SDSPStreamVoice& voice, CDSPStreamManager& stream);
  static u32 UpdateStream(void* buf1, u32 len1, void* buf2, u32 len2, u32 user);
  static u32 UpdateSecondaryStream(void* buf1, u32 len1, void* buf2, u32 len2, u32 user);

  int AllocateVoice(int volume, uchar pan, SND_ADPCMSTREAM_INFO* info, bool primary);
  int ActivateVoices();
  void UpdateARAM();
  void ReadData(uchar* left, uchar* right, uint length, int dataOffset, uint fileCur);
  void BufferStream();
  void StopStream();
  void Deallocate();

  SRSFHeader mHeader;
  DVDFileInfo mFiles[10];
  int mDataOffset;
  int mFileCur;
  int mReadsPending;
  char mVolume;
  int mType;
  int mState;
  rstl::reserved_vector< int, 2 > mVoices;
  rstl::optional_object< CFilePreload > mPreload;
} ATTRIBUTE_ALIGN(32);
CHECK_SIZEOF(CDSPStreamManager, 0x300)

#endif // _CDSPSTREAMMANAGER
