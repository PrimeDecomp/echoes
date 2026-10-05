#include "Kyoto/Animation/CParticlePOINode.hpp"

CParticlePOINode::CParticlePOINode(CInputStream& in) : CPOINode(in), mData(in) {}

CParticlePOINode CParticlePOINode::CopyNodeMinusStartTime(const CParticlePOINode& node,
                                                          const CCharAnimTime& startTime) {
  return CParticlePOINode(node.GetNameHash(), node.GetPoiType(), node.GetTime() - startTime,
                          node.GetIndex(), node.GetSaveState(), node.GetWeight(),
                          node.GetCharacterIndex(), node.GetFlags(), node.GetParticleData());
}

float CParticlePOINode::GetMaximumDistance() const {
  if (GetFlags() & 0x8000000) {
    return 60.f;
  }
  if (GetFlags() & 0x4000000) {
    return 40.f;
  }
  if (GetFlags() & 0x2000000) {
    return 20.f;
  }
  return 3.4028235e38f;
}
