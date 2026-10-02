#ifndef _CAUXEFFECTPARAMETERS
#define _CAUXEFFECTPARAMETERS

#include "types.h"

// Guessed names. The leading processing state and flag still need identification.
struct SFlangerAuxParameters {
  uint x0_[5];
  bool x14_;
  float mDelay;
  float mFeedback;
  float mOut;
  float mLFOFrequency;
  float mLFODepth;
  float mLFOWave;
  float mDry;
  float mDelayPhase;
};

struct SBitcrusherAuxParameters {
  uint x0_[5];
  bool x14_;
  float x18_;
  float mGain;
  float mBitDepth;
  float mSampleRateReduction;
};

struct SPhaserAuxParameters {
  uint x0_[5];
  bool x14_;
  float mFrequency;
  float mFeedback;
  float mInvert;
  float mWet;
  float mDry;
  float mSweep;
};

CHECK_SIZEOF(SFlangerAuxParameters, 0x38)
CHECK_SIZEOF(SBitcrusherAuxParameters, 0x28)
CHECK_SIZEOF(SPhaserAuxParameters, 0x30)

#endif // _CAUXEFFECTPARAMETERS
