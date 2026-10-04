#include "MetroidPrime/ScriptLoaderRel.hpp"

SPlantScarabSwarm_FuncPtrs* gLoader_PlantScarabSwarm; // Guessed global name.

void SetSPlantScarabSwarm_FuncPtrs(SPlantScarabSwarm_FuncPtrs* callbacks) {
  gLoader_PlantScarabSwarm = callbacks;
}

// Guessed loader name.
CEntity* LoadPlantScarabSwarm(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_PlantScarabSwarm->mLoader(mgr, input, info);
}
