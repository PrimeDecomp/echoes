#include "Kyoto/Animation/CAnimPOIData.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CAnimPOIData::CAnimPOIData(CInputStream& in)
: mVersion(in.Get< uint >()), mBoolNodes(in), mInt32Nodes(in), mParticleNodes(in) {
  if (mVersion > 1) {
    mSoundNodes = rstl::vector< CSoundPOINode >(in);
  }
}
