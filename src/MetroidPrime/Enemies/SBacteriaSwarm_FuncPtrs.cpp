#include "MetroidPrime/ScriptLoaderRel.hpp"

SBacteriaSwarm_FuncPtrs* gLoader_BacteriaSwarm; // Guessed global name.

void SetSBacteriaSwarm_FuncPtrs(SBacteriaSwarm_FuncPtrs* callbacks) {
  gLoader_BacteriaSwarm = callbacks;
}

// Guessed loader name.
CEntity* RelProxy_LoadBacteriaSwarm(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_BacteriaSwarm->mLoader(mgr, input, info);
}
