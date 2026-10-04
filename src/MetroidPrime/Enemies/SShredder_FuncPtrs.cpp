#include "MetroidPrime/ScriptLoaderRel.hpp"

SShredder_FuncPtrs* gLoader_Shredder; // Guessed global name.

void SetSShredder_FuncPtrs(SShredder_FuncPtrs* callbacks) { gLoader_Shredder = callbacks; }

// Guessed loader name.
CEntity* LoadShredder(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Shredder->mLoader(mgr, input, info);
}
