#ifndef _CPARTICLEPOINODE
#define _CPARTICLEPOINODE

#include "Kyoto/Animation/CPOINode.hpp"
#include "Kyoto/Particles/CParticleData.hpp"

class CParticlePOINode : public CPOINode {
public:
  CParticlePOINode(uint nameHash, ushort type, const CCharAnimTime& time, int index, bool unique,
                   float weight, int charIdx, int flags, const CParticleData& data)
  : CPOINode(nameHash, type, time, index, unique, weight, charIdx, flags), mData(data) {}

  explicit CParticlePOINode(CInputStream& in);

  const CParticleData& GetParticleData() const { return mData; }

  static CParticlePOINode CopyNodeMinusStartTime(const CParticlePOINode& node,
                                                 const CCharAnimTime& startTime);

private:
  CParticleData mData;
};
CHECK_SIZEOF(CParticlePOINode, 0x44)

#endif // _CPARTICLEPOINODE
