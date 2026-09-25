#ifndef _CPARTICLEDATA
#define _CPARTICLEDATA

#include "types.h"

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Math/CVector3f.hpp"

class CInputStream;
class CParticleData {
public:
  enum EParentedMode {
    kPM_Initial,
    kPM_ContinuousEmitter,
    kPM_ContinuousSystem,
  };

  CParticleData(int duration, const SObjectTag& tag, CSegId bone, float scale,
                EParentedMode mode)
  : mDuration(duration)
  , mParticle(tag)
  , mBone(bone)
  , mScale(scale)
  , mParentMode(mode) {}

  CParticleData(CInputStream& in);

private:
  int mDuration;
  SObjectTag mParticle;
  // Echoes stores a segment ID where Prime stored the bone name.
  CSegId mBone;
  float mScale;
  EParentedMode mParentMode;
};
CHECK_SIZEOF(CParticleData, 0x18)

class CAuxiliaryParticleData {
private:
  uint mDuration;
  SObjectTag mParticle;
  CVector3f mTranslation;
  float mScale;
};

#endif // _CPARTICLEDATA
