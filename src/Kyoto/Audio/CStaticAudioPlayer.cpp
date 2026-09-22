#include "Kyoto/Audio/CAudioSys.hpp"
#include "rstl/algorithm.hpp"
#include <Kyoto/Alloc/CMemory.hpp>
#include <Kyoto/Audio/CStaticAudioPlayer.hpp>

#include <Kyoto/CDvdFile.hpp>
#include <Kyoto/CDvdRequest.hpp>

#include <rstl/math.hpp>

#include <dolphin/ai.h>
#include <dolphin/os.h>
#include <stdint.h>

class CInterruptGuard {
  bool x0_enabled;

public:
  CInterruptGuard() : x0_enabled(OSDisableInterrupts()) {}
  ~CInterruptGuard() { OSRestoreInterrupts(x0_enabled); }
};

static CStaticAudioPlayer* sCurrentPlayer = nullptr;
static rstl::reserved_vector< FAudioCallback, 4 > sAICallbacks;
static bool sDMACallbackInstalled ATTRIBUTE_ALIGN(8) = false;
static FAudioCallback sOldDMACallback = nullptr;

void CStaticAudioPlayer::InstallAICallback() {
  bool old = CAudioSys::IsAICallbackEnabled();
  CAudioSys::EnableAICallback(true);

  if (!sDMACallbackInstalled && sAICallbacks.size() != 0) {
    sOldDMACallback = AIRegisterDMACallback(AICallback);
    sDMACallbackInstalled = true;
  } else if (sDMACallbackInstalled && sAICallbacks.size() == 0) {
    AIRegisterDMACallback(sOldDMACallback);
    sOldDMACallback = 0;
    sDMACallbackInstalled = false;
  }

  CAudioSys::EnableAICallback(old);
}

void CStaticAudioPlayer::AICallback() {
  sOldDMACallback();

  for (int i = 0; i < sAICallbacks.size(); ++i) {
    sAICallbacks[i]();
  }
}

void CStaticAudioPlayer::RunDMACallback(const FAudioCallback callback) {
  CInterruptGuard interrupts;
  const rstl::reserved_vector< FAudioCallback, 4 >::iterator it =
      rstl::find(sAICallbacks.begin(), sAICallbacks.end(), callback);
  if (it == sAICallbacks.end()) {
    sAICallbacks.push_back(callback);
  }

  InstallAICallback();
}

void CStaticAudioPlayer::CancelDMACallback(FAudioCallback callback) {
  CInterruptGuard interrupts;

  const rstl::reserved_vector< FAudioCallback, 4 >::iterator it =
      rstl::find(sAICallbacks.begin(), sAICallbacks.end(), callback);
  if (it != sAICallbacks.end()) {
    sAICallbacks.erase(it);
  }

  InstallAICallback();
}

CStaticAudioPlayer::CStaticAudioPlayer(const rstl::string& filepath, const int loopStart,
                                       const int loopEnd)
: x0_filepath(filepath)
, x10_rsfRem(-1)
, x18_curSamp(0)
, x1c_loopStartSamp(loopStart & ~1)
, x20_loopEndSamp(loopEnd & ~1)
, x24_curBuf(0)
, x28_dmaBufferA(static_cast< uchar* >(CMemory::Alloc(640, IAllocator::kHI_RoundUpLen)))
, x30_dmaBufferB(static_cast< uchar* >(CMemory::Alloc(640, IAllocator::kHI_RoundUpLen)))
, xc0_volume(32768) {
  CDvdFile dvdFile(filepath.data());
  x10_rsfRem = dvdFile.GetFileSize();
  x14_rsfLength = x10_rsfRem;
  int bufferCount = ((x10_rsfRem - 1) + 0x4000) / 0x4000;
  x48_buffers.reserve(bufferCount);
  x38_dvdRequests.reserve(bufferCount);

  for (int i = x10_rsfRem; i > 0; i -= 0x4000) {
    uint bufferSize = 0x4000;
    if (i <= 0x4000) {
      bufferSize = (i + 31) & ~31;
    }

    rstl::auto_ptr< uchar > buf(
        static_cast< uchar* >(CMemory::Alloc(bufferSize, IAllocator::kHI_RoundUpLen)));
    x48_buffers.push_back_unsafe(buf);
    x38_dvdRequests.push_back_unsafe(dvdFile.SyncRead(buf.get(), bufferSize));
  }
}

CStaticAudioPlayer::~CStaticAudioPlayer() { StopMixOut(); }

const bool CStaticAudioPlayer::IsReady() const {
  return !x38_dvdRequests.empty() ? x38_dvdRequests.back()->IsComplete() : true;
}

void CStaticAudioPlayer::StartMixOut() {
  if (sCurrentPlayer == this) {
    return;
  }

  x38_dvdRequests = rstl::vector< rstl::auto_ptr< CDvdRequest > >();
  x18_curSamp = 0;
  g72x_init_state(&x58_leftState);
  g72x_init_state(&x8c_rightState);
  sCurrentPlayer = this;
  RunDMACallback(MixCallback);
}

void CStaticAudioPlayer::StopMixOut() {
  if (sCurrentPlayer == this) {
    CancelDMACallback(MixCallback);
    sCurrentPlayer = nullptr;
  }
}

void CStaticAudioPlayer::MixCallback() { sCurrentPlayer->DoMix(); }

void CStaticAudioPlayer::DoMix() {
  const ushort* aiStart = static_cast< const ushort* >(OSPhysicalToCached(AIGetDMAStartAddr()));
  x24_curBuf ^= 1;
  uintptr_t buf =
      reinterpret_cast< uintptr_t >(x24_curBuf != 0 ? x30_dmaBufferB.get() : x28_dmaBufferA.get());

  AIInitDMA(buf, 0x280);
  u32 cookie = OSEnableInterrupts();
  if (aiStart != 0) {
    DCInvalidateRange(const_cast< ushort* >(aiStart), 0x280);
  }

  Decode(reinterpret_cast< ushort* >(buf), aiStart, 160);
  DCFlushRange(reinterpret_cast< void* >(buf), 0x280);
  OSRestoreInterrupts(cookie);
}

static void MixToMono(ushort* data, int numSamples) {
  short* samples = reinterpret_cast< short* >(data);
  for (int i = 0; i < numSamples * 2; i += 2) {
    int sample = (samples[0] + samples[1]) / 2;
    short clamped;
    if (sample < -32768) {
      clamped = -32768;
    } else if (sample > 32767) {
      clamped = 32767;
    } else {
      clamped = sample;
    }
    samples[0] = clamped;
    samples[1] = clamped;
    samples += 2;
  }
}

void CStaticAudioPlayer::Decode(ushort* out, const ushort* in, int numSamples) {
  int curSamp = x18_curSamp / 2;
  int loopEndSamp = x20_loopEndSamp / 2;
  int loopStartSamp = x1c_loopStartSamp / 2;
  DecodeMonoAndMix(out, in, numSamples, curSamp, loopEndSamp, loopStartSamp, xc0_volume,
                   x58_leftState);

  int halfLen = x14_rsfLength / 2;
  DecodeMonoAndMix(out + 1, in + 1, numSamples, curSamp + halfLen, loopEndSamp + halfLen,
                   loopStartSamp + halfLen, xc0_volume, x8c_rightState);

  if (CAudioSys::GetSurroundMode() == CAudioSys::kSM_Mono) {
    MixToMono(out, numSamples);
  }

  int remSamples = numSamples;
  while (remSamples != 0) {
    int remTillLoop = x20_loopEndSamp - x18_curSamp;
    int rs = remSamples;
    int consumed = rstl::min_val(rs, remTillLoop);
    x18_curSamp += consumed;
    remSamples -= consumed;
    if (x18_curSamp == x20_loopEndSamp) {
      x18_curSamp = x1c_loopStartSamp;
    }
  }
}

void CStaticAudioPlayer::DecodeMonoAndMix(ushort* out, const ushort* in, int numSamples,
                                          int startSample, int sampleEnd, int sampleStart, int vol,
                                          g72x_state& state) {
  ushort* outCursor = out;
  const ushort* inCursor = in;
  int curSample = startSample;
  for (int remBytes = numSamples / 2; remBytes != 0;) {
    int rb = remBytes;
    int curBuf = curSample / 0x4000;
    int thisBytes = ((curBuf + 1) * 0x4000) - curSample;
    thisBytes = rstl::min_val(rb, thisBytes);

    int remTillLoop = sampleEnd - curSample;
    thisBytes = rstl::min_val(thisBytes, remTillLoop);

    uchar* byte = x48_buffers[curBuf].get() + (curSample - (curBuf * 0x4000));
    int i = 0;
    while (i < thisBytes) {
      int samp1 = reinterpret_cast< const short* >(inCursor)[0] +
                  ((vol * g721_decoder(*byte & 0xf, &state)) >> 15);
      int samp2 = reinterpret_cast< const short* >(inCursor)[2] +
                  ((vol * g721_decoder(*byte >> 4, &state)) >> 15);

      short clamped1;
      if (samp1 < -0x8000) {
        clamped1 = -0x8000;
      } else if (samp1 > 0x7fff) {
        clamped1 = 0x7fff;
      } else {
        clamped1 = samp1;
      }
      outCursor[0] = clamped1;

      short clamped2;
      if (samp2 < -0x8000) {
        clamped2 = -0x8000;
      } else if (samp2 > 0x7fff) {
        clamped2 = 0x7fff;
      } else {
        clamped2 = samp2;
      }
      outCursor[2] = clamped2;

      outCursor += 4;
      ++byte;
      inCursor += 4;
      ++i;
    }

    curSample += thisBytes;
    remBytes -= thisBytes;
    if (curSample == sampleEnd) {
      curSample = sampleStart;
    }
  }
}

void CStaticAudioPlayer::SetVolume(uchar vol) {
  if (static_cast< uchar >(vol) > 127) {
    vol = 127;
  }
  xc0_volume = CAudioSys::kVolumeTable[static_cast< uchar >(vol)];
}
