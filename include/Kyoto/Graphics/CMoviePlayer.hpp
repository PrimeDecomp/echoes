#ifndef _CMOVIEPLAYER
#define _CMOVIEPLAYER

#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/CDvdRequest.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "dolphin/thp/THPInfo.h"
#include "rstl/vector.hpp"
#include "types.h"

#include <dolphin/thp/THPFile.h>

class CMoviePlayer {
public:
  enum EPlayMode {
    kPM_Stopped,
    kPM_Playing,
  };

  class CTHPTextureSet {
  public:
    CTHPTextureSet(void* y, void* u, void* v, void* audio);
    CTHPTextureSet(const CTHPTextureSet& other);
    ~CTHPTextureSet();

    void* Y() { return mY.get(); }

    void* U() { return mU.get(); }

    void* V() { return mV.get(); }

    void* Audio() { return mAudio.get(); }

    uint GetAudioSamples() const { return mAudioSamples; }

    void SetAudioSamples(uint samples) { mAudioSamples = samples; }
    uint GetAudioSamplesConsumed() const { return mAudioSamplesConsumed; }

    void SetAudioSamplesConsumed(uint samples) { mAudioSamplesConsumed = samples; }

  private:
    rstl::auto_ptr< uchar > mY;
    rstl::auto_ptr< uchar > mU;
    rstl::auto_ptr< uchar > mV;
    rstl::auto_ptr< uchar > mAudio;
    uint mAudioSamples;
    uint mAudioSamplesConsumed;
  };

  static void SetSfxVolume(uchar);
  static void SetAudioEnabled(bool enabled);
  static bool GetAudioEnabled();

  CMoviePlayer(const char* path, const float preLoadSeconds, const bool loop,
               const bool deinterlace);
  ~CMoviePlayer();

  void Update(float dt);
  void DecodeFromRead(const void* ptr);

  bool DrawVideo() const;
  void DrawFrame(int left, int right, int bottom, int top);
  void DrawFrame(const CVector3f&, const CVector3f&, const CVector3f&, const CVector3f&);
  void MixAudio(short* out, const short* in, unsigned long samples);

  EPlayMode GetPlayMode() const;
  void SetPlayMode(EPlayMode mode);
  float GetTotalSeconds() const;
  float GetPlayedSeconds() const;
  uint GetWidth() const;
  uint GetHeight() const;
  bool CanDrawVideo() const { return mIndexLoad.null(); }
  bool ContinueLoading();
  void Rewind();
  bool GetIsFullyCached() const;
  bool GetIsMovieFinishedPlaying() const;
  void DisableLoop() { mLoop = false; }
  bool IsLooping() const { return mLoop; }

private:
  struct SIndexLoad;
  static void VerifyCallbackStatus();
  static void StaticMyAudioCallback();
  void InitializeTextures();
  void ReadCompleted();
  void PostDVDReadRequestIfNeeded();
  void PrefetchNextFrame();
  void HandleDVDInterrupt(DVDFileInfo* info);
  static void DVDCallback(s32 result, DVDFileInfo* info);
  void CancelReadRequests();
  CDvdFile mDvdFile;
  THPHeader mHeader;
  THPFrameCompInfo mThpComponents;
  THPVideoInfo mVideoInfo;
  THPAudioInfo mAudioInfo;
  rstl::vector< CTHPTextureSet > mTextures;
  rstl::auto_ptr< uchar > mRequestBuffer;
  rstl::single_ptr< CRealDvdRequest > mRequest;
  rstl::vector< rstl::auto_ptr< uchar > > mRequestQueue;
  rstl::single_ptr< SIndexLoad > mIndexLoad;
  int mNextReadSize;
  int mNextReadOff;
  int mReadSizeWrapped;
  int mReadOffWrapped;
  int mCurLoadFrame;
  int mRequestFrameWrapped;
  int mCurFrame;
  int mDecodedTexSlot;
  int mDrawTexSlot;
  int mAudioSlot;
  int mDecodedTexCount;
  float mFrameRem;
  int mPrefetchFrame;
  int mPrefetchOff;
  int mPrefetchSize;
  rstl::single_ptr< uchar > mPrefetchBuffer;
  rstl::single_ptr< CRealDvdRequest > mPrefetchRequest;
  EPlayMode mPlayMode;
  float mTotalSeconds;
  float mCurSeconds;
  float mPreLoadSeconds;
  int mPreLoadFrames;
  bool mLoop : 1;
  bool mDeinterlace : 1;
  bool mIs60Hz : 1;
  bool mHasAudio : 1;
  bool mFieldFlip : 1;
  uint mCachedBytes;
  int mFieldIndex;
  uchar mVolume;
};
CHECK_SIZEOF(CMoviePlayer, 0x120)

#endif // _CMOVIEPLAYER
