#include "MetroidPrime/ScriptLoaderRel.hpp"

SScriptFogOverlay_FuncPtrs* gLoader_FogOverlay; // Guessed global name.

void SetSScriptFogOverlay_FuncPtrs(SScriptFogOverlay_FuncPtrs* callbacks) {
  gLoader_FogOverlay = callbacks;
}

// Guessed loader name.
CEntity* LoadFogOverlay(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_FogOverlay->mLoader(mgr, input, info);
}
