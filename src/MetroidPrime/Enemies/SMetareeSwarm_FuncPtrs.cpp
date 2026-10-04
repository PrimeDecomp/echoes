#include "MetroidPrime/ScriptLoaderRel.hpp"

SMetareeSwarm_FuncPtrs* gLoader_MetareeSwarm; // Guessed global name.

void SetSMetareeSwarm_FuncPtrs(SMetareeSwarm_FuncPtrs* callbacks) {
  gLoader_MetareeSwarm = callbacks;
}

// Guessed loader name.
CEntity* LoadMetareeSwarm(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_MetareeSwarm->mLoader(mgr, input, info);
}
