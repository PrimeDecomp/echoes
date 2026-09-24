#ifndef _CEFFECTCOMPONENT
#define _CEFFECTCOMPONENT

#include "Kyoto/Particles/CParticleData.hpp"

class CEffectComponent {
public:
  explicit CEffectComponent(CInputStream& in);

  uint GetComponentNameHash() const { return x0_nameHash; }
  const SObjectTag& GetParticleTag() const { return x4_tag; }
  CSegId GetSegmentId() const { return xc_bone; }
  float GetScale() const { return x10_scale; }
  CParticleData::EParentedMode GetParentedMode() const { return x14_parentedMode; }
  uint GetFlags() const { return x18_flags; }

private:
  SObjectTag GetSObjectTagFromStream(CInputStream& in);

  // Echoes keeps a CPOINode string hash and segment ID instead of Prime's names.
  uint x0_nameHash;
  SObjectTag x4_tag;
  CSegId xc_bone;
  float x10_scale;
  CParticleData::EParentedMode x14_parentedMode;
  uint x18_flags;
};
CHECK_SIZEOF(CEffectComponent, 0x1c)

#endif // _CEFFECTCOMPONENT
