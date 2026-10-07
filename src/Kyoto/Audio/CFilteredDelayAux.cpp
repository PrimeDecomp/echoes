#include "Kyoto/Audio/CAuxEffectParameters.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"

// Guessed backend names; the three-channel state and filtering operations are target-derived.
void PrepareFilteredDelayAux(SFilteredDelayAuxParameters* parameters) {
  {
    const int blockCount = (parameters->mDelayMs[0] * 32000u) / 1000u / 160u - 1;
    parameters->mBlockCounts[0] = blockCount > 1 ? blockCount : 1;
  }
  {
    const int blockCount = (parameters->mDelayMs[1] * 32000u) / 1000u / 160u - 1;
    parameters->mBlockCounts[1] = blockCount > 1 ? blockCount : 1;
  }
  {
    const int blockCount = (parameters->mDelayMs[2] * 32000u) / 1000u / 160u - 1;
    parameters->mBlockCounts[2] = blockCount > 1 ? blockCount : 1;
  }
  parameters->mCurrentFeedback[0] = (parameters->mFeedbackPercent[0] << 7) / 100;
  parameters->mCurrentFeedback[1] = (parameters->mFeedbackPercent[1] << 7) / 100;
  parameters->mCurrentFeedback[2] = (parameters->mFeedbackPercent[2] << 7) / 100;
  parameters->mCurrentOutput[0] = (parameters->mOutputPercent[0] << 7) / 100;
  parameters->mCurrentOutput[1] = (parameters->mOutputPercent[1] << 7) / 100;
  parameters->mCurrentOutput[2] = (parameters->mOutputPercent[2] << 7) / 100;
  parameters->mBlockPositions[0] = 0;
  parameters->mBlockPositions[1] = 0;
  parameters->mBlockPositions[2] = 0;
  parameters->mLowPassCoefficient =
      static_cast< s32 >(128.f * (parameters->mLowPassFrequency / 32000.f));
  parameters->mHighPassCoefficient =
      static_cast< s32 >(128.f * (parameters->mHighPassFrequency / 32000.f));
  parameters->mLowPassHistory[0] = 0;
  parameters->mLowPassHistory[1] = 0;
  parameters->mLowPassHistory[2] = 0;
  parameters->mHighPassHistory[0] = 0;
  parameters->mHighPassHistory[1] = 0;
  parameters->mHighPassHistory[2] = 0;
  parameters->mDelayBuffers[0] = rs_new s32[parameters->mBlockCounts[0] * 160];
  parameters->mDelayBuffers[1] = rs_new s32[parameters->mBlockCounts[1] * 160];
  parameters->mDelayBuffers[2] = rs_new s32[parameters->mBlockCounts[2] * 160];
  CBasics::ZeroMemory(parameters->mDelayBuffers[0],
                      parameters->mBlockCounts[0] * 160 * sizeof(s32));
  CBasics::ZeroMemory(parameters->mDelayBuffers[1],
                      parameters->mBlockCounts[1] * 160 * sizeof(s32));
  CBasics::ZeroMemory(parameters->mDelayBuffers[2],
                      parameters->mBlockCounts[2] * 160 * sizeof(s32));
}

void ShutdownFilteredDelayAux(SFilteredDelayAuxParameters* parameters) {
  CMemory::Free(parameters->mDelayBuffers[0]);
  CMemory::Free(parameters->mDelayBuffers[1]);
  CMemory::Free(parameters->mDelayBuffers[2]);
}

void ProcessFilteredDelayAux(uchar reason, SND_AUX_INFO* info,
                             SFilteredDelayAuxParameters* parameters) {
  if (reason != SND_AUX_REASON_BUFFERUPDATE) {
    return;
  }

  s32* channels[3] = {info->data.bufferUpdate.left, info->data.bufferUpdate.right,
                      info->data.bufferUpdate.surround};
  s32* delayBuffers[3] = {parameters->mDelayBuffers[0], parameters->mDelayBuffers[1],
                          parameters->mDelayBuffers[2]};
  const s32 lowPass = parameters->mLowPassCoefficient;
  const s32 highPass = parameters->mHighPassCoefficient;
  const s32 lowPassInv = 128 - lowPass;
  const s32 highPassInv = 128 - highPass;
  for (int channel = 0; channel < 3; ++channel) {
    const s32 output = parameters->mCurrentOutput[channel];
    const s32 feedback = parameters->mCurrentFeedback[channel];
    if (output != 0) {
      s32* samples = channels[channel];
      s32* delay = delayBuffers[channel] + parameters->mBlockPositions[channel] * 160;
      for (int sample = 0; sample < 160; ++sample) {
        const s32 delayed = *delay;
        const s32 input = *samples;
        parameters->mLowPassHistory[channel] =
            (delayed * lowPass + parameters->mLowPassHistory[channel] * lowPassInv) >> 7;
        parameters->mHighPassHistory[channel] =
            (parameters->mLowPassHistory[channel] * highPass +
             parameters->mHighPassHistory[channel] * highPassInv) >>
            7;
        const s32 band =
            parameters->mLowPassHistory[channel] - parameters->mHighPassHistory[channel];
        *samples++ = input + (band * output >> 7);
        *delay++ = input + (delayed * feedback >> 7);
      }

      ++parameters->mBlockPositions[channel];
      if (static_cast< s32 >(parameters->mBlockPositions[channel]) >=
          static_cast< s32 >(parameters->mBlockCounts[channel])) {
        parameters->mBlockPositions[channel] = 0;
      }
    }
  }
}
