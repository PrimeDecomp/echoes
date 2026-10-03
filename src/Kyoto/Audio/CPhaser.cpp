#include "Kyoto/Audio/CPhaser.hpp"
#include "Kyoto/Math/CMath.hpp"

#include <math.h>
#include <string.h>

void CPhaser::ProcessSamples(float** inputs, float** outputs, long sampleFrames, bool replacing) {
  float* input[3] = {inputs[0], inputs[1], inputs[2]};
  float* output[3] = {outputs[0], outputs[1], outputs[2]};
  while (--sampleFrames >= 0) {
    for (int channel = 0; channel < 3; ++channel) {
      SFilterCoefficient& filter = mCoefficients[channel];
      filter.mAngularFrequencyRatio = 2.0 * M_PI * mChannelFrequency[channel] / mSampleRate;
      filter.mCoefficient =
          (1.0 - filter.mAngularFrequencyRatio) / (1.0 + filter.mAngularFrequencyRatio);
    }
    if (mInvert > 0.5) {
      mFeedbackSign = 1.0;
      mInvert = 1.f;
    } else {
      mFeedbackSign = -1.0;
      mInvert = 0.f;
    }
    // Snapshot all inputs before writing outputs, including aliased buffers.
    for (int channel = 0; channel < 3; ++channel)
      mInputs[channel].mRawInput = *input[channel]++;
    for (int channel = 0; channel < 3; ++channel)
      mInputs[channel].mFeedbackInput =
          mFeedbackSign * 0.999999 * mFeedback * (1.2e-7 + mHistory[channel].mOutputs[3]) +
          mInputs[channel].mRawInput;

    for (int channel = 0; channel < 3; ++channel) {
      SFilterHistory& history = mHistory[channel];
      double value = mInputs[channel].mFeedbackInput;
      for (int stage = 0; stage < 4; ++stage) {
        history.mOutputs[stage] =
            mCoefficients[channel].mCoefficient * (value + history.mOutputs[stage]) -
            history.mInputs[stage];
        // Native surround stages 0/1 repeat right-channel stores instead of advancing
        // surround input history. Those right-channel values are already stored.
        if (channel != 2 || stage >= 2)
          history.mInputs[stage] = value;
        value = history.mOutputs[stage];
      }
    }

    mLFORate = 0.02 + (replacing ? 5.98 : 3.98) * mFrequency;
    mLFOPhase += mLFORate * mLFOPhaseStep;
    while (mLFOPhase >= 2048.0)
      mLFOPhase -= 2048.0;
    while (mLFOPhase < 0.0)
      mLFOPhase += 2048.0;
    mLFOIndex = static_cast< int >(mLFOPhase);
    mLFOFraction = mLFOPhase - mLFOIndex;
    mLeftLFO =
        mSineTable[mLFOIndex] + mLFOFraction * (mSineTable[mLFOIndex + 1] - mSineTable[mLFOIndex]);
    mRightLFO = -mLeftLFO;
    mSurroundLFO = mLeftLFO;

    for (int channel = 0; channel < 3; ++channel) {
      const float mixed = static_cast< float >(mInputs[channel].mRawInput * mDry +
                                               mHistory[channel].mOutputs[3] * mWet);
      if (replacing)
        *output[channel]++ = mixed;
      else
        *output[channel]++ += mixed;
    }
    mLeftLFO = (1.0 + mLeftLFO) * 0.5;
    mRightLFO = (1.0 + mRightLFO) * 0.5;
    const float sweep = 100.f + 7800.f * mSweepRange;
    mChannelFrequency[0] = mBaseFrequency + mLeftLFO * sweep;
    mChannelFrequency[1] = mBaseFrequency + mRightLFO * sweep;
    mChannelFrequency[2] = mBaseFrequency + mSurroundLFO * sweep;
  }
}

CPhaser::CPhaser(AudioMasterCallback audioMaster)
: AudioEffectX(audioMaster, 8, 6)
, mFrequency(0.2f)
, mFeedback(0.7f)
, mInvert(1.f)
, mWet(0.5f)
, mDry(0.5f)
, mSweepRange(0.75f)
, x1e8_(0.0)
, x1f0_(0.0)
, x1f8_(0.0)
, mBaseFrequency(100.0)
, mFeedbackSign(1.0)
, mInitialFeedback(mFeedback)
, mLFORate(0.0)
, mLFOPhaseStep(2048.0 / mSampleRate)
, mLFOPhase(0.0)
, mFullCircle(8.0 * atan(1.0)) {
  for (int channel = 0; channel < 3; ++channel) {
    for (int stage = 0; stage < 4; ++stage) {
      mHistory[channel].mOutputs[stage] = 0.0;
      mHistory[channel].mInputs[stage] = 0.0;
    }
    mChannelFrequency[channel] = mBaseFrequency;
  }
  for (mTableBuildIndex = 0; mTableBuildIndex < 2049; ++mTableBuildIndex)
    mSineTable[mTableBuildIndex] = sin(mFullCircle * mTableBuildIndex / 2048.0);

  setNumInputs(2);
  setNumOutputs(2);
  setUniqueID('SSPH');
  canProcessReplacing(true);
  canMono(true);
  strcpy(mProgramName, "SSPH");
}

CPhaser::~CPhaser() {}

bool CPhaser::getVendorString(char* text) {
  strcpy(text, "SureShotStudio");
  return true;
}

bool CPhaser::getEffectName(char* text) {
  strcpy(text, "SureShot Phaser");
  return true;
}

bool CPhaser::getProductString(char* text) {
  strcpy(text, "My very first VST Plug, a Phaser");
  return true;
}

void CPhaser::setProgramName(char* name) { strcpy(mProgramName, name); }

void CPhaser::getProgramName(char* name) { strcpy(name, mProgramName); }

void CPhaser::setParameter(long index, float value) {
  switch (index) {
  case 0:
    mFrequency = value;
    break;
  case 1:
    mFeedback = value;
    break;
  case 2:
    mInvert = value;
    break;
  case 3:
    mWet = value;
    break;
  case 4:
    mDry = value;
    break;
  case 5:
    mSweepRange = value;
    break;
  }
}

float CPhaser::getParameter(long index) {
  switch (index) {
  case 0:
    return mFrequency;
  case 1:
    return mFeedback;
  case 2:
    return mInvert;
  case 3:
    return mWet;
  case 4:
    return mDry;
  case 5:
    return mSweepRange;
  default:
    return 0.f;
  }
}

void CPhaser::getParameterName(long index, char* name) {
  switch (index) {
  case 0:
    strcpy(name, "Freq");
    break;
  case 1:
    strcpy(name, "Feedback");
    break;
  case 2:
    strcpy(name, "Invert");
    break;
  case 3:
    strcpy(name, "Wetmix");
    break;
  case 4:
    strcpy(name, "Drymix");
    break;
  case 5:
    strcpy(name, "SwpRange");
    break;
  }
}

void CPhaser::getParameterDisplay(long index, char* text) {
  switch (index) {
  case 0:
    float2string(3.98f * mFrequency + 0.02000001f, text);
    break;
  case 1:
    float2string(0.999999f * mFeedback, text);
    break;
  case 2:
    float2string(2.f * mInvert - 1.f, text);
    break;
  case 3:
    dB2string(mWet, text);
    break;
  case 4:
    dB2string(mDry, text);
    break;
  case 5:
    float2string(7800.f * mSweepRange + 200.f, text);
    break;
  }
}

void CPhaser::getParameterLabel(long index, char* label) {
  switch (index) {
  case 0:
    strcpy(label, "Hz");
    break;
  case 1:
    strcpy(label, "Amount");
    break;
  case 2:
    strcpy(label, "Fase");
    break;
  case 3:
  case 4:
    strcpy(label, "dB");
    break;
  case 5:
    strcpy(label, "Size");
    break;
  }
}

void CPhaser::process(float** inputs, float** outputs, long sampleFrames) {
  ProcessSamples(inputs, outputs, sampleFrames, false);
}

void CPhaser::processReplacing(float** inputs, float** outputs, long sampleFrames) {
  ProcessSamples(inputs, outputs, sampleFrames, true);
}
