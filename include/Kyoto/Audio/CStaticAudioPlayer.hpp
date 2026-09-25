#ifndef _CSTATICAUDIOPLAYER
#define _CSTATICAUDIOPLAYER

#include "types.h"

#include <Kyoto/Audio/g721.h>
#include <rstl/auto_ptr.hpp>
#include <rstl/string.hpp>
#include <rstl/vector.hpp>

class CDvdRequest;
typedef void (*FAudioCallback)();

class CStaticAudioPlayer {
public:
  CStaticAudioPlayer(const rstl::string& filepath, const int loopStart, const int loopEnd);
  ~CStaticAudioPlayer();

  const bool IsReady() const;
  void StartMixOut();
  void StopMixOut();

  static void MixCallback();
  void DoMix();
  static void RunDMACallback(FAudioCallback);
  static void CancelDMACallback(FAudioCallback);
  static void InstallAICallback();
  static void AICallback();

  void Decode(ushort* out, const ushort* in, int numSamples);
  void DecodeMonoAndMix(ushort* out, const ushort* in, int numSamples, int startSample,
                        int sampleEnd, int sampleStart, int vol, g72x_state& state);
  void SetVolume(uchar vol);

private:
  rstl::string mFilepath;
  int mRsfRem;
  int mRsfLength;
  int mCurSamp;
  int mLoopStartSamp;
  int mLoopEndSamp;
  int mCurBuf;
  rstl::auto_ptr< uchar > mDmaBufferA;
  rstl::auto_ptr< uchar > mDmaBufferB;
  rstl::vector< rstl::auto_ptr< CDvdRequest > > mDvdRequests;
  rstl::vector< rstl::auto_ptr< uchar > > mBuffers;
  g72x_state mLeftState;
  g72x_state mRightState;
  ushort mVolume;
};

CHECK_SIZEOF(CStaticAudioPlayer, 0xC4)

#endif // _CSTATICAUDIOPLAYER
