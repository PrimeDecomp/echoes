#include "Kyoto/Audio/CAuxEffectParameters.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Audio/CBitcrusher.hpp"
#include "Kyoto/Audio/CFlanger.hpp"
#include "Kyoto/Audio/CPhaser.hpp"
#include "Kyoto/Basics/CCast.hpp"

static const int kBufferSamples = 160;
static const float kSampleScale = 16777215.f;
static const float kInvSampleScale = 1.f / kSampleScale;

// Guessed name; the host callback returns one for every request.
static long CustomAuxAudioMaster(AEffect* effect, long opcode, long index, long value, void* ptr,
                                 float option) {
  return 1;
}

void ProcessCustomAux(uchar reason, SND_AUX_INFO* info, SAuxEffectProcessingState* processing) {
  if (reason != SND_AUX_REASON_BUFFERUPDATE)
    return;

  float* inputs[3] = {processing->mLeftBuffer, processing->mRightBuffer,
                      processing->mSurroundBuffer};
  float* outputs[3] = {processing->mLeftBuffer, processing->mRightBuffer,
                       processing->mSurroundBuffer};
  for (int sample = 0; sample < kBufferSamples; ++sample) {
    processing->mLeftBuffer[sample] =
        kInvSampleScale * CCast::LtoF(info->data.bufferUpdate.left[sample]);
    processing->mRightBuffer[sample] =
        kInvSampleScale * CCast::LtoF(info->data.bufferUpdate.right[sample]);
    processing->mSurroundBuffer[sample] =
        kInvSampleScale * CCast::LtoF(info->data.bufferUpdate.surround[sample]);
  }

  AEffect* effect = processing->mEffectDescriptor;
  if (processing->mProcessReplacing)
    effect->mProcessReplacing(effect, inputs, outputs, kBufferSamples);
  else
    effect->mProcess(effect, inputs, outputs, kBufferSamples);

  for (int sample = 0; sample < kBufferSamples; ++sample) {
    info->data.bufferUpdate.left[sample] =
        CCast::FtoL(kSampleScale * processing->mLeftBuffer[sample]);
    info->data.bufferUpdate.right[sample] =
        CCast::FtoL(kSampleScale * processing->mRightBuffer[sample]);
    info->data.bufferUpdate.surround[sample] =
        CCast::FtoL(kSampleScale * processing->mSurroundBuffer[sample]);
  }
}

static inline void PrepareProcessingState(SAuxEffectProcessingState& processing,
                                          AudioEffect* processor) {
  processing.mProcessor = processor;
  processing.mEffectDescriptor = processor->AudioEffect::getAeffect();
  processing.mLeftBuffer = rs_new float[kBufferSamples];
  processing.mRightBuffer = rs_new float[kBufferSamples];
  processing.mSurroundBuffer = rs_new float[kBufferSamples];
}

static inline bool ShutdownProcessingState(SAuxEffectProcessingState& processing) {
  delete processing.mProcessor;
  delete[] processing.mLeftBuffer;
  delete[] processing.mRightBuffer;
  delete[] processing.mSurroundBuffer;
  return true;
}

bool PrepareFlangerAux(SFlangerAuxParameters* parameters) {
  AudioEffect* processor = rs_new CFlanger(CustomAuxAudioMaster);
  PrepareProcessingState(parameters->mProcessing, processor);
  processor->setParameter(0, parameters->mDelay);
  processor->setParameter(7, parameters->mDelayPhase);
  processor->setParameter(6, parameters->mDry);
  processor->setParameter(1, parameters->mFeedback);
  processor->setParameter(4, parameters->mLFODepth);
  processor->setParameter(3, parameters->mLFOFrequency);
  processor->setParameter(5, parameters->mLFOWave);
  processor->setParameter(2, parameters->mOut);
  return true;
}

bool ShutdownFlangerAux(SFlangerAuxParameters* parameters) {
  return ShutdownProcessingState(parameters->mProcessing);
}

bool PrepareBitcrusherAux(SBitcrusherAuxParameters* parameters) {
  AudioEffect* processor = rs_new CBitcrusher(CustomAuxAudioMaster);
  PrepareProcessingState(parameters->mProcessing, processor);
  processor->setParameter(0, parameters->mDistortionType);
  processor->setParameter(1, parameters->mGain);
  processor->setParameter(2, parameters->mBitDepth);
  processor->setParameter(3, parameters->mSampleRateReduction);
  return true;
}

bool ShutdownBitcrusherAux(SBitcrusherAuxParameters* parameters) {
  return ShutdownProcessingState(parameters->mProcessing);
}

bool PreparePhaserAux(SPhaserAuxParameters* parameters) {
  AudioEffect* processor = rs_new CPhaser(CustomAuxAudioMaster);
  PrepareProcessingState(parameters->mProcessing, processor);
  processor->setParameter(0, parameters->mFrequency);
  processor->setParameter(1, parameters->mFeedback);
  processor->setParameter(2, parameters->mInvert);
  processor->setParameter(3, parameters->mWet);
  processor->setParameter(4, parameters->mDry);
  processor->setParameter(5, parameters->mSweep);
  return true;
}

bool ShutdownPhaserAux(SPhaserAuxParameters* parameters) {
  return ShutdownProcessingState(parameters->mProcessing);
}
