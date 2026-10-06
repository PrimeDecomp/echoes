#ifndef _AUDIOEFFECT
#define _AUDIOEFFECT

#include "Kyoto/Audio/AEffect.hpp"

class AEffEditor;

// SDK-correlated class/API names, not recovered Echoes exports. Native virtual order is retained.
class AudioEffect {
public:
  AudioEffect(AudioMasterCallback audioMaster, long numPrograms, long numParameters);
  virtual ~AudioEffect();

  virtual void setParameter(long index, float value) {}

  virtual float getParameter(long index) { return 0.f; }

  virtual void setParameterAutomated(long index, float value);
  virtual void process(float** inputs, float** outputs, long sampleFrames) = 0;

  virtual void processReplacing(float** inputs, float** outputs, long sampleFrames) {}

  virtual long dispatcher(long opcode, long index, long value, void* ptr, float option);

  virtual void open() {}

  virtual void close() {}

  virtual long getProgram() { return mCurrentProgram; }

  virtual void setProgram(long program) { mCurrentProgram = program; }

  virtual void setProgramName(char* name) { *name = 0; }

  virtual void getProgramName(char* name) { *name = 0; }

  virtual void getParameterLabel(long index, char* label) { *label = 0; }

  virtual void getParameterDisplay(long index, char* text) { *text = 0; }

  virtual void getParameterName(long index, char* name) { *name = 0; }

  virtual float getVu() { return 0.f; }

  virtual long getChunk(void** data, bool isPreset = false) { return 0; }

  virtual long setChunk(void* data, long byteSize, bool isPreset = false) { return 0; }

  virtual void setSampleRate(float sampleRate) { mSampleRate = sampleRate; }

  virtual void setBlockSize(long blockSize) { mBlockSize = blockSize; }

  virtual void suspend() {}

  virtual void resume() {}

  virtual void setUniqueID(long id) { mEffect.mUniqueId = id; }

  virtual void setNumInputs(long count) { mEffect.mNumInputs = count; }

  virtual void setNumOutputs(long count) { mEffect.mNumOutputs = count; }

  virtual void hasVu(bool state = true);
  virtual void hasClip(bool state = true);
  virtual void canMono(bool state = true);
  virtual void canProcessReplacing(bool state = true);
  virtual void programsAreChunks(bool state = true);

  virtual void setRealtimeQualities(long qualities) { mEffect.mRealtimeQualities = qualities; }

  virtual void setOfflineQualities(long qualities) { mEffect.mOfflineQualities = qualities; }

  virtual void setInitialDelay(long delay) { mEffect.mInitialDelay = delay; }

  virtual float getSampleRate() { return mSampleRate; }

  virtual long getBlockSize() { return mBlockSize; }

  virtual long getMasterVersion();
  virtual long getCurrentUniqueId();
  virtual void masterIdle();
  virtual bool isOutputConnected(long output);
  virtual bool isInputConnected(long input);
  virtual void dB2string(float value, char* text);
  virtual void Hz2string(float samples, char* text);
  virtual void ms2string(float samples, char* text);
  virtual void float2string(float value, char* text);
  virtual void int2string(long value, char* text);

  AEffect* getAeffect() { return &mEffect; }

protected:
  float mSampleRate;
  AEffEditor* mEditor;
  AudioMasterCallback mAudioMaster;
  long mNumPrograms;
  long mNumParameters;
  long mCurrentProgram;
  long mBlockSize;
  AEffect mEffect;
};
CHECK_SIZEOF(AudioEffect, 0xb0)

#endif // _AUDIOEFFECT
