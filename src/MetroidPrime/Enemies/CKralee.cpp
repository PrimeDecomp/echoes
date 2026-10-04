#include "MetroidPrime/ScriptLoaderRel.hpp"

SKralee_FuncPtrs* gLoader_Kralee; // Guessed global name.

void SetSKralee_FuncPtrs(SKralee_FuncPtrs* callbacks) { gLoader_Kralee = callbacks; }

// Guessed loader name.
CEntity* LoadKralee(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Kralee->mLoadKralee(mgr, input, info);
}
