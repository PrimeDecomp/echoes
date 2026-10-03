#ifndef _CBITCRUSHER
#define _CBITCRUSHER

#include "Kyoto/Audio/AudioEffectX.hpp"

// Guessed name, supported by the native program name and quantization parameters.
class CBitcrusher : public AudioEffectX {
public:
  explicit CBitcrusher(AudioMasterCallback audioMaster);

  // AudioEffect
  ~CBitcrusher() override;
  void setParameter(long index, float value) override;
  float getParameter(long index) override;
  void process(float** inputs, float** outputs, long sampleFrames) override;
  void processReplacing(float** inputs, float** outputs, long sampleFrames) override;
  void setProgramName(char* name) override;
  void getProgramName(char* name) override;
  void getParameterLabel(long index, char* label) override;
  void getParameterDisplay(long index, char* text) override;
  void getParameterName(long index, char* name) override;

private:
  float mDistortionType;
  float mGain;
  float mBitDepth;
  float mSampleRateReduction;
  uint xc0_; // No accesses identified; storage type and purpose remain unresolved.
  uint xc4_;
  uint xc8_;
  float mHeldSamples[3];
  int mSampleCounter;
  char mProgramName[32];
};
CHECK_SIZEOF(CBitcrusher, 0xfc)

#endif // _CBITCRUSHER
