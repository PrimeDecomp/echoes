#include "MetroidPrime/ScriptLoaderRel.hpp"

SIng_FuncPtrs* gLoader_Ing; // Guessed global name.

void SetSIng_FuncPtrs(SIng_FuncPtrs* callbacks) { gLoader_Ing = callbacks; }

// Guessed loader name.
CEntity* LoadIngs(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Ing->mLoader(mgr, input, info);
}
