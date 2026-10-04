#include "MetroidPrime/ScriptLoaderRel.hpp"

SBlogg_FuncPtrs* gLoader_Blogg; // Guessed global name.

void SetSBlogg_FuncPtrs(SBlogg_FuncPtrs* callbacks) { gLoader_Blogg = callbacks; }

// Guessed loader name.
CEntity* LoadBlogg(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Blogg->mLoader(mgr, input, info);
}
