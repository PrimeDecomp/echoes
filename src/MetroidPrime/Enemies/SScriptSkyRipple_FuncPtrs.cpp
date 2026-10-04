#include "MetroidPrime/ScriptLoaderRel.hpp"

SScriptSkyRipple_FuncPtrs* gLoader_SkyRipple; // Guessed global name.

void SetSScriptSkyRipple_FuncPtrs(SScriptSkyRipple_FuncPtrs* callbacks) {
  gLoader_SkyRipple = callbacks;
}

// Guessed loader name.
CEntity* LoadSkyRipple(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_SkyRipple->mLoader(mgr, input, info);
}
