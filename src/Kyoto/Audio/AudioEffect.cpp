#include "Kyoto/Audio/AudioEffect.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Audio/AEffEditor.hpp"

#include <math.h>
#include <stdio.h>
#include <string.h>

long dispatchEffectClass(AEffect* effect, long opcode, long index, long value, void* ptr,
                         float option) {
  AudioEffect* object = static_cast< AudioEffect* >(effect->mObject);
  if (opcode == kEO_Close) {
    object->dispatcher(opcode, index, value, ptr, option);
    delete object;
    return 1;
  }
  return object->dispatcher(opcode, index, value, ptr, option);
}

float getParameterClass(AEffect* effect, long index) {
  return static_cast< AudioEffect* >(effect->mObject)->getParameter(index);
}

void setParameterClass(AEffect* effect, long index, float value) {
  static_cast< AudioEffect* >(effect->mObject)->setParameter(index, value);
}

void processClass(AEffect* effect, float** inputs, float** outputs, long sampleFrames) {
  static_cast< AudioEffect* >(effect->mObject)->process(inputs, outputs, sampleFrames);
}

void processClassReplacing(AEffect* effect, float** inputs, float** outputs, long sampleFrames) {
  static_cast< AudioEffect* >(effect->mObject)->processReplacing(inputs, outputs, sampleFrames);
}

AudioEffect::AudioEffect(AudioMasterCallback audioMaster, long numPrograms, long numParameters)
: mEditor(nullptr)
, mAudioMaster(audioMaster)
, mNumPrograms(numPrograms)
, mNumParameters(numParameters)
, mCurrentProgram(0) {
  memset(&mEffect, 0, sizeof(mEffect));
  mEffect.mMagic = 'VstP';
  mEffect.mDispatcher = dispatchEffectClass;
  mEffect.mProcess = processClass;
  mEffect.mSetParameter = setParameterClass;
  mEffect.mGetParameter = getParameterClass;
  mEffect.mNumPrograms = numPrograms;
  mEffect.mNumParameters = numParameters;
  mEffect.mNumInputs = 1;
  mEffect.mNumOutputs = 2;
  mEffect.mFlags = 0;
  mEffect.mReserved1 = 0;
  mEffect.mReserved2 = 0;
  mEffect.mInitialDelay = 0;
  mEffect.mRealtimeQualities = 0;
  mEffect.mOfflineQualities = 0;
  mEffect.mIORatio = 1.f;
  mEffect.mObject = this;
  mEffect.mUser = nullptr;
  mEffect.mUniqueId = 'NoEf';
  mEffect.mVersion = 1;
  mEffect.mProcessReplacing = processClassReplacing;
  mSampleRate = 44100.f;
  mBlockSize = 1024;
}

AudioEffect::~AudioEffect() { delete mEditor; }

long AudioEffect::dispatcher(long opcode, long index, long value, void* ptr, float option) {
  long result = 0;
  switch (opcode) {
  case kEO_Open:
    open();
    break;
  case kEO_Close:
    close();
    break;
  case kEO_SetProgram:
    if (value < mNumPrograms)
      setProgram(value);
    break;
  case kEO_GetProgram:
    result = getProgram();
    break;
  case kEO_SetProgramName:
    setProgramName(static_cast< char* >(ptr));
    break;
  case kEO_GetProgramName:
    getProgramName(static_cast< char* >(ptr));
    break;
  case kEO_GetParameterLabel:
    getParameterLabel(index, static_cast< char* >(ptr));
    break;
  case kEO_GetParameterDisplay:
    getParameterDisplay(index, static_cast< char* >(ptr));
    break;
  case kEO_GetParameterName:
    getParameterName(index, static_cast< char* >(ptr));
    break;
  case kEO_GetVu:
    result = static_cast< long >(32767.0 * getVu());
    break;
  case kEO_SetSampleRate:
    setSampleRate(option);
    break;
  case kEO_SetBlockSize:
    setBlockSize(value);
    break;
  case kEO_MainsChanged:
    if (value == 0)
      suspend();
    else
      resume();
    break;
  case kEO_EditGetRect:
    if (mEditor != nullptr)
      result = mEditor->getRect(static_cast< ERect** >(ptr));
    break;
  case kEO_EditOpen:
    if (mEditor != nullptr)
      result = mEditor->open(ptr);
    break;
  case kEO_EditClose:
    if (mEditor != nullptr)
      mEditor->close();
    break;
  case kEO_EditIdle:
    if (mEditor != nullptr)
      mEditor->idle();
    break;
  case kEO_Identify:
    result = 'NvEf';
    break;
  case kEO_GetChunk:
    result = getChunk(static_cast< void** >(ptr), index != 0);
    break;
  case kEO_SetChunk:
    result = setChunk(ptr, value, index != 0);
    break;
  }
  return result;
}

long AudioEffect::getMasterVersion() {
  long version = 1;
  if (mAudioMaster != nullptr) {
    version = mAudioMaster(&mEffect, kAM_Version, 0, 0, nullptr, 0.f);
    if (version == 0)
      version = 1;
  }
  return version;
}

long AudioEffect::getCurrentUniqueId() {
  long id = 0;
  if (mAudioMaster != nullptr)
    id = mAudioMaster(&mEffect, kAM_CurrentId, 0, 0, nullptr, 0.f);
  return id;
}

void AudioEffect::masterIdle() {
  if (mAudioMaster != nullptr)
    mAudioMaster(&mEffect, kAM_Idle, 0, 0, nullptr, 0.f);
}

bool AudioEffect::isOutputConnected(long output) {
  long result = 0;
  if (mAudioMaster != nullptr)
    result = mAudioMaster(&mEffect, kAM_PinConnected, output, 0, nullptr, 0.f);
  return result == 0;
}

bool AudioEffect::isInputConnected(long input) {
  long result = 0;
  if (mAudioMaster != nullptr)
    result = mAudioMaster(&mEffect, kAM_PinConnected, input, 1, nullptr, 0.f);
  return result == 0;
}

void AudioEffect::canProcessReplacing(bool state) {
  if (state)
    mEffect.mFlags |= kEF_CanReplacing;
  else
    mEffect.mFlags &= ~kEF_CanReplacing;
}

void AudioEffect::programsAreChunks(bool state) {
  if (state)
    mEffect.mFlags |= kEF_ProgramChunks;
  else
    mEffect.mFlags &= ~kEF_ProgramChunks;
}

void AudioEffect::canMono(bool state) {
  if (state)
    mEffect.mFlags |= kEF_CanMono;
  else
    mEffect.mFlags &= ~kEF_CanMono;
}

void AudioEffect::hasVu(bool state) {
  if (state)
    mEffect.mFlags |= kEF_HasVu;
  else
    mEffect.mFlags &= ~kEF_HasVu;
}

void AudioEffect::hasClip(bool state) {
  if (state)
    mEffect.mFlags |= kEF_HasClip;
  else
    mEffect.mFlags &= ~kEF_HasClip;
}

void AudioEffect::dB2string(float value, char* text) {
  if (value <= 0.f)
    strcpy(text, "  -oo   ");
  else
    float2string(static_cast< float >(20.0 * static_cast< float >(log10(value))), text);
}

void AudioEffect::Hz2string(float samples, char* text) {
  float sampleRate = getSampleRate();
  if (samples == 0.f)
    float2string(0.f, text);
  else
    float2string(sampleRate / samples, text);
}

void AudioEffect::ms2string(float samples, char* text) {
  float sampleRate = getSampleRate();
  float2string(static_cast< float >(1000.0 * samples / sampleRate), text);
}

void AudioEffect::float2string(float value, char* text) {
  char buffer[32];
  double magnitude = value;
  bool negative = false;
  long length = 0;
  if (magnitude < 0.0) {
    magnitude = -magnitude;
    negative = true;
    length = 1;
    if (magnitude > 9999999.0) {
      strcpy(buffer, " Huge!  ");
      return;
    }
  } else if (magnitude > 99999999.0) {
    strcpy(buffer, " Huge!  ");
    return;
  }

  buffer[31] = 0;
  buffer[30] = '.';
  double integer = floor(magnitude);
  buffer[29] = static_cast< char >(static_cast< long >(fmod(integer, 10.0)) + '0');
  integer /= 10.0;
  length += 2;
  char* digit = buffer + 28;
  while (integer >= 1.0 && length < 8) {
    *digit-- = static_cast< char >(static_cast< long >(fmod(integer, 10.0)) + '0');
    integer /= 10.0;
    ++length;
  }
  if (negative)
    *digit-- = '-';
  strcpy(text, digit + 1);

  if (length < 8) {
    buffer[31] = 0;
    digit = buffer + 30;
    double fraction = fmod(magnitude, 1.0);
    fraction *= pow(10.0, 8 - length);
    while (length < 8) {
      if (fraction <= 0.0)
        *digit-- = '0';
      else {
        *digit-- = static_cast< char >(static_cast< long >(fmod(fraction, 10.0)) + '0');
        fraction /= 10.0;
      }
      ++length;
    }
    strcat(text, digit + 1);
  }
}

void AudioEffect::int2string(long value, char* text) {
  if (value >= 100000000)
    strcpy(text, " Huge!  ");
  else {
    char buffer[9];
    sprintf(buffer, "%7d", value);
    buffer[8] = 0;
    strcpy(text, buffer);
  }
}

void AudioEffect::setParameterAutomated(long index, float value) {
  setParameter(index, value);
  if (mAudioMaster != nullptr)
    mAudioMaster(&mEffect, kAM_Automate, index, 0, nullptr, value);
}
