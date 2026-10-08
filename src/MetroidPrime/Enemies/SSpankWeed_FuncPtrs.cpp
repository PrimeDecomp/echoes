#include "MetroidPrime/ScriptLoaderRel.hpp"

SSpankWeed_FuncPtrs* gLoader_SpankWeed; // Guessed global name.

void SetSSpankWeed_FuncPtrs(SSpankWeed_FuncPtrs* callbacks) { gLoader_SpankWeed = callbacks; }

// Guessed loader name.
CEntity* RelProxy_LoadSpankWeed(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_SpankWeed->mLoader(mgr, input, info);
}
