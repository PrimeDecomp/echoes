#include "Kyoto/Particles/CParticleData.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CParticleData::CParticleData(CInputStream& in)
: mDuration(in.Get(TGetType(mDuration)))
, mParticle(in)
, mBone(in)
, mScale(in.Get(TGetType(mScale)))
, mParentMode(EParentedMode(in.Get< uint >())) {}
