#include "Kyoto/Audio/CPhaser.hpp"
#include "Kyoto/Math/CMath.hpp"

#include <math.h>
#include <string.h>

CPhaser::CPhaser(AudioMasterCallback audioMaster)
: AudioEffectX(audioMaster, 8, 6)
, mFrequency(0.2f)
, mFeedback(0.7f)
, mInvert(1.f)
, mWet(0.5f)
, mDry(0.5f)
, mSweepRange(0.75f) {
  for (int channel = 0; channel < 3; ++channel) {
    for (int stage = 0; stage < 4; ++stage) {
      mHistory[channel].mOutputs[stage] = 0.0;
    }
    for (int stage = 0; stage < 4; ++stage) {
      mHistory[channel].mInputs[stage] = 0.0;
    }
  }
  x1e8_ = 0.0;
  x1f0_ = 0.0;
  x1f8_ = 0.0;
  mBaseFrequency = 100.0;
  for (int channel = 0; channel < 3; ++channel) {
    mChannelFrequency[channel] = mBaseFrequency;
  }
  mFeedbackSign = 1.0;
  mInitialFeedback = mFeedback;
  mLFORate = 0.0;
  mLFOPhaseStep = 2048.0 / mSampleRate;
  mLFOPhase = 0.0;
  mFullCircle = 8.0 * atan(1.0);
  for (mTableBuildIndex = 0; mTableBuildIndex < 2049; ++mTableBuildIndex)
    mSineTable[mTableBuildIndex] = sin(mFullCircle * mTableBuildIndex / 2048.0);

  setNumInputs(2);
  setNumOutputs(2);
  setUniqueID('SSPH');
  canMono(true);
  canProcessReplacing(true);
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
  float v = 0.f;
  switch (index) {
  case 0:
    v = mFrequency;
    break;
  case 1:
    v = mFeedback;
    break;
  case 2:
    v = mInvert;
    break;
  case 3:
    v = mWet;
    break;
  case 4:
    v = mDry;
    break;
  case 5:
    v = mSweepRange;
    break;
  }
  return v;
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
    float2string(2.f * mInvert + -1.f, text);
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
    strcpy(label, "dB");
    break;
  case 4:
    strcpy(label, "dB");
    break;
  case 5:
    strcpy(label, "Size");
    break;
  }
}

void CPhaser::process(float** inputs, float** outputs, long sampleFrames) {
  float* in1 = inputs[0];
  float* in2 = inputs[1];
  float* in3 = inputs[2];
  float* out1 = outputs[0];
  float* out2 = outputs[1];
  float* out3 = outputs[2];
  while (--sampleFrames >= 0) {
    for (int channel = 0; channel < 3; ++channel) {
      mCoefficients[channel].mAngularFrequencyRatio =
          2.0 * M_PI * mChannelFrequency[channel] / mSampleRate;
      mCoefficients[channel].mCoefficient = (1.0 - mCoefficients[channel].mAngularFrequencyRatio) /
                                            (1.0 + mCoefficients[channel].mAngularFrequencyRatio);
    }
    if (mInvert <= 0.5) {
      mFeedbackSign = -1.0;
      mInvert = 0.f;
    } else {
      mFeedbackSign = 1.0;
      mInvert = 1.f;
    }
    mInputs[0].mRawInput = *in1++;
    mInputs[1].mRawInput = *in2++;
    mInputs[2].mRawInput = *in3++;
    mInputs[0].mFeedbackInput =
        mFeedbackSign * (0.999999 * mFeedback * (1.2e-7 + mHistory[0].mOutputs[3])) +
        mInputs[0].mRawInput;
    mInputs[1].mFeedbackInput =
        mFeedbackSign * (0.999999 * mFeedback * (1.2e-7 + mHistory[1].mOutputs[3])) +
        mInputs[1].mRawInput;
    mInputs[2].mFeedbackInput =
        mFeedbackSign * (0.999999 * mFeedback * (1.2e-7 + mHistory[2].mOutputs[3])) +
        mInputs[2].mRawInput;
    mHistory[0].mOutputs[0] = mCoefficients[0].mCoefficient * (mInputs[0].mFeedbackInput + mHistory[0].mOutputs[0]) -
                              mHistory[0].mInputs[0];
    mHistory[0].mInputs[0] = mInputs[0].mFeedbackInput;
    mHistory[0].mOutputs[1] = mCoefficients[0].mCoefficient * (mHistory[0].mOutputs[0] + mHistory[0].mOutputs[1]) -
                              mHistory[0].mInputs[1];
    mHistory[0].mInputs[1] = mHistory[0].mOutputs[0];
    mHistory[0].mOutputs[2] = mCoefficients[0].mCoefficient * (mHistory[0].mOutputs[1] + mHistory[0].mOutputs[2]) -
                              mHistory[0].mInputs[2];
    mHistory[0].mInputs[2] = mHistory[0].mOutputs[1];
    mHistory[0].mOutputs[3] = mCoefficients[0].mCoefficient * (mHistory[0].mOutputs[2] + mHistory[0].mOutputs[3]) -
                              mHistory[0].mInputs[3];
    mHistory[0].mInputs[3] = mHistory[0].mOutputs[2];
    mHistory[1].mOutputs[0] = mCoefficients[1].mCoefficient * (mInputs[1].mFeedbackInput + mHistory[1].mOutputs[0]) -
                              mHistory[1].mInputs[0];
    mHistory[1].mInputs[0] = mInputs[1].mFeedbackInput;
    mHistory[1].mOutputs[1] = mCoefficients[1].mCoefficient * (mHistory[1].mOutputs[0] + mHistory[1].mOutputs[1]) -
                              mHistory[1].mInputs[1];
    mHistory[1].mInputs[1] = mHistory[1].mOutputs[0];
    mHistory[1].mOutputs[2] = mCoefficients[1].mCoefficient * (mHistory[1].mOutputs[1] + mHistory[1].mOutputs[2]) -
                              mHistory[1].mInputs[2];
    mHistory[1].mInputs[2] = mHistory[1].mOutputs[1];
    mHistory[1].mOutputs[3] = mCoefficients[1].mCoefficient * (mHistory[1].mOutputs[2] + mHistory[1].mOutputs[3]) -
                              mHistory[1].mInputs[3];
    mHistory[1].mInputs[3] = mHistory[1].mOutputs[2];
    mHistory[2].mOutputs[0] = mCoefficients[2].mCoefficient * (mInputs[2].mFeedbackInput + mHistory[2].mOutputs[0]) -
                              mHistory[2].mInputs[0];
    mHistory[1].mInputs[0] = mInputs[1].mFeedbackInput;
    mHistory[2].mOutputs[1] = mCoefficients[2].mCoefficient * (mHistory[2].mOutputs[0] + mHistory[2].mOutputs[1]) -
                              mHistory[2].mInputs[1];
    mHistory[1].mInputs[1] = mHistory[1].mOutputs[0];
    mHistory[2].mOutputs[2] = mCoefficients[2].mCoefficient * (mHistory[2].mOutputs[1] + mHistory[2].mOutputs[2]) -
                              mHistory[2].mInputs[2];
    mHistory[2].mInputs[2] = mHistory[2].mOutputs[1];
    mHistory[2].mOutputs[3] = mCoefficients[2].mCoefficient * (mHistory[2].mOutputs[2] + mHistory[2].mOutputs[3]) -
                              mHistory[2].mInputs[3];
    mHistory[2].mInputs[3] = mHistory[2].mOutputs[2];
    mLFORate = 0.02 + 3.98 * mFrequency;
    mLFOPhase += mLFORate * mLFOPhaseStep;
    while (mLFOPhase >= 2048.0) {
      mLFOPhase -= 2048.0;
    }
    while (mLFOPhase < 0.0) {
      mLFOPhase += 2048.0;
    }
    mLFOIndex = static_cast< int >(mLFOPhase);
    mLFOFraction = mLFOPhase - mLFOIndex;
    mLeftLFO =
        mSineTable[mLFOIndex] + mLFOFraction * (mSineTable[mLFOIndex + 1] - mSineTable[mLFOIndex]);
    mRightLFO = -1.0 * mLeftLFO;
    mSurroundLFO = mLeftLFO;
    *out1++ += static_cast< float >(mInputs[0].mRawInput * mDry + mHistory[0].mOutputs[3] * mWet);
    *out2++ += static_cast< float >(mInputs[1].mRawInput * mDry + mHistory[1].mOutputs[3] * mWet);
    *out3++ += static_cast< float >(mInputs[2].mRawInput * mDry + mHistory[2].mOutputs[3] * mWet);
    mLeftLFO = (mLeftLFO + 1.0) / 2.0;
    mRightLFO = (mRightLFO + 1.0) / 2.0;
    mChannelFrequency[0] = mBaseFrequency + mLeftLFO * (100.f + 7800.f * mSweepRange);
    mChannelFrequency[1] = mBaseFrequency + mRightLFO * (100.f + 7800.f * mSweepRange);
    mChannelFrequency[2] = mBaseFrequency + mSurroundLFO * (100.f + 7800.f * mSweepRange);
  }
}

void CPhaser::processReplacing(float** inputs, float** outputs, long sampleFrames) {
  float* in1 = inputs[0];
  float* in2 = inputs[1];
  float* in3 = inputs[2];
  float* out1 = outputs[0];
  float* out2 = outputs[1];
  float* out3 = outputs[2];
  while (--sampleFrames >= 0) {
    for (int channel = 0; channel < 3; ++channel) {
      mCoefficients[channel].mAngularFrequencyRatio =
          2.0 * M_PI * mChannelFrequency[channel] / mSampleRate;
      mCoefficients[channel].mCoefficient = (1.0 - mCoefficients[channel].mAngularFrequencyRatio) /
                                            (1.0 + mCoefficients[channel].mAngularFrequencyRatio);
    }
    if (mInvert <= 0.5) {
      mFeedbackSign = -1.0;
      mInvert = 0.f;
    } else {
      mFeedbackSign = 1.0;
      mInvert = 1.f;
    }
    mInputs[0].mRawInput = *in1++;
    mInputs[1].mRawInput = *in2++;
    mInputs[2].mRawInput = *in3++;
    mInputs[0].mFeedbackInput =
        mFeedbackSign * (0.999999 * mFeedback * (1.2e-7 + mHistory[0].mOutputs[3])) +
        mInputs[0].mRawInput;
    mInputs[1].mFeedbackInput =
        mFeedbackSign * (0.999999 * mFeedback * (1.2e-7 + mHistory[1].mOutputs[3])) +
        mInputs[1].mRawInput;
    mInputs[2].mFeedbackInput =
        mFeedbackSign * (0.999999 * mFeedback * (1.2e-7 + mHistory[2].mOutputs[3])) +
        mInputs[2].mRawInput;
    mHistory[0].mOutputs[0] = mCoefficients[0].mCoefficient * (mInputs[0].mFeedbackInput + mHistory[0].mOutputs[0]) -
                              mHistory[0].mInputs[0];
    mHistory[0].mInputs[0] = mInputs[0].mFeedbackInput;
    mHistory[0].mOutputs[1] = mCoefficients[0].mCoefficient * (mHistory[0].mOutputs[0] + mHistory[0].mOutputs[1]) -
                              mHistory[0].mInputs[1];
    mHistory[0].mInputs[1] = mHistory[0].mOutputs[0];
    mHistory[0].mOutputs[2] = mCoefficients[0].mCoefficient * (mHistory[0].mOutputs[1] + mHistory[0].mOutputs[2]) -
                              mHistory[0].mInputs[2];
    mHistory[0].mInputs[2] = mHistory[0].mOutputs[1];
    mHistory[0].mOutputs[3] = mCoefficients[0].mCoefficient * (mHistory[0].mOutputs[2] + mHistory[0].mOutputs[3]) -
                              mHistory[0].mInputs[3];
    mHistory[0].mInputs[3] = mHistory[0].mOutputs[2];
    mHistory[1].mOutputs[0] = mCoefficients[1].mCoefficient * (mInputs[1].mFeedbackInput + mHistory[1].mOutputs[0]) -
                              mHistory[1].mInputs[0];
    mHistory[1].mInputs[0] = mInputs[1].mFeedbackInput;
    mHistory[1].mOutputs[1] = mCoefficients[1].mCoefficient * (mHistory[1].mOutputs[0] + mHistory[1].mOutputs[1]) -
                              mHistory[1].mInputs[1];
    mHistory[1].mInputs[1] = mHistory[1].mOutputs[0];
    mHistory[1].mOutputs[2] = mCoefficients[1].mCoefficient * (mHistory[1].mOutputs[1] + mHistory[1].mOutputs[2]) -
                              mHistory[1].mInputs[2];
    mHistory[1].mInputs[2] = mHistory[1].mOutputs[1];
    mHistory[1].mOutputs[3] = mCoefficients[1].mCoefficient * (mHistory[1].mOutputs[2] + mHistory[1].mOutputs[3]) -
                              mHistory[1].mInputs[3];
    mHistory[1].mInputs[3] = mHistory[1].mOutputs[2];
    mHistory[2].mOutputs[0] = mCoefficients[2].mCoefficient * (mInputs[2].mFeedbackInput + mHistory[2].mOutputs[0]) -
                              mHistory[2].mInputs[0];
    mHistory[1].mInputs[0] = mInputs[1].mFeedbackInput;
    mHistory[2].mOutputs[1] = mCoefficients[2].mCoefficient * (mHistory[2].mOutputs[0] + mHistory[2].mOutputs[1]) -
                              mHistory[2].mInputs[1];
    mHistory[1].mInputs[1] = mHistory[1].mOutputs[0];
    mHistory[2].mOutputs[2] = mCoefficients[2].mCoefficient * (mHistory[2].mOutputs[1] + mHistory[2].mOutputs[2]) -
                              mHistory[2].mInputs[2];
    mHistory[2].mInputs[2] = mHistory[2].mOutputs[1];
    mHistory[2].mOutputs[3] = mCoefficients[2].mCoefficient * (mHistory[2].mOutputs[2] + mHistory[2].mOutputs[3]) -
                              mHistory[2].mInputs[3];
    mHistory[2].mInputs[3] = mHistory[2].mOutputs[2];
    mLFORate = 0.02 + 5.98 * mFrequency;
    mLFOPhase += mLFORate * mLFOPhaseStep;
    while (mLFOPhase >= 2048.0) {
      mLFOPhase -= 2048.0;
    }
    while (mLFOPhase < 0.0) {
      mLFOPhase += 2048.0;
    }
    mLFOIndex = static_cast< int >(mLFOPhase);
    mLFOFraction = mLFOPhase - mLFOIndex;
    mLeftLFO =
        mSineTable[mLFOIndex] + mLFOFraction * (mSineTable[mLFOIndex + 1] - mSineTable[mLFOIndex]);
    mRightLFO = -1.0 * mLeftLFO;
    mSurroundLFO = mLeftLFO;
    *out1++ = static_cast< float >(mInputs[0].mRawInput * mDry + mHistory[0].mOutputs[3] * mWet);
    *out2++ = static_cast< float >(mInputs[1].mRawInput * mDry + mHistory[1].mOutputs[3] * mWet);
    *out3++ = static_cast< float >(mInputs[2].mRawInput * mDry + mHistory[2].mOutputs[3] * mWet);
    mLeftLFO = (mLeftLFO + 1.0) / 2.0;
    mRightLFO = (mRightLFO + 1.0) / 2.0;
    mChannelFrequency[0] = mBaseFrequency + mLeftLFO * (100.f + 7800.f * mSweepRange);
    mChannelFrequency[1] = mBaseFrequency + mRightLFO * (100.f + 7800.f * mSweepRange);
    mChannelFrequency[2] = mBaseFrequency + mSurroundLFO * (100.f + 7800.f * mSweepRange);
  }
}
