#include "MetroidPrime/ScriptLoaderRel.hpp"

SIngBlobSwarm_FuncPtrs* gLoader_IngBlobSwarm; // Guessed global name.

void SetSIngBlobSwarm_FuncPtrs(SIngBlobSwarm_FuncPtrs* callbacks) {
  gLoader_IngBlobSwarm = callbacks;
}

// Guessed loader name.
CEntity* LoadIngBlobSwarm(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_IngBlobSwarm->mLoader(mgr, input, info);
}
