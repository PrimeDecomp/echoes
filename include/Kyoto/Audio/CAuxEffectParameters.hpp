#ifndef _CAUXEFFECTPARAMETERS
#define _CAUXEFFECTPARAMETERS

#include "types.h"
#include <musyx/musyx.h>

// Guessed types and member names, based on the native processor and VstP callback descriptor.
class CAuxEffectProcessor;
struct SAuxEffectDescriptor;

struct SAuxEffectProcessingState {
  CAuxEffectProcessor* mProcessor;
  SAuxEffectDescriptor* mEffectDescriptor;
  float* mLeftBuffer;
  float* mRightBuffer;
  float* mSurroundBuffer;
  bool mProcessReplacing;
};
CHECK_SIZEOF(SAuxEffectProcessingState, 0x18)

struct SFlangerAuxParameters {
  SAuxEffectProcessingState mProcessing;
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
  SAuxEffectProcessingState mProcessing;
  float mDistortionType; // Guessed name; native parameter label is "Dist type".
  float mGain;
  float mBitDepth;
  float mSampleRateReduction;
};

struct SPhaserAuxParameters {
  SAuxEffectProcessingState mProcessing;
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

// Guessed name and fields, recovered from the three-channel filtered-delay callback.
struct SFilteredDelayAuxParameters {
  u32 mBlockCounts[3];
  u32 mBlockPositions[3];
  u32 mCurrentFeedback[3];
  u32 mCurrentOutput[3];
  s32 mLowPassCoefficient;
  s32 mHighPassCoefficient;
  s32* mDelayBuffers[3];
  s32 mLowPassHistory[3];
  s32 mHighPassHistory[3];
  u32 mDelayMs[3];
  u32 mFeedbackPercent[3];
  u32 mOutputPercent[3];
  u32 mLowPassFrequency;
  u32 mHighPassFrequency;
};
CHECK_SIZEOF(SFilteredDelayAuxParameters, 0x88)

// Guessed callback names; implementations remain in their native processing TUs.
bool PrepareFlangerAux(SFlangerAuxParameters* parameters);
bool ShutdownFlangerAux(SFlangerAuxParameters* parameters);
bool PrepareBitcrusherAux(SBitcrusherAuxParameters* parameters);
bool ShutdownBitcrusherAux(SBitcrusherAuxParameters* parameters);
bool PreparePhaserAux(SPhaserAuxParameters* parameters);
bool ShutdownPhaserAux(SPhaserAuxParameters* parameters);
void ProcessCustomAux(uchar reason, SND_AUX_INFO* info, SAuxEffectProcessingState* processing);
void PrepareFilteredDelayAux(SFilteredDelayAuxParameters* parameters);
void ShutdownFilteredDelayAux(SFilteredDelayAuxParameters* parameters);
void ProcessFilteredDelayAux(uchar reason, SND_AUX_INFO* info,
                             SFilteredDelayAuxParameters* parameters);

#endif // _CAUXEFFECTPARAMETERS
