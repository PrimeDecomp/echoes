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
  hasVu(true);
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
  float* input0 = inputs[0];
  float* input1 = inputs[1];
  float* input2 = inputs[2];
  float* output0 = outputs[0];
  float* output1 = outputs[1];
  float* output2 = outputs[2];
  SFilterHistory& left = mHistory[0];
  SFilterHistory& right = mHistory[1];
  SFilterHistory& surround = mHistory[2];
  while (--sampleFrames >= 0) {
    mCoefficients[0].mAngularFrequencyRatio = 2.0 * M_PI * mChannelFrequency[0] / mSampleRate;
    mCoefficients[0].mCoefficient = (1.0 - mCoefficients[0].mAngularFrequencyRatio) /
                                    (1.0 + mCoefficients[0].mAngularFrequencyRatio);
    mCoefficients[1].mAngularFrequencyRatio = 2.0 * M_PI * mChannelFrequency[1] / mSampleRate;
    mCoefficients[1].mCoefficient = (1.0 - mCoefficients[1].mAngularFrequencyRatio) /
                                    (1.0 + mCoefficients[1].mAngularFrequencyRatio);
    mCoefficients[2].mAngularFrequencyRatio = 2.0 * M_PI * mChannelFrequency[2] / mSampleRate;
    mCoefficients[2].mCoefficient = (1.0 - mCoefficients[2].mAngularFrequencyRatio) /
                                    (1.0 + mCoefficients[2].mAngularFrequencyRatio);
    if (mInvert <= 0.5) {
      mFeedbackSign = -1.0;
      mInvert = 0.f;
    } else {
      mFeedbackSign = 1.0;
      mInvert = 1.f;
    }
    mInputs[0].mRawInput = *input0++;
    mInputs[1].mRawInput = *input1++;
    mInputs[2].mRawInput = *input2++;
    mInputs[0].mFeedbackInput =
        mFeedbackSign * (0.999999 * mFeedback * (1.2e-7 + left.mOutputs[3])) + mInputs[0].mRawInput;
    mInputs[1].mFeedbackInput =
        mFeedbackSign * (0.999999 * mFeedback * (1.2e-7 + right.mOutputs[3])) +
        mInputs[1].mRawInput;
    mInputs[2].mFeedbackInput =
        mFeedbackSign * (0.999999 * mFeedback * (1.2e-7 + surround.mOutputs[3])) +
        mInputs[2].mRawInput;
    left.mOutputs[0] =
        mCoefficients[0].mCoefficient * (mInputs[0].mFeedbackInput + left.mOutputs[0]) -
        left.mInputs[0];
    left.mInputs[0] = mInputs[0].mFeedbackInput;
    left.mOutputs[1] =
        mCoefficients[0].mCoefficient * (left.mOutputs[0] + left.mOutputs[1]) - left.mInputs[1];
    left.mInputs[1] = left.mOutputs[0];
    left.mOutputs[2] =
        mCoefficients[0].mCoefficient * (left.mOutputs[1] + left.mOutputs[2]) - left.mInputs[2];
    left.mInputs[2] = left.mOutputs[1];
    left.mOutputs[3] =
        mCoefficients[0].mCoefficient * (left.mOutputs[2] + left.mOutputs[3]) - left.mInputs[3];
    left.mInputs[3] = left.mOutputs[2];
    right.mOutputs[0] =
        mCoefficients[1].mCoefficient * (mInputs[1].mFeedbackInput + right.mOutputs[0]) -
        right.mInputs[0];
    right.mInputs[0] = mInputs[1].mFeedbackInput;
    right.mOutputs[1] =
        mCoefficients[1].mCoefficient * (right.mOutputs[0] + right.mOutputs[1]) - right.mInputs[1];
    right.mInputs[1] = right.mOutputs[0];
    right.mOutputs[2] =
        mCoefficients[1].mCoefficient * (right.mOutputs[1] + right.mOutputs[2]) - right.mInputs[2];
    right.mInputs[2] = right.mOutputs[1];
    right.mOutputs[3] =
        mCoefficients[1].mCoefficient * (right.mOutputs[2] + right.mOutputs[3]) - right.mInputs[3];
    right.mInputs[3] = right.mOutputs[2];
    surround.mOutputs[0] =
        mCoefficients[2].mCoefficient * (mInputs[2].mFeedbackInput + surround.mOutputs[0]) -
        surround.mInputs[0];
    right.mInputs[0] = mInputs[2].mFeedbackInput;
    surround.mOutputs[1] =
        mCoefficients[2].mCoefficient * (surround.mOutputs[0] + surround.mOutputs[1]) -
        surround.mInputs[1];
    right.mInputs[1] = surround.mOutputs[0];
    surround.mOutputs[2] =
        mCoefficients[2].mCoefficient * (surround.mOutputs[1] + surround.mOutputs[2]) -
        surround.mInputs[2];
    surround.mInputs[2] = surround.mOutputs[1];
    surround.mOutputs[3] =
        mCoefficients[2].mCoefficient * (surround.mOutputs[2] + surround.mOutputs[3]) -
        surround.mInputs[3];
    surround.mInputs[3] = surround.mOutputs[2];
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
    mRightLFO = -mLeftLFO;
    mSurroundLFO = mLeftLFO;
    *output0++ += static_cast< float >(mInputs[0].mRawInput * mDry + left.mOutputs[3] * mWet);
    *output1++ += static_cast< float >(mInputs[1].mRawInput * mDry + right.mOutputs[3] * mWet);
    *output2++ += static_cast< float >(mInputs[2].mRawInput * mDry + surround.mOutputs[3] * mWet);
    mLeftLFO = (1.0 + mLeftLFO) * 0.5;
    mRightLFO = (1.0 + mRightLFO) * 0.5;
    const float sweep = 100.f + 7800.f * mSweepRange;
    mChannelFrequency[0] = mBaseFrequency + mLeftLFO * sweep;
    mChannelFrequency[1] = mBaseFrequency + mRightLFO * sweep;
    mChannelFrequency[2] = mBaseFrequency + mSurroundLFO * sweep;
  }
}

void CPhaser::processReplacing(float** inputs, float** outputs, long sampleFrames) {
  float* input0 = inputs[0];
  float* input1 = inputs[1];
  float* input2 = inputs[2];
  float* output0 = outputs[0];
  float* output1 = outputs[1];
  float* output2 = outputs[2];
  SFilterHistory& left = mHistory[0];
  SFilterHistory& right = mHistory[1];
  SFilterHistory& surround = mHistory[2];
  while (--sampleFrames >= 0) {
    mCoefficients[0].mAngularFrequencyRatio = 2.0 * M_PI * mChannelFrequency[0] / mSampleRate;
    mCoefficients[0].mCoefficient = (1.0 - mCoefficients[0].mAngularFrequencyRatio) /
                                    (1.0 + mCoefficients[0].mAngularFrequencyRatio);
    mCoefficients[1].mAngularFrequencyRatio = 2.0 * M_PI * mChannelFrequency[1] / mSampleRate;
    mCoefficients[1].mCoefficient = (1.0 - mCoefficients[1].mAngularFrequencyRatio) /
                                    (1.0 + mCoefficients[1].mAngularFrequencyRatio);
    mCoefficients[2].mAngularFrequencyRatio = 2.0 * M_PI * mChannelFrequency[2] / mSampleRate;
    mCoefficients[2].mCoefficient = (1.0 - mCoefficients[2].mAngularFrequencyRatio) /
                                    (1.0 + mCoefficients[2].mAngularFrequencyRatio);
    if (mInvert <= 0.5) {
      mFeedbackSign = -1.0;
      mInvert = 0.f;
    } else {
      mFeedbackSign = 1.0;
      mInvert = 1.f;
    }
    mInputs[0].mRawInput = *input0++;
    mInputs[1].mRawInput = *input1++;
    mInputs[2].mRawInput = *input2++;
    mInputs[0].mFeedbackInput =
        mFeedbackSign * (0.999999 * mFeedback * (1.2e-7 + left.mOutputs[3])) + mInputs[0].mRawInput;
    mInputs[1].mFeedbackInput =
        mFeedbackSign * (0.999999 * mFeedback * (1.2e-7 + right.mOutputs[3])) +
        mInputs[1].mRawInput;
    mInputs[2].mFeedbackInput =
        mFeedbackSign * (0.999999 * mFeedback * (1.2e-7 + surround.mOutputs[3])) +
        mInputs[2].mRawInput;
    left.mOutputs[0] =
        mCoefficients[0].mCoefficient * (mInputs[0].mFeedbackInput + left.mOutputs[0]) -
        left.mInputs[0];
    left.mInputs[0] = mInputs[0].mFeedbackInput;
    left.mOutputs[1] =
        mCoefficients[0].mCoefficient * (left.mOutputs[0] + left.mOutputs[1]) - left.mInputs[1];
    left.mInputs[1] = left.mOutputs[0];
    left.mOutputs[2] =
        mCoefficients[0].mCoefficient * (left.mOutputs[1] + left.mOutputs[2]) - left.mInputs[2];
    left.mInputs[2] = left.mOutputs[1];
    left.mOutputs[3] =
        mCoefficients[0].mCoefficient * (left.mOutputs[2] + left.mOutputs[3]) - left.mInputs[3];
    left.mInputs[3] = left.mOutputs[2];
    right.mOutputs[0] =
        mCoefficients[1].mCoefficient * (mInputs[1].mFeedbackInput + right.mOutputs[0]) -
        right.mInputs[0];
    right.mInputs[0] = mInputs[1].mFeedbackInput;
    right.mOutputs[1] =
        mCoefficients[1].mCoefficient * (right.mOutputs[0] + right.mOutputs[1]) - right.mInputs[1];
    right.mInputs[1] = right.mOutputs[0];
    right.mOutputs[2] =
        mCoefficients[1].mCoefficient * (right.mOutputs[1] + right.mOutputs[2]) - right.mInputs[2];
    right.mInputs[2] = right.mOutputs[1];
    right.mOutputs[3] =
        mCoefficients[1].mCoefficient * (right.mOutputs[2] + right.mOutputs[3]) - right.mInputs[3];
    right.mInputs[3] = right.mOutputs[2];
    surround.mOutputs[0] =
        mCoefficients[2].mCoefficient * (mInputs[2].mFeedbackInput + surround.mOutputs[0]) -
        surround.mInputs[0];
    right.mInputs[0] = mInputs[2].mFeedbackInput;
    surround.mOutputs[1] =
        mCoefficients[2].mCoefficient * (surround.mOutputs[0] + surround.mOutputs[1]) -
        surround.mInputs[1];
    right.mInputs[1] = surround.mOutputs[0];
    surround.mOutputs[2] =
        mCoefficients[2].mCoefficient * (surround.mOutputs[1] + surround.mOutputs[2]) -
        surround.mInputs[2];
    surround.mInputs[2] = surround.mOutputs[1];
    surround.mOutputs[3] =
        mCoefficients[2].mCoefficient * (surround.mOutputs[2] + surround.mOutputs[3]) -
        surround.mInputs[3];
    surround.mInputs[3] = surround.mOutputs[2];
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
    mRightLFO = -mLeftLFO;
    mSurroundLFO = mLeftLFO;
    *output0++ = static_cast< float >(mInputs[0].mRawInput * mDry + left.mOutputs[3] * mWet);
    *output1++ = static_cast< float >(mInputs[1].mRawInput * mDry + right.mOutputs[3] * mWet);
    *output2++ = static_cast< float >(mInputs[2].mRawInput * mDry + surround.mOutputs[3] * mWet);
    mLeftLFO = (1.0 + mLeftLFO) * 0.5;
    mRightLFO = (1.0 + mRightLFO) * 0.5;
    const float sweep = 100.f + 7800.f * mSweepRange;
    mChannelFrequency[0] = mBaseFrequency + mLeftLFO * sweep;
    mChannelFrequency[1] = mBaseFrequency + mRightLFO * sweep;
    mChannelFrequency[2] = mBaseFrequency + mSurroundLFO * sweep;
  }
}
