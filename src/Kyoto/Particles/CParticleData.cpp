#include "Kyoto/Particles/CParticleData.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CParticleData::CParticleData(CInputStream& in)
: x0_duration(in.Get(TGetType(x0_duration)))
, x4_particle(in)
, xc_bone(in)
, x10_scale(in.Get(TGetType(x10_scale)))
, x14_parentMode(EParentedMode(in.Get< uint >())) {}
