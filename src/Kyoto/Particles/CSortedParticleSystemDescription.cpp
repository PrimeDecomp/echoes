#include "Kyoto/Particles/CSortedParticleSystemDescription.hpp"

#include "Kyoto/Particles/CSpawnSystemKeyframeData.hpp"

CSortedParticleSystemDescription::CSortedParticleSystemDescription() : mSPWN(nullptr) {}

CSortedParticleSystemDescription::~CSortedParticleSystemDescription() { delete mSPWN; }
