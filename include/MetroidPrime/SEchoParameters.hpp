#ifndef _SECHOPARAMETERS
#define _SECHOPARAMETERS

#include "types.h"

struct SEchoParameters {
  SEchoParameters(bool isEchoEmitter, bool onlyEmitDamage, uint numSoundWaves,
                  float spaceBetweenWaves, float waveLineSize, float forcedMinimumVis);

  uint mIsEchoEmitter : 1;
  uint mOnlyEmitDamage : 1;
  uint mNumSoundWaves : 30;
  float mSpaceBetweenWaves;
  float mWaveLineSize;
  float mForcedMinimumVis;
};
CHECK_SIZEOF(SEchoParameters, 0x10)

#endif // _SECHOPARAMETERS
