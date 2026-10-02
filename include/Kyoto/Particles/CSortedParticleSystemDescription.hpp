#ifndef _CSORTEDPARTICLESYSTEMDESCRIPTION
#define _CSORTEDPARTICLESYSTEMDESCRIPTION

#include "types.h"

class CSpawnSystemKeyframeData;

// Guessed class name; the SRSM reader owns only the SPWN keyframe property.
class CSortedParticleSystemDescription {
public:
  CSortedParticleSystemDescription();
  ~CSortedParticleSystemDescription();

  CSpawnSystemKeyframeData* mSPWN;
};
CHECK_SIZEOF(CSortedParticleSystemDescription, 0x4)

#endif // _CSORTEDPARTICLESYSTEMDESCRIPTION
