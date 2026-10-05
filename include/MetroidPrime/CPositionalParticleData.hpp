#ifndef _CPOSITIONALPARTICLEDATA
#define _CPOSITIONALPARTICLEDATA

#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/SObjectTag.hpp"

// Original class name exported by the Echoes Wii build.
class CPositionalParticleData {
public:
  int GetDuration() const { return mDuration; }
  const SObjectTag& GetParticleAssetInfo() const { return mParticle; }
  const CTransform4f& GetTransform() const { return mTransform; }
  float GetScale() const { return mScale; }

private:
  int mDuration;
  SObjectTag mParticle;
  CTransform4f mTransform;
  float mScale;
};
CHECK_SIZEOF(CPositionalParticleData, 0x40)

#endif // _CPOSITIONALPARTICLEDATA
