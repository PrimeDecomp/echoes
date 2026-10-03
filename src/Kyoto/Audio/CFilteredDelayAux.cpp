#include "Kyoto/Audio/CAuxEffectParameters.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"

// Guessed backend names; the three-channel state and filtering operations are target-derived.
void PrepareFilteredDelayAux(SFilteredDelayAuxParameters* parameters) {
  for (int channel = 0; channel < 3; ++channel) {
    const int blockCount = (parameters->mDelayMs[channel] * 32000u) / 1000u / 160u - 1;
    parameters->mBlockCounts[channel] = blockCount > 1 ? blockCount : 1;
  }

  for (int channel = 0; channel < 3; ++channel) {
    parameters->mCurrentFeedback[channel] = (parameters->mFeedbackPercent[channel] << 7) / 100;
    parameters->mCurrentOutput[channel] = (parameters->mOutputPercent[channel] << 7) / 100;
    parameters->mBlockPositions[channel] = 0;
  }

  parameters->mLowPassCoefficient =
      static_cast< s32 >(128.f * (parameters->mLowPassFrequency / 32000.f));
  parameters->mHighPassCoefficient =
      static_cast< s32 >(128.f * (parameters->mHighPassFrequency / 32000.f));
  for (int channel = 0; channel < 3; ++channel) {
    parameters->mLowPassHistory[channel] = 0;
    parameters->mHighPassHistory[channel] = 0;
  }

  for (int channel = 0; channel < 3; ++channel) {
    parameters->mDelayBuffers[channel] = rs_new s32[parameters->mBlockCounts[channel] * 160];
  }

  for (int channel = 0; channel < 3; ++channel) {
    CBasics::ZeroMemory(parameters->mDelayBuffers[channel],
                        parameters->mBlockCounts[channel] * 160 * sizeof(s32));
  }
}

void ShutdownFilteredDelayAux(SFilteredDelayAuxParameters* parameters) {
  for (int channel = 0; channel < 3; ++channel) {
    CMemory::Free(parameters->mDelayBuffers[channel]);
  }
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
  for (int channel = 0; channel < 3; ++channel) {
    const s32 output = parameters->mCurrentOutput[channel];
    const s32 feedback = parameters->mCurrentFeedback[channel];
    if (output != 0) {
      s32* delay = delayBuffers[channel] + parameters->mBlockPositions[channel] * 160;
      for (int sample = 0; sample < 160; ++sample) {
        const s32 delayed = delay[sample];
        const s32 input = channels[channel][sample];
        parameters->mLowPassHistory[channel] =
            (delayed * lowPass + parameters->mLowPassHistory[channel] * (128 - lowPass)) >> 7;
        parameters->mHighPassHistory[channel] =
            (parameters->mLowPassHistory[channel] * highPass +
             parameters->mHighPassHistory[channel] * (128 - highPass)) >>
            7;
        channels[channel][sample] =
            input +
            ((parameters->mLowPassHistory[channel] - parameters->mHighPassHistory[channel]) *
                 output >>
             7);
        delay[sample] = input + (delayed * feedback >> 7);
      }

      ++parameters->mBlockPositions[channel];
      if (static_cast< s32 >(parameters->mBlockPositions[channel]) >=
          static_cast< s32 >(parameters->mBlockCounts[channel])) {
        parameters->mBlockPositions[channel] = 0;
      }
    }
  }
}
