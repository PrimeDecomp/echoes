#include "Kyoto/Audio/AudioEffectX.hpp"
#include "Kyoto/Alloc/CMemory.hpp"

AudioEffectX::AudioEffectX(AudioMasterCallback audioMaster, long numPrograms, long numParameters)
: AudioEffect(audioMaster, numPrograms, numParameters) {}

AudioEffectX::~AudioEffectX() {}

long AudioEffectX::dispatcher(long opcode, long index, long value, void* ptr, float option) {
  switch (opcode) {
  case kEO_ProcessEvents:
    return processEvents(static_cast< VstEvents* >(ptr));
  case kEO_CanBeAutomated:
    return canParameterBeAutomated(index) ? 1 : 0;
  case kEO_StringToParameter:
    return string2parameter(index, static_cast< char* >(ptr)) ? 1 : 0;
  case kEO_GetNumCategories:
    return getNumCategories();
  case kEO_GetProgramNameIndexed:
    return getProgramNameIndexed(value, index, static_cast< char* >(ptr)) ? 1 : 0;
  case kEO_CopyProgram:
    return copyProgram(index) ? 1 : 0;
  case kEO_ConnectInput:
    inputConnected(index, value != 0);
    return 1;
  case kEO_ConnectOutput:
    outputConnected(index, value != 0);
    return 1;
  case kEO_GetInputProperties:
    return getInputProperties(index, static_cast< VstPinProperties* >(ptr)) ? 1 : 0;
  case kEO_GetOutputProperties:
    return getOutputProperties(index, static_cast< VstPinProperties* >(ptr)) ? 1 : 0;
  case kEO_GetPlugCategory:
    return getPlugCategory();
  case kEO_GetCurrentPosition:
    return reportCurrentPosition();
  case kEO_GetDestinationBuffer:
    return reinterpret_cast< long >(reportDestinationBuffer());
  case kEO_OfflineNotify:
    return offlineNotify(static_cast< VstAudioFile* >(ptr), value, index != 0);
  case kEO_OfflinePrepare:
    return offlinePrepare(static_cast< VstOfflineTask* >(ptr), value);
  case kEO_OfflineRun:
    return offlineRun(static_cast< VstOfflineTask* >(ptr), value);
  case kEO_ProcessVariableIo:
    return processVariableIo(static_cast< VstVariableIo* >(ptr)) ? 1 : 0;
  case kEO_SetSpeakerArrangement:
    return setSpeakerArrangement(reinterpret_cast< VstSpeakerArrangement* >(value),
                                 static_cast< VstSpeakerArrangement* >(ptr))
               ? 1
               : 0;
  case kEO_SetBlockSizeAndSampleRate:
    setBlockSizeAndSampleRate(value, option);
    return 1;
  case kEO_SetBypass:
    return setBypass(value != 0) ? 1 : 0;
  case kEO_GetEffectName:
    return getEffectName(static_cast< char* >(ptr)) ? 1 : 0;
  case kEO_GetErrorText:
    return getErrorText(static_cast< char* >(ptr)) ? 1 : 0;
  case kEO_GetVendorString:
    return getVendorString(static_cast< char* >(ptr)) ? 1 : 0;
  case kEO_GetProductString:
    return getProductString(static_cast< char* >(ptr)) ? 1 : 0;
  case kEO_GetVendorVersion:
    return getVendorVersion();
  case kEO_VendorSpecific:
    return vendorSpecific(index, value, ptr, option);
  case kEO_CanDo:
    return canDo(static_cast< char* >(ptr));
  case kEO_GetTailSize:
    return getGetTailSize();
  case kEO_Idle:
    return fxIdle();
  case kEO_GetIcon:
    return reinterpret_cast< long >(getIcon());
  case kEO_SetViewPosition:
    return setViewPosition(index, value) ? 1 : 0;
  case kEO_GetParameterProperties:
    return getParameterProperties(index, static_cast< VstParameterProperties* >(ptr)) ? 1 : 0;
  case kEO_KeysRequired:
    return !keysRequired();
  case kEO_GetVstVersion:
    return getVstVersion();
  default:
    return AudioEffect::dispatcher(opcode, index, value, ptr, option);
  }
}

void AudioEffectX::wantEvents(long filter) {
  if (mAudioMaster != nullptr)
    mAudioMaster(&mEffect, kAM_WantMidi, 0, filter, nullptr, 0.f);
}

VstTimeInfo* AudioEffectX::getTimeInfo(long filter) {
  if (mAudioMaster != nullptr)
    return reinterpret_cast< VstTimeInfo* >(mAudioMaster(&mEffect, kAM_GetTime, 0, filter, nullptr, 0.f));
  return nullptr;
}

long AudioEffectX::tempoAt(long position) {
  if (mAudioMaster != nullptr)
    return mAudioMaster(&mEffect, kAM_TempoAt, 0, position, nullptr, 0.f);
  return 0;
}

long AudioEffectX::getNumAutomatableParameters() {
  if (mAudioMaster != nullptr)
    return mAudioMaster(&mEffect, kAM_GetNumAutomatableParameters, 0, 0, nullptr, 0.f);
  return 0;
}

long AudioEffectX::getParameterQuantization() {
  if (mAudioMaster != nullptr)
    return mAudioMaster(&mEffect, kAM_GetParameterQuantization, 0, 0, nullptr, 0.f);
  return 0;
}

bool AudioEffectX::ioChanged() {
  if (mAudioMaster != nullptr)
    return mAudioMaster(&mEffect, kAM_IOChanged, 0, 0, nullptr, 0.f) != 0;
  return false;
}

bool AudioEffectX::needIdle() {
  if (mAudioMaster != nullptr)
    return mAudioMaster(&mEffect, kAM_NeedIdle, 0, 0, nullptr, 0.f) != 0;
  return false;
}

bool AudioEffectX::sizeWindow(long width, long height) {
  if (mAudioMaster != nullptr)
    return mAudioMaster(&mEffect, kAM_SizeWindow, width, height, nullptr, 0.f) != 0;
  return false;
}

float AudioEffectX::updateSampleRate() {
  if (mAudioMaster != nullptr)
    mAudioMaster(&mEffect, kAM_GetSampleRate, 0, 0, nullptr, 0.f);
  return mSampleRate;
}

long AudioEffectX::updateBlockSize() {
  if (mAudioMaster != nullptr)
    mAudioMaster(&mEffect, kAM_GetBlockSize, 0, 0, nullptr, 0.f);
  return mBlockSize;
}

long AudioEffectX::getInputLatency() {
  if (mAudioMaster != nullptr)
    return mAudioMaster(&mEffect, kAM_GetInputLatency, 0, 0, nullptr, 0.f);
  return 0;
}

long AudioEffectX::getOutputLatency() {
  if (mAudioMaster != nullptr)
    return mAudioMaster(&mEffect, kAM_GetOutputLatency, 0, 0, nullptr, 0.f);
  return 0;
}

AEffect* AudioEffectX::getPreviousPlug(long input) {
  if (mAudioMaster != nullptr)
    return reinterpret_cast< AEffect* >(mAudioMaster(&mEffect, kAM_GetPreviousPlug, 0, 0, nullptr, 0.f));
  return nullptr;
}

AEffect* AudioEffectX::getNextPlug(long output) {
  if (mAudioMaster != nullptr)
    return reinterpret_cast< AEffect* >(mAudioMaster(&mEffect, kAM_GetNextPlug, 0, 0, nullptr, 0.f));
  return nullptr;
}

long AudioEffectX::willProcessReplacing() {
  if (mAudioMaster != nullptr)
    return mAudioMaster(&mEffect, kAM_WillReplaceOrAccumulate, 0, 0, nullptr, 0.f);
  return 0;
}

long AudioEffectX::getCurrentProcessLevel() {
  if (mAudioMaster != nullptr)
    return mAudioMaster(&mEffect, kAM_GetCurrentProcessLevel, 0, 0, nullptr, 0.f);
  return 0;
}

long AudioEffectX::getAutomationState() {
  if (mAudioMaster != nullptr)
    return mAudioMaster(&mEffect, kAM_GetAutomationState, 0, 0, nullptr, 0.f);
  return 0;
}

void AudioEffectX::wantAsyncOperation(bool state) {
  if (state)
    mEffect.mFlags |= kEF_Async;
  else
    mEffect.mFlags &= ~kEF_Async;
}

void AudioEffectX::hasExternalBuffer(bool state) {
  if (state)
    mEffect.mFlags |= kEF_ExternalBuffer;
  else
    mEffect.mFlags &= ~kEF_ExternalBuffer;
}

bool AudioEffectX::offlineRead(VstOfflineTask* task, VstOfflineOption option, bool readSource) {
  if (mAudioMaster != nullptr)
    return mAudioMaster(&mEffect, kAM_OfflineRead, readSource, option, task, 0.f) != 0;
  return false;
}

bool AudioEffectX::offlineWrite(VstOfflineTask* task, VstOfflineOption option) {
  if (mAudioMaster != nullptr)
    return mAudioMaster(&mEffect, kAM_OfflineWrite, 0, option, task, 0.f) != 0;
  return false;
}

bool AudioEffectX::offlineStart(VstAudioFile* files, long count, long newCount) {
  if (mAudioMaster != nullptr)
    return mAudioMaster(&mEffect, kAM_OfflineStart, newCount, count, files, 0.f) != 0;
  return false;
}

bool AudioEffectX::offlineGetCurrentPass() {
  if (mAudioMaster != nullptr)
    return mAudioMaster(&mEffect, kAM_OfflineGetCurrentPass, 0, 0, nullptr, 0.f) != 0;
  return false;
}

bool AudioEffectX::offlineGetCurrentMetaPass() {
  if (mAudioMaster != nullptr)
    return mAudioMaster(&mEffect, kAM_OfflineGetCurrentMetaPass, 0, 0, nullptr, 0.f) != 0;
  return false;
}

void AudioEffectX::setOutputSamplerate(float sampleRate) {
  if (mAudioMaster != nullptr)
    mAudioMaster(&mEffect, kAM_SetOutputSampleRate, 0, 0, nullptr, sampleRate);
}

bool AudioEffectX::getSpeakerArrangement(VstSpeakerArrangement* input,
                                         VstSpeakerArrangement* output) {
  if (mAudioMaster != nullptr)
    return mAudioMaster(&mEffect, kAM_GetSpeakerArrangement, 0, reinterpret_cast< long >(input),
                          output, 0.f) != 0;
  return false;
}

bool AudioEffectX::getHostVendorString(char* text) {
  if (mAudioMaster != nullptr)
    return mAudioMaster(&mEffect, kAM_GetVendorString, 0, 0, text, 0.f) != 0;
  return false;
}

bool AudioEffectX::getHostProductString(char* text) {
  if (mAudioMaster != nullptr)
    return mAudioMaster(&mEffect, kAM_GetProductString, 0, 0, text, 0.f) != 0;
  return false;
}

long AudioEffectX::getHostVendorVersion() {
  if (mAudioMaster != nullptr)
    return mAudioMaster(&mEffect, kAM_GetVendorVersion, 0, 0, nullptr, 0.f);
  return 0;
}

long AudioEffectX::hostVendorSpecific(long firstArgument, long secondArgument, void* ptr,
                                      float option) {
  if (mAudioMaster != nullptr)
    return mAudioMaster(&mEffect, kAM_VendorSpecific, firstArgument, secondArgument, ptr, option);
  return 0;
}

bool AudioEffectX::canHostDo(char* text) {
  if (mAudioMaster != nullptr)
    return mAudioMaster(&mEffect, kAM_CanDo, 0, 0, text, 0.f) != 0;
  return false;
}

void AudioEffectX::isSynth(bool state) {
  if (state)
    mEffect.mFlags |= kEF_IsSynth;
  else
    mEffect.mFlags &= ~kEF_IsSynth;
}

void AudioEffectX::noTail(bool state) {
  if (state)
    mEffect.mFlags |= kEF_NoSoundInStop;
  else
    mEffect.mFlags &= ~kEF_NoSoundInStop;
}

long AudioEffectX::getHostLanguage() {
  if (mAudioMaster != nullptr)
    return mAudioMaster(&mEffect, kAM_GetLanguage, 0, 0, nullptr, 0.f);
  return 0;
}

void* AudioEffectX::openWindow(VstWindow* window) {
  if (mAudioMaster != nullptr)
    return reinterpret_cast< void* >(mAudioMaster(&mEffect, kAM_OpenWindow, 0, 0, window, 0.f));
  return nullptr;
}

bool AudioEffectX::closeWindow(VstWindow* window) {
  if (mAudioMaster != nullptr)
    return mAudioMaster(&mEffect, kAM_CloseWindow, 0, 0, window, 0.f) != 0;
  return false;
}

void* AudioEffectX::getDirectory() {
  if (mAudioMaster != nullptr)
    return reinterpret_cast< void* >(mAudioMaster(&mEffect, kAM_GetDirectory, 0, 0, nullptr, 0.f));
  return nullptr;
}

bool AudioEffectX::updateDisplay() {
  if (mAudioMaster != nullptr)
    return mAudioMaster(&mEffect, kAM_UpdateDisplay, 0, 0, nullptr, 0.f) != 0;
  return false;
}
