#ifndef _AUDIOEFFECTX
#define _AUDIOEFFECTX

#include "Kyoto/Audio/AudioEffect.hpp"

// SDK-correlated extended interface; target-specific slots/signatures differ from newer SDKs.
class AudioEffectX : public AudioEffect {
public:
  AudioEffectX(AudioMasterCallback audioMaster, long numPrograms, long numParameters);

  // AudioEffect
  ~AudioEffectX() override;
  long dispatcher(long opcode, long index, long value, void* ptr, float option) override;

  virtual void wantEvents(long filter = 1);
  virtual VstTimeInfo* getTimeInfo(long filter);
  virtual long tempoAt(long position);

  virtual long processEvents(VstEvents* events) { return 0; }

  virtual long getNumAutomatableParameters();
  virtual long getParameterQuantization();

  virtual bool canParameterBeAutomated(long index) { return true; }

  virtual bool string2parameter(long index, char* text) { return false; }

  virtual float getChannelParameter(long channel, long index) { return 0.f; }

  virtual long getNumCategories() { return 1; }

  virtual bool getProgramNameIndexed(long category, long index, char* text) { return false; }

  virtual bool copyProgram(long destination) { return false; }

  virtual bool ioChanged();
  virtual bool needIdle();
  virtual bool sizeWindow(long width, long height);
  virtual float updateSampleRate();
  virtual long updateBlockSize();
  virtual long getInputLatency();
  virtual long getOutputLatency();
  virtual AEffect* getPreviousPlug(long input);
  virtual AEffect* getNextPlug(long output);

  virtual void inputConnected(long index, bool state) {}

  virtual void outputConnected(long index, bool state) {}

  virtual bool getInputProperties(long index, VstPinProperties* properties) { return false; }

  virtual bool getOutputProperties(long index, VstPinProperties* properties) { return false; }

  virtual VstPlugCategory getPlugCategory() {
    return mEffect.mFlags & kEF_IsSynth ? kPlugCategSynth : kPlugCategUnknown;
  }

  virtual long willProcessReplacing();
  virtual long getCurrentProcessLevel();
  virtual long getAutomationState();
  virtual void wantAsyncOperation(bool state = true);
  virtual void hasExternalBuffer(bool state = true);

  virtual long reportCurrentPosition() { return 0; }

  virtual float* reportDestinationBuffer() { return nullptr; }

  virtual bool offlineRead(VstOfflineTask* task, VstOfflineOption option, bool readSource = true);
  virtual bool offlineWrite(VstOfflineTask* task, VstOfflineOption option);
  virtual bool offlineStart(VstAudioFile* files, long count, long newCount);
  virtual bool offlineGetCurrentPass();
  virtual bool offlineGetCurrentMetaPass();

  virtual bool offlineNotify(VstAudioFile* files, long count, bool start) { return false; }

  virtual bool offlinePrepare(VstOfflineTask* tasks, long count) { return false; }

  virtual bool offlineRun(VstOfflineTask* tasks, long count) { return false; }

  virtual long offlineGetNumPasses() { return 0; }

  virtual long offlineGetNumMetaPasses() { return 0; }

  virtual void setOutputSamplerate(float sampleRate);
  virtual bool getSpeakerArrangement(VstSpeakerArrangement* input, VstSpeakerArrangement* output);
  virtual bool getHostVendorString(char* text);
  virtual bool getHostProductString(char* text);
  virtual long getHostVendorVersion();
  virtual long hostVendorSpecific(long firstArgument, long secondArgument, void* ptr, float option);
  virtual bool canHostDo(char* text);
  virtual void isSynth(bool state = true);
  virtual void noTail(bool state = true);
  virtual long getHostLanguage();
  virtual void* openWindow(VstWindow* window);
  virtual bool closeWindow(VstWindow* window);
  virtual void* getDirectory();
  virtual bool updateDisplay();

  virtual bool processVariableIo(VstVariableIo* variableIo) { return false; }

  virtual bool setSpeakerArrangement(VstSpeakerArrangement* input, VstSpeakerArrangement* output) {
    return false;
  }

  virtual void setBlockSizeAndSampleRate(long blockSize, float sampleRate) {
    mBlockSize = blockSize;
    mSampleRate = sampleRate;
  }

  virtual bool setBypass(bool bypass) { return false; }

  virtual bool getEffectName(char* name) { return false; }

  virtual bool getErrorText(char* text) { return false; }

  virtual bool getVendorString(char* text) { return false; }

  virtual bool getProductString(char* text) { return false; }

  virtual long getVendorVersion() { return 0; }

  virtual long vendorSpecific(long firstArgument, long secondArgument, void* ptr, float option) {
    return 0;
  }

  virtual long canDo(char* text) { return 0; }

  virtual void* getIcon() { return nullptr; }

  virtual bool setViewPosition(long x, long y) { return false; }

  virtual long getGetTailSize() { return 0; }

  virtual long fxIdle() { return 0; }

  virtual bool getParameterProperties(long index, VstParameterProperties* properties) {
    return false;
  }

  virtual bool keysRequired() { return false; }

  virtual long getVstVersion() { return 2; }
};
CHECK_SIZEOF(AudioEffectX, 0xb0)

#endif // _AUDIOEFFECTX
