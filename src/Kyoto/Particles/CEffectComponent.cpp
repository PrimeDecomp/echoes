#include "Kyoto/Particles/CEffectComponent.hpp"

#include "Kyoto/Animation/CPOINode.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

CEffectComponent::CEffectComponent(CInputStream& in)
: x0_nameHash(CPOINode::GetHashForString(rstl::string(in).data()))
, x4_tag(GetSObjectTagFromStream(in))
, xc_bone(in)
, x10_scale(in.Get< float >())
, x14_parentedMode(CParticleData::EParentedMode(in.Get< uint >()))
, x18_flags(in.Get< uint >()) {}

SObjectTag CEffectComponent::GetSObjectTagFromStream(CInputStream& in) {
  const FourCC type = in.Get< FourCC >();
  const CAssetId id = in.Get< CAssetId >();
  return SObjectTag(type, id);
}
