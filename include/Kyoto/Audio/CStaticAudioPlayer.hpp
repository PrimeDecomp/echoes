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
  rstl::string x0_filepath;
  int x10_rsfRem;
  int x14_rsfLength;
  int x18_curSamp;
  int x1c_loopStartSamp;
  int x20_loopEndSamp;
  int x24_curBuf;
  rstl::auto_ptr< uchar > x28_dmaBufferA;
  rstl::auto_ptr< uchar > x30_dmaBufferB;
  rstl::vector< rstl::auto_ptr< CDvdRequest > > x38_dvdRequests;
  rstl::vector< rstl::auto_ptr< uchar > > x48_buffers;
  g72x_state x58_leftState;
  g72x_state x8c_rightState;
  ushort xc0_volume;
};

CHECK_SIZEOF(CStaticAudioPlayer, 0xC4)

#endif // _CSTATICAUDIOPLAYER
