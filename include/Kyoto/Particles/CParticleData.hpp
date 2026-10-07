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

  CParticleData(int duration = 0, const SObjectTag& tag = SObjectTag(0, 0),
                const CSegId& bone = CSegId(0), float scale = 1.f, EParentedMode mode = kPM_Initial)
  : mDuration(duration), mParticle(tag), mBone(bone), mScale(scale), mParentMode(mode) {}

  CParticleData(CInputStream& in);

  int GetDuration() const { return mDuration; }
  const SObjectTag& GetParticleAssetInfo() const { return mParticle; }
  CSegId GetSegmentId() const { return mBone; }
  float GetScale() const { return mScale; }
  EParentedMode GetParentedMode() const { return mParentMode; }

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
