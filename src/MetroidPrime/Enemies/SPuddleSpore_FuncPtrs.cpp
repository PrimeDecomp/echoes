#include "MetroidPrime/ScriptLoaderRel.hpp"

SPuddleSpore_FuncPtrs* gLoader_PuddleSpore; // Guessed global name.

void SetSPuddleSpore_FuncPtrs(SPuddleSpore_FuncPtrs* callbacks) { gLoader_PuddleSpore = callbacks; }

// Guessed loader name.
CEntity* RelProxy_LoadPuddleSpore(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_PuddleSpore->mLoader(mgr, input, info);
}
