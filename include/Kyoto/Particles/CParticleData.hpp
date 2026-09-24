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
  : x0_duration(duration)
  , x4_particle(tag)
  , xc_bone(bone)
  , x10_scale(scale)
  , x14_parentMode(mode) {}

  CParticleData(CInputStream& in);

private:
  int x0_duration;
  SObjectTag x4_particle;
  // Echoes stores a segment ID where Prime stored the bone name.
  CSegId xc_bone;
  float x10_scale;
  EParentedMode x14_parentMode;
};
CHECK_SIZEOF(CParticleData, 0x18)

class CAuxiliaryParticleData {
private:
  uint x0_duration;
  SObjectTag x4_particle;
  CVector3f xc_translation;
  float x18_scale;
};

#endif // _CPARTICLEDATA
