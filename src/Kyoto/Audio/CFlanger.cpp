#include "Kyoto/Audio/CFlanger.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Audio/AEffEditor.hpp"
#include "Kyoto/Basics/CBasics.hpp"

#include <math.h>
#include <stdio.h>
#include <string.h>

CFlanger::SProgram::SProgram()
: mDelay(1.f)
, mFeedback(0.9f)
, mOutput(0.9f)
, mLFOFrequencyComplement(0.001f)
, mLFODepth(0.8f)
, mLFOWave(1.f)
, mDry(0.f)
, mLFOPhase(1.f) {
  strcpy(mName, "Init");
}

CFlanger::CFlanger(AudioMasterCallback audioMaster)
: AudioEffectX(audioMaster, 16, 8)
, mPrograms(16, SProgram())
, mLeftDelay(rs_new float[320])
, mRightDelay(rs_new float[320])
, mSurroundDelay(rs_new float[320])
, mDelay(0.f)
, mFeedback(0.f)
, mOutput(0.f)
, mLFOFrequencyComplement(0.f)
, mLFODepth(0.f)
, mLFOWave(0.f)
, mDry(0.f)
, mLFOPhase(0.f)
, mLeftLFOCounter(0)
, mRightLFOCounter(0)
, mSurroundLFOCounter(0)
, mPeak(0.f)
, mDelaySamples(0)
, mLFOPhasePercent(0)
, mDelayBufferSamples(320)
, mLeftWritePosition(0)
, mRightWritePosition(0)
, mSurroundWritePosition(0)
, mLeftReadPosition(0)
, mRightReadPosition(0)
, mSurroundReadPosition(0) {
  setProgram(0);
  setNumInputs(3);
  setNumOutputs(3);
  hasVu(true);
  canProcessReplacing(true);
  setUniqueID('FGRC');
  suspend();
}

CFlanger::~CFlanger() {}

void CFlanger::setProgram(long program) {
  mCurrentProgram = program;
  SProgram& selected = mPrograms[program];
  setParameter(0, selected.mDelay);
  setParameter(1, selected.mFeedback);
  setParameter(2, selected.mOutput);
  setParameter(3, selected.mLFOFrequencyComplement);
  setParameter(4, selected.mLFODepth);
  setParameter(5, selected.mLFOWave);
  setParameter(6, selected.mDry);
  setParameter(7, selected.mLFOPhase);
}

void CFlanger::SetChannelDelay(long channel, float delay) {
  int delaySamples = static_cast< int >(delay * static_cast< float >(mDelayBufferSamples - 1));
  if (channel == 1) {
    int position = mLeftWritePosition - delaySamples;
    if (position < 0)
      position += mDelayBufferSamples;
    mLeftReadPosition = position;
  } else if (channel == 2) {
    int position = mRightWritePosition - delaySamples;
    if (position < 0)
      position += mDelayBufferSamples;
    mRightReadPosition = position;
  } else if (channel == 3) {
    int position = mSurroundWritePosition - delaySamples;
    if (position < 0)
      position += mDelayBufferSamples;
    mSurroundReadPosition = position;
  }
}

void CFlanger::SetDelay(float delay) {
  mDelay = delay;
  mDelaySamples = static_cast< int >(delay * static_cast< float >(mDelayBufferSamples - 1));
  mPrograms[mCurrentProgram].mDelay = delay;
  SetChannelDelay(1, delay);
  SetChannelDelay(2, delay);
  SetChannelDelay(3, delay);
}

void CFlanger::SetLFOPhase(float phase) {
  mLFOPhase = phase;
  mLFOPhasePercent = static_cast< int >(100.f * phase);
  mPrograms[mCurrentProgram].mLFOPhase = phase;
  mLeftLFOCounter = mRightLFOCounter;
}

void CFlanger::SetLFOFrequency(float frequency) {
  mLFOFrequencyComplement = 1.f - frequency;
  mLFOPeriod = static_cast< float >(640000.0 * mLFOFrequencyComplement);
  mPrograms[mCurrentProgram].mLFOFrequencyComplement = mLFOFrequencyComplement;
}

float CFlanger::GetLFOFrequency() { return 1.f - mLFOFrequencyComplement; }

void CFlanger::setProgramName(char* name) { strcpy(mPrograms[mCurrentProgram].mName, name); }

void CFlanger::getProgramName(char* name) {
  if (strcmp(mPrograms[mCurrentProgram].mName, "Init") == 0)
    sprintf(name, "%s %d", mPrograms[mCurrentProgram].mName, mCurrentProgram + 1);
  else
    strcpy(name, mPrograms[mCurrentProgram].mName);
}

void CFlanger::suspend() {
  CBasics::ZeroMemory(mLeftDelay.get(), mDelayBufferSamples * sizeof(float));
  CBasics::ZeroMemory(mRightDelay.get(), mDelayBufferSamples * sizeof(float));
  CBasics::ZeroMemory(mSurroundDelay.get(), mDelayBufferSamples * sizeof(float));
}

float CFlanger::getVu() {
  float peak = mPeak;
  mPeak = 0.f;
  return peak;
}

void CFlanger::setParameter(long index, float value) {
  SProgram& program = mPrograms[mCurrentProgram];
  switch (index) {
  case 0:
    SetDelay(value);
    break;
  case 1:
    program.mFeedback = value;
    mFeedback = value;
    break;
  case 2:
    program.mOutput = value;
    mOutput = value;
    break;
  case 3:
    SetLFOFrequency(value);
    break;
  case 4:
    program.mLFODepth = value;
    mLFODepth = value;
    break;
  case 5:
    program.mLFOWave = value;
    mLFOWave = value;
    break;
  case 6:
    program.mDry = value;
    mDry = value;
    break;
  case 7:
    SetLFOPhase(value);
    break;
  }
  if (mEditor != nullptr)
    mEditor->postUpdate();
}

float CFlanger::getParameter(long index) {
  float value = 0.f;
  switch (index) {
  case 0:
    value = mDelay;
    break;
  case 1:
    value = mFeedback;
    break;
  case 2:
    value = mOutput;
    break;
  case 3:
    value = GetLFOFrequency();
    break;
  case 4:
    value = mLFODepth;
    break;
  case 5:
    value = mLFOWave;
    break;
  case 6:
    value = mDry;
    break;
  case 7:
    value = mLFOPhase;
    break;
  }
  return value;
}

void CFlanger::getParameterName(long index, char* name) {
  switch (index) {
  case 0:
    strcpy(name, " Delay  ");
    break;
  case 1:
    strcpy(name, "FeedBack");
    break;
  case 2:
    strcpy(name, " Volume ");
    break;
  case 3:
    strcpy(name, "LFO Freq");
    break;
  case 4:
    strcpy(name, "LFO Dept");
    break;
  case 5:
    strcpy(name, "LFO Wave");
    break;
  case 6:
    strcpy(name, "Wet<>Dry");
    break;
  case 7:
    strcpy(name, "LFOPhase");
    break;
  }
}

void CFlanger::getParameterDisplay(long index, char* text) {
  switch (index) {
  case 0:
    ms2string(static_cast< float >(mDelaySamples), text);
    break;
  case 1:
    float2string(mFeedback, text);
    break;
  case 2:
    dB2string(mOutput, text);
    break;
  case 3:
    Hz2string(mLFOPeriod, text);
    break;
  case 4:
    float2string(mLFODepth, text);
    break;
  case 5:
    float2string(mLFOWave, text);
    break;
  case 6:
    float2string(mDry, text);
    break;
  case 7:
    int2string(mLFOPhasePercent, text);
    break;
  }
}

void CFlanger::getParameterLabel(long index, char* label) {
  switch (index) {
  case 0:
    strcpy(label, "mseconds");
    break;
  case 1:
    strcpy(label, " amount ");
    break;
  case 2:
    strcpy(label, "   dB   ");
    break;
  case 3:
    strcpy(label, "   Hz   ");
    break;
  case 4:
    strcpy(label, " amount ");
    break;
  case 5:
    strcpy(label, " shape  ");
    break;
  case 6:
    strcpy(label, " amount ");
    break;
  case 7:
    strcpy(label, " amount ");
    break;
  }
}

void CFlanger::UpdateChannelLFO(long sampleFrames, long channel) {
  int wave = static_cast< int >(mLFOWave);
  int period = static_cast< int >(mLFOPeriod);
  int counter = 0;
  float modulation = 0.f;
  if (channel == 1)
    period += static_cast< int >(static_cast< double >(mLFOPeriod) * mLFOPhase);
  float availableDelay = 1.f - mDelay;
  int halfPeriod = period / 2;
  if (channel == 1) {
    mLeftLFOCounter += sampleFrames;
    if (mLeftLFOCounter > period)
      mLeftLFOCounter = 0;
    counter = mLeftLFOCounter;
  } else if (channel == 2) {
    mRightLFOCounter += sampleFrames;
    if (mRightLFOCounter > period)
      mRightLFOCounter = 0;
    counter = mRightLFOCounter;
  } else if (channel == 3) {
    mSurroundLFOCounter += sampleFrames;
    if (mSurroundLFOCounter > period)
      mSurroundLFOCounter = 0;
    counter = mSurroundLFOCounter;
  }

  if (wave == 0) {
    double sine =
        sin(3.14159265358979323846 * (2.0 * (static_cast< double >(counter) / (period + 1))));
    sine *= 0.5;
    sine += 0.5;
    modulation = static_cast< float >(sine);
  } else if (wave == 1) {
    if (counter < halfPeriod)
      modulation = static_cast< float >(counter) / static_cast< float >(halfPeriod + 1);
    else
      modulation = static_cast< float >(period - counter) / static_cast< float >(halfPeriod + 1);
  }

  float delay = availableDelay * modulation;
  delay *= mLFODepth;
  delay += mDelay;
  if (delay > 1.0)
    delay = 1.f;
  if (delay < 0.0)
    delay = 0.f;
  SetChannelDelay(channel, delay);
}

void CFlanger::ProcessSamples(float* input, float* output, float* delayWrite, float* delayRead,
                              long sampleFrames) {
  float dry = mDry;
  float wet = 1.f - dry;
  float feedback = mFeedback;
  float volume = mOutput;
  float peak = mPeak;
  while (--sampleFrames >= 0) {
    float inputSample = *input++;
    float delayedSample = *delayRead++;
    float mixedSample = inputSample;
    float oldOutput = *output;
    inputSample += delayedSample * feedback;
    mixedSample *= dry;
    mixedSample += delayedSample * wet;
    mixedSample *= volume;
    if (mixedSample > peak)
      peak = mixedSample;
    *delayWrite++ = inputSample;
    *output++ = oldOutput + mixedSample;
  }
  mPeak = peak;
}

void CFlanger::process(float** inputs, float** outputs, long sampleFrames) {
  long count;
  long writeSpace;
  long readSpace;
  float* inLeft;
  float* inRight;
  float* inSurround;
  float* outLeft;
  float* outRight;
  float* outSurround;
  long remainingLeft, remainingRight, remainingSurround;
  remainingLeft = remainingRight = remainingSurround = sampleFrames;
  inLeft = inputs[0];
  inRight = inputs[1];
  inSurround = inputs[2];
  outLeft = outputs[0];
  outRight = outputs[1];
  outSurround = outputs[2];
  while (remainingLeft > 0 || remainingRight > 0 || remainingSurround > 0) {
    if (remainingLeft > 0) {
      writeSpace = mDelayBufferSamples - mLeftWritePosition;
      if (writeSpace < remainingLeft || mDelayBufferSamples - mLeftReadPosition < remainingLeft) {
        readSpace = mDelayBufferSamples - mLeftReadPosition;
        if (writeSpace < readSpace) {
          count = writeSpace;
        } else {
          count = readSpace;
        }
      } else {
        count = remainingLeft;
      }
      ProcessSamples(inLeft, outLeft, mLeftDelay.get() + mLeftWritePosition, mLeftDelay.get() + mLeftReadPosition, count);
      inLeft += count;
      outLeft += count;
      mLeftWritePosition += count;
      if (mLeftWritePosition >= mDelayBufferSamples) {
        mLeftWritePosition -= mDelayBufferSamples;
      }
      mLeftReadPosition += count;
      if (mLeftReadPosition >= mDelayBufferSamples) {
        mLeftReadPosition -= mDelayBufferSamples;
      }
      remainingLeft -= count;
      UpdateChannelLFO(count, 1);
    }
    if (remainingRight > 0) {
      writeSpace = mDelayBufferSamples - mRightWritePosition;
      if (writeSpace < remainingRight || mDelayBufferSamples - mRightReadPosition < remainingRight) {
        readSpace = mDelayBufferSamples - mRightReadPosition;
        if (writeSpace < readSpace) {
          count = writeSpace;
        } else {
          count = readSpace;
        }
      } else {
        count = remainingRight;
      }
      ProcessSamples(inRight, outRight, mRightDelay.get() + mRightWritePosition, mRightDelay.get() + mRightReadPosition, count);
      inRight += count;
      outRight += count;
      mRightWritePosition += count;
      if (mRightWritePosition >= mDelayBufferSamples) {
        mRightWritePosition -= mDelayBufferSamples;
      }
      mRightReadPosition += count;
      if (mRightReadPosition >= mDelayBufferSamples) {
        mRightReadPosition -= mDelayBufferSamples;
      }
      remainingRight -= count;
      UpdateChannelLFO(count, 2);
    }
    if (remainingSurround > 0) {
      writeSpace = mDelayBufferSamples - mSurroundWritePosition;
      if (writeSpace < remainingSurround || mDelayBufferSamples - mSurroundReadPosition < remainingSurround) {
        readSpace = mDelayBufferSamples - mSurroundReadPosition;
        if (writeSpace < readSpace) {
          count = writeSpace;
        } else {
          count = readSpace;
        }
      } else {
        count = remainingSurround;
      }
      ProcessSamples(inSurround, outSurround, mSurroundDelay.get() + mSurroundWritePosition, mSurroundDelay.get() + mSurroundReadPosition, count);
      inSurround += count;
      outSurround += count;
      mSurroundWritePosition += count;
      if (mSurroundWritePosition >= mDelayBufferSamples) {
        mSurroundWritePosition -= mDelayBufferSamples;
      }
      mSurroundReadPosition += count;
      if (mSurroundReadPosition >= mDelayBufferSamples) {
        mSurroundReadPosition -= mDelayBufferSamples;
      }
      remainingSurround -= count;
      UpdateChannelLFO(count, 3);
    }
  }
}

void CFlanger::ProcessReplacingSamples(float* input, float* output, float* delayWrite,
                                       float* delayRead, long sampleFrames) {
  float dry = mDry;
  float wet = 1.f - dry;
  float feedback = mFeedback;
  float volume = mOutput;
  float peak = mPeak;
  while (--sampleFrames >= 0) {
    float inputSample = *input++;
    float delayedSample = *delayRead++;
    float mixedSample = inputSample;
    inputSample += delayedSample * feedback;
    mixedSample *= dry;
    mixedSample += delayedSample * wet;
    mixedSample *= volume;
    if (mixedSample > peak)
      peak = mixedSample;
    *delayWrite++ = inputSample;
    *output++ = mixedSample;
  }
  mPeak = peak;
}

void CFlanger::processReplacing(float** inputs, float** outputs, long sampleFrames) {
  long count;
  long writeSpace;
  long readSpace;
  float* inLeft;
  float* inRight;
  float* inSurround;
  float* outLeft;
  float* outRight;
  float* outSurround;
  long remainingLeft, remainingRight, remainingSurround;
  remainingLeft = remainingRight = remainingSurround = sampleFrames;
  inLeft = inputs[0];
  inRight = inputs[1];
  inSurround = inputs[3];
  outLeft = outputs[0];
  outRight = outputs[1];
  outSurround = outputs[3];
  while (remainingLeft > 0 || remainingRight > 0 || remainingSurround > 0) {
    if (remainingLeft > 0) {
      writeSpace = mDelayBufferSamples - mLeftWritePosition;
      if (writeSpace < remainingLeft || mDelayBufferSamples - mLeftReadPosition < remainingLeft) {
        readSpace = mDelayBufferSamples - mLeftReadPosition;
        if (writeSpace < readSpace) {
          count = writeSpace;
        } else {
          count = readSpace;
        }
      } else {
        count = remainingLeft;
      }
      ProcessReplacingSamples(inLeft, outLeft, mLeftDelay.get() + mLeftWritePosition, mLeftDelay.get() + mLeftReadPosition, count);
      inLeft += count;
      outLeft += count;
      mLeftWritePosition += count;
      if (mLeftWritePosition >= mDelayBufferSamples) {
        mLeftWritePosition -= mDelayBufferSamples;
      }
      mLeftReadPosition += count;
      if (mLeftReadPosition >= mDelayBufferSamples) {
        mLeftReadPosition -= mDelayBufferSamples;
      }
      remainingLeft -= count;
      UpdateChannelLFO(count, 1);
    }
    if (remainingRight > 0) {
      writeSpace = mDelayBufferSamples - mRightWritePosition;
      if (writeSpace < remainingRight || mDelayBufferSamples - mRightReadPosition < remainingRight) {
        readSpace = mDelayBufferSamples - mRightReadPosition;
        if (writeSpace < readSpace) {
          count = writeSpace;
        } else {
          count = readSpace;
        }
      } else {
        count = remainingRight;
      }
      ProcessReplacingSamples(inRight, outRight, mRightDelay.get() + mRightWritePosition, mRightDelay.get() + mRightReadPosition, count);
      inRight += count;
      outRight += count;
      mRightWritePosition += count;
      if (mRightWritePosition >= mDelayBufferSamples) {
        mRightWritePosition -= mDelayBufferSamples;
      }
      mRightReadPosition += count;
      if (mRightReadPosition >= mDelayBufferSamples) {
        mRightReadPosition -= mDelayBufferSamples;
      }
      remainingRight -= count;
      UpdateChannelLFO(count, 2);
    }
    if (remainingSurround > 0) {
      writeSpace = mDelayBufferSamples - mSurroundWritePosition;
      if (writeSpace < remainingSurround || mDelayBufferSamples - mSurroundReadPosition < remainingSurround) {
        readSpace = mDelayBufferSamples - mSurroundReadPosition;
        if (writeSpace < readSpace) {
          count = writeSpace;
        } else {
          count = readSpace;
        }
      } else {
        count = remainingSurround;
      }
      ProcessReplacingSamples(inSurround, outSurround, mSurroundDelay.get() + mSurroundWritePosition, mSurroundDelay.get() + mSurroundReadPosition, count);
      inSurround += count;
      outSurround += count;
      mSurroundWritePosition += count;
      if (mSurroundWritePosition >= mDelayBufferSamples) {
        mSurroundWritePosition -= mDelayBufferSamples;
      }
      mSurroundReadPosition += count;
      if (mSurroundReadPosition >= mDelayBufferSamples) {
        mSurroundReadPosition -= mDelayBufferSamples;
      }
      remainingSurround -= count;
      UpdateChannelLFO(count, 3);
    }
  }
}
