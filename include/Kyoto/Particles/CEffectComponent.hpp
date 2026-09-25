#ifndef _CEFFECTCOMPONENT
#define _CEFFECTCOMPONENT

#include "Kyoto/Particles/CParticleData.hpp"

class CEffectComponent {
public:
  explicit CEffectComponent(CInputStream& in);

  uint GetComponentNameHash() const { return mNameHash; }
  const SObjectTag& GetParticleTag() const { return mTag; }
  CSegId GetSegmentId() const { return mBone; }
  float GetScale() const { return mScale; }
  CParticleData::EParentedMode GetParentedMode() const { return mParentedMode; }
  uint GetFlags() const { return mFlags; }

private:
  SObjectTag GetSObjectTagFromStream(CInputStream& in);

  // Echoes keeps a CPOINode string hash and segment ID instead of Prime's names.
  uint mNameHash;
  SObjectTag mTag;
  CSegId mBone;
  float mScale;
  CParticleData::EParentedMode mParentedMode;
  uint mFlags;
};
CHECK_SIZEOF(CEffectComponent, 0x1c)

#endif // _CEFFECTCOMPONENT
