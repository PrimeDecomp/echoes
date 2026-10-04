#include "MetroidPrime/ScriptLoaderRel.hpp"

SMinorIng_FuncPtrs* gLoader_MinorIng; // Guessed global name.

void SetSMinorIng_FuncPtrs(SMinorIng_FuncPtrs* callbacks) { gLoader_MinorIng = callbacks; }

// Guessed loader name.
CEntity* LoadMinorIng(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_MinorIng->mLoader(mgr, input, info);
}
