#ifndef _CFLANGER
#define _CFLANGER

#include "Kyoto/Audio/AudioEffectX.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/reserved_vector.hpp"

// Guessed class/record/helper names, supported by native parameter labels and delay/LFO behavior.
class CFlanger : public AudioEffectX {
public:
  explicit CFlanger(AudioMasterCallback audioMaster);

  // AudioEffect
  ~CFlanger() override;
  void setParameter(long index, float value) override;
  float getParameter(long index) override;
  void process(float** inputs, float** outputs, long sampleFrames) override;
  void processReplacing(float** inputs, float** outputs, long sampleFrames) override;
  void setProgram(long program) override;
  void setProgramName(char* name) override;
  void getProgramName(char* name) override;
  void getParameterLabel(long index, char* label) override;
  void getParameterDisplay(long index, char* text) override;
  void getParameterName(long index, char* name) override;
  float getVu() override;
  void suspend() override;

private:
  struct SProgram {
    SProgram();
    float mDelay;
    float mFeedback;
    float mOutput;
    float mLFOFrequencyComplement;
    float mLFODepth;
    float mLFOWave;
    float mDry;
    float mLFOPhase;
    char mName[24];
  };

  void ProcessReplacingSamples(float* input, float* output, float* delayWrite, float* delayRead,
                               long sampleFrames);
  void ProcessSamples(float* input, float* output, float* delayWrite, float* delayRead,
                      long sampleFrames);
  void UpdateChannelLFO(long sampleFrames, long channel);
  float GetLFOFrequency();
  void SetLFOFrequency(float frequency);
  void SetLFOPhase(float phase);
  void SetDelay(float delay);
  void SetChannelDelay(long channel, float delay);

  rstl::reserved_vector< SProgram, 16 > mPrograms;
  rstl::auto_ptr< float > mLeftDelay;
  rstl::auto_ptr< float > mRightDelay;
  rstl::auto_ptr< float > mSurroundDelay;
  float mDelay;
  float mFeedback;
  float mOutput;
  float mLFOFrequencyComplement;
  float mLFODepth;
  float mLFOWave;
  float mDry;
  float mLFOPhase;
  float mLFOPeriod;
  int mLeftLFOCounter;
  int mRightLFOCounter;
  int mSurroundLFOCounter;
  float mPeak;
  int mDelaySamples;
  int mLFOPhasePercent;
  int mDelayBufferSamples;
  int mLeftWritePosition;
  int mRightWritePosition;
  int mSurroundWritePosition;
  int mLeftReadPosition;
  int mRightReadPosition;
  int mSurroundReadPosition;
};
CHECK_SIZEOF(CFlanger, 0x4a4)

#endif // _CFLANGER
