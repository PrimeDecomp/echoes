#include "MetroidPrime/ScriptLoaderRel.hpp"

SDarkTrooper_FuncPtrs* gLoader_DarkTrooper; // Guessed global name.

void SetSDarkTrooper_FuncPtrs(SDarkTrooper_FuncPtrs* callbacks) { gLoader_DarkTrooper = callbacks; }

// Guessed loader name.
CEntity* LoadDarkTrooper(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_DarkTrooper->mLoader(mgr, input, info);
}
