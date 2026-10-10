#include "Kyoto/Particles/CEffectComponent.hpp"

#include "Kyoto/Animation/CPOINode.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

CEffectComponent::CEffectComponent(CInputStream& in)
: mNameHash(CPOINode::GetHashForString(rstl::string(in).data()))
, mTag(GetSObjectTagFromStream(in))
, mBone(in)
, mScale(in.Get< float >())
, mParentedMode(CParticleData::EParentedMode(in.Get< uint >()))
, mFlags(in.Get< uint >()) {}

SObjectTag CEffectComponent::GetSObjectTagFromStream(CInputStream& in) {
    unsigned char* ptr = *(unsigned char**)((char*)&in + 0x8);
    in.ReadInt32();
    int val = *(int*)ptr;
    return SObjectTag(val, in.ReadInt32());
}
