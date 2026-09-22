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

    void* Y() { return x0_y.get(); }

    void* U() { return x8_u.get(); }

    void* V() { return x10_v.get(); }

    void* Audio() { return x18_audio.get(); }

    uint GetAudioSamples() const { return x20_audioSamples; }

    void SetAudioSamples(uint samples) { x20_audioSamples = samples; }
    uint GetAudioSamplesConsumed() const { return x24_audioSamplesConsumed; }

    void SetAudioSamplesConsumed(uint samples) { x24_audioSamplesConsumed = samples; }

  private:
    rstl::auto_ptr< uchar > x0_y;
    rstl::auto_ptr< uchar > x8_u;
    rstl::auto_ptr< uchar > x10_v;
    rstl::auto_ptr< uchar > x18_audio;
    uint x20_audioSamples;
    uint x24_audioSamplesConsumed;
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
  bool CanDrawVideo() const { return xb4_indexLoad.null(); }
  bool ContinueLoading();
  void Rewind();
  bool GetIsFullyCached() const;
  bool GetIsMovieFinishedPlaying() const;
  void DisableLoop() { x110_24_loop = false; }
  bool IsLooping() const { return x110_24_loop; }

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
  CDvdFile x0_dvdFile;
  THPHeader x28_header;
  THPFrameCompInfo x58_thpComponents;
  THPVideoInfo x6c_videoInfo;
  THPAudioInfo x78_audioInfo;
  rstl::vector< CTHPTextureSet > x88_textures;
  rstl::auto_ptr< uchar > x98_requestBuffer;
  rstl::single_ptr< CRealDvdRequest > xa0_request;
  rstl::vector< rstl::auto_ptr< uchar > > xa4_requestQueue;
  rstl::single_ptr< SIndexLoad > xb4_indexLoad;
  int xb8_nextReadSize;
  int xbc_nextReadOff;
  int xc0_readSizeWrapped;
  int xc4_readOffWrapped;
  int xc8_curLoadFrame;
  int xcc_requestFrameWrapped;
  int xd0_curFrame;
  int xd4_decodedTexSlot;
  int xd8_drawTexSlot;
  int xdc_audioSlot;
  int xe0_decodedTexCount;
  float xe4_frameRem;
  int xe8_prefetchFrame;
  int xec_prefetchOff;
  int xf0_prefetchSize;
  rstl::single_ptr< uchar > xf4_prefetchBuffer;
  rstl::single_ptr< CRealDvdRequest > xf8_prefetchRequest;
  EPlayMode xfc_playMode;
  float x100_totalSeconds;
  float x104_curSeconds;
  float x108_preLoadSeconds;
  int x10c_preLoadFrames;
  bool x110_24_loop : 1;
  bool x110_25_deinterlace : 1;
  bool x110_26_is60Hz : 1;
  bool x110_27_hasAudio : 1;
  bool x110_28_fieldFlip : 1;
  uint x114_cachedBytes;
  int x118_fieldIndex;
  uchar x11c_volume;
};
CHECK_SIZEOF(CMoviePlayer, 0x120)

#endif // _CMOVIEPLAYER
