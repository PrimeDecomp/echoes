#include "MetroidPrime/ScriptLoaderRel.hpp"

SAIMannedTurret_FuncPtrs* gLoader_AIMannedTurret; // Guessed global name.

void SetSAIMannedTurret_FuncPtrs(SAIMannedTurret_FuncPtrs* callbacks) {
  gLoader_AIMannedTurret = callbacks;
}

// Guessed loader name.
CEntity* LoadAIMannedTurret(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_AIMannedTurret->mLoader(mgr, input, info);
}
