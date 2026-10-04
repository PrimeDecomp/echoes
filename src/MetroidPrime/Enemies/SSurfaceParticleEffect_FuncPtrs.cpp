#include "MetroidPrime/ScriptLoaderRel.hpp"

SSurfaceParticleEffect_FuncPtrs* gFactory_SurfaceParticleEffect; // Guessed global name.

void SetSSurfaceParticleEffect_FuncPtrs(SSurfaceParticleEffect_FuncPtrs* callbacks) {
  gFactory_SurfaceParticleEffect = callbacks;
}
