#include "MetroidPrime/ScriptLoaderRel.hpp"

SGlowbug_FuncPtrs* gLoader_Glowbug; // Guessed global name.

void SetSGlowbug_FuncPtrs(SGlowbug_FuncPtrs* callbacks) { gLoader_Glowbug = callbacks; }

// Guessed loader name.
CEntity* RelProxy_LoadGlowBug(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Glowbug->mLoader(mgr, input, info);
}
