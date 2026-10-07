#include "Kyoto/Audio/CBitcrusher.hpp"
#include "Kyoto/Math/CMath.hpp"

#include <math.h>
#include <stdio.h>
#include <string.h>

CBitcrusher::CBitcrusher(AudioMasterCallback audioMaster)
: AudioEffectX(audioMaster, 1, 4)
, mDistortionType(0.f)
, mGain(1.f)
, mBitDepth(0.f)
, mSampleRateReduction(1.f) {
  mHeldSamples[1] = 0.f;
  mHeldSamples[0] = 0.f;
  mSampleCounter = 0;
  // The native constructor does not initialize the surround held sample.
  canMono(true);
  setNumInputs(2);
  setNumOutputs(2);
  setUniqueID('Bits');
  canProcessReplacing(true);
  strcpy(mProgramName, "Bitcrusher VST");
}

CBitcrusher::~CBitcrusher() {}

void CBitcrusher::setProgramName(char* name) { strcpy(mProgramName, name); }

void CBitcrusher::getProgramName(char* name) { strcpy(name, mProgramName); }

void CBitcrusher::setParameter(long index, float value) {
  switch (index) {
  case 0:
    mDistortionType = value;
    break;
  case 1:
    mGain = value;
    break;
  case 2:
    mBitDepth = value;
    break;
  case 3:
    mSampleRateReduction = value;
    break;
  }
}

float CBitcrusher::getParameter(long index) {
  float ret = 0.f;
  switch (index) {
  case 0:
    ret = mDistortionType;
    break;
  case 1:
    ret = mGain;
    break;
  case 2:
    ret = mBitDepth;
    break;
  case 3:
    ret = mSampleRateReduction;
    break;
  }
  return ret;
}

void CBitcrusher::getParameterName(long index, char* name) {
  switch (index) {
  case 0:
    strcpy(name, "Dist type");
    break;
  case 1:
    strcpy(name, "Gain");
    break;
  case 2:
    strcpy(name, "Bit-depth");
    break;
  case 3:
    strcpy(name, "Downsampling");
    break;
  }
}

// The native calls pass text as a stray trailing vararg.
void CBitcrusher::getParameterDisplay(long index, char* text) {
  switch (index) {
  case 0:
    if (mDistortionType < 0.33)
      sprintf(text, "%i", 1, text);
    if (mDistortionType >= 0.33 && mDistortionType <= 0.66)
      sprintf(text, "%i", 2, text);
    if (mDistortionType > 0.66)
      sprintf(text, "%i", 3, text);
    break;
  case 1:
    sprintf(text, "%f", 20.0 * static_cast< float >(log10(-16.f * mGain + 17.f)), text);
    break;
  case 2:
    sprintf(text, "%i", static_cast< int >(-23.f * mBitDepth + 24.f));
    break;
  case 3:
    sprintf(text, "%i", static_cast< int >(-39.f * mSampleRateReduction + 40.f));
    break;
  }
}

void CBitcrusher::getParameterLabel(long index, char* label) {
  switch (index) {
  case 0:
    strcpy(label, "");
    break;
  case 1:
    strcpy(label, "dB");
    break;
  case 2:
    strcpy(label, "Bits");
    break;
  case 3:
    strcpy(label, "x");
    break;
  }
}

void CBitcrusher::process(float** inputs, float** outputs, long sampleFrames) {}

void CBitcrusher::processReplacing(float** inputs, float** outputs, long sampleFrames) {
  float* input[3] = {inputs[0], inputs[1], inputs[2]};
  float* output[3] = {outputs[0], outputs[1], outputs[2]};
  const float quantization = powf(2.f, -23.f * mBitDepth + 24.f);
  const float reciprocal = 1.f / quantization;
  const float gain = -16.f * mGain + 17.f;

  // The middle distortion mode and accumulating callback are native no-ops.
  if (mDistortionType < 0.33) {
    while (--sampleFrames >= 0) {
      float samples[3];
      for (int channel = 0; channel < 3; ++channel)
        samples[channel] = CMath::Clamp(-1.f, gain * *input[channel]++, 1.f);

      if (static_cast< float >(mSampleCounter) < -39.f * mSampleRateReduction + 39.f) {
        ++mSampleCounter;
      } else {
        for (int channel = 0; channel < 3; ++channel)
          mHeldSamples[channel] = samples[channel];
        mSampleCounter = 0;
      }
      for (int channel = 0; channel < 3; ++channel)
        *output[channel]++ =
            reciprocal *
            static_cast< float >(static_cast< int >(mHeldSamples[channel] * quantization));
    }
  }
  if (mDistortionType > 0.66) {
    while (--sampleFrames >= 0) {
      float samples[3];
      for (int channel = 0; channel < 3; ++channel)
        samples[channel] = gain * *input[channel]++;
      samples[0] = CMath::FastSinR(samples[0]);
      samples[1] = CMath::FastSinR(samples[1]);
      // The native surround path omits sine distortion.
      if (static_cast< float >(mSampleCounter) < -39.f * mSampleRateReduction + 39.f) {
        ++mSampleCounter;
      } else {
        for (int channel = 0; channel < 3; ++channel)
          mHeldSamples[channel] = samples[channel];
        mSampleCounter = 0;
      }
      for (int channel = 0; channel < 3; ++channel)
        *output[channel]++ =
            reciprocal *
            static_cast< float >(static_cast< int >(mHeldSamples[channel] * quantization));
    }
  }
}
