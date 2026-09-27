#ifndef _CANIMPOIDATA
#define _CANIMPOIDATA

#include "Kyoto/Animation/CBoolPOINode.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"

#include "rstl/vector.hpp"

class CAnimPOIData {
public:
  explicit CAnimPOIData(CInputStream& in);

  const rstl::vector< CBoolPOINode >& GetBoolPOIStream() const { return mBoolNodes; }
  const rstl::vector< CInt32POINode >& GetInt32POIStream() const { return mInt32Nodes; }
  const rstl::vector< CParticlePOINode >& GetParticlePOIStream() const { return mParticleNodes; }
  const rstl::vector< CSoundPOINode >& GetSoundPOIStream() const { return mSoundNodes; }

private:
  uint mVersion;
  rstl::vector< CBoolPOINode > mBoolNodes;
  rstl::vector< CInt32POINode > mInt32Nodes;
  rstl::vector< CParticlePOINode > mParticleNodes;
  rstl::vector< CSoundPOINode > mSoundNodes;
};
CHECK_SIZEOF(CAnimPOIData, 0x44)

#endif // _CANIMPOIDATA
