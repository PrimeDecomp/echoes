#ifndef _CPOSITIONALPARTICLEDATA
#define _CPOSITIONALPARTICLEDATA

#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/SObjectTag.hpp"

// Original class name exported by the Echoes Wii build.
class CPositionalParticleData {
private:
  int mDuration;
  SObjectTag mParticle;
  CTransform4f mTransform;
  float mScale;
};
CHECK_SIZEOF(CPositionalParticleData, 0x40)

#endif // _CPOSITIONALPARTICLEDATA
