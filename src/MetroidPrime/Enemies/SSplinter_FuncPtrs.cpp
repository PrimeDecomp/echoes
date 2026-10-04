#include "MetroidPrime/ScriptLoaderRel.hpp"

SSplinter_FuncPtrs* gLoader_Splinter; // Guessed global name.

void SetSSplinter_FuncPtrs(SSplinter_FuncPtrs* callbacks) { gLoader_Splinter = callbacks; }

// Guessed loader name.
CEntity* LoadSplinter(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Splinter->mLoader(mgr, input, info);
}
