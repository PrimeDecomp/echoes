#ifndef _CPHASER
#define _CPHASER

#include "Kyoto/Audio/AudioEffectX.hpp"

// Guessed name, supported by the native effect name and four-stage all-pass filters.
class CPhaser : public AudioEffectX {
public:
  explicit CPhaser(AudioMasterCallback audioMaster);

  // AudioEffect
  ~CPhaser() override;
  void setParameter(long index, float value) override;
  float getParameter(long index) override;
  void process(float** inputs, float** outputs, long sampleFrames) override;
  void processReplacing(float** inputs, float** outputs, long sampleFrames) override;
  void setProgramName(char* name) override;
  void getProgramName(char* name) override;
  void getParameterLabel(long index, char* label) override;
  void getParameterDisplay(long index, char* text) override;
  void getParameterName(long index, char* name) override;

  // AudioEffectX
  bool getEffectName(char* text) override;
  bool getVendorString(char* text) override;
  bool getProductString(char* text) override;

private:
  struct SFilterHistory {
    double mOutputs[4];
    double mInputs[4];
  };
  struct SFilterCoefficient {
    double mCoefficient;
    double mAngularFrequencyRatio;
  };
  struct SInput {
    double mFeedbackInput;
    double mRawInput;
  };

  float mFrequency;
  float mFeedback;
  float mInvert;
  float mWet;
  float mDry;
  float mSweepRange;
  SFilterHistory mHistory[3];
  SFilterCoefficient mCoefficients[3];
  SInput mInputs[3];
  double x1e8_; // Constructor-only zero stores; no runtime consumers identified.
  double x1f0_;
  double x1f8_;
  double mBaseFrequency;
  double mChannelFrequency[3];
  double mFeedbackSign;
  double mInitialFeedback; // Constructor snapshot; runtime use not identified.
  double mLFORate;
  double mLFOPhaseStep;
  double mLFOPhase;
  double mFullCircle;
  double mLeftLFO;
  double mLFOFraction;
  double mRightLFO;
  double mSurroundLFO;
  int mLFOIndex;
  int mTableBuildIndex;
  double mSineTable[2049];
  char mProgramName[32];
};
CHECK_SIZEOF(CPhaser, 0x42a0)

#endif // _CPHASER
