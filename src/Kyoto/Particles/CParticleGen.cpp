#include "Kyoto/Particles/CParticleGen.hpp"

// Wii exports corroborate the member names; GameCube readers establish their roles.
uint CParticleGen::sDrawFlags;
uint CParticleGen::sDrawMask;

void CParticleGen::AddModifier(CWarp* warp) {
  mModifiersList.push_back(warp);
}
