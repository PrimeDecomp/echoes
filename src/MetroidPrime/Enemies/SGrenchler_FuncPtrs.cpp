#include "MetroidPrime/ScriptLoaderRel.hpp"

SGrenchler_FuncPtrs* gLoader_Grenchler; // Guessed global name.

void SetSGrenchler_FuncPtrs(SGrenchler_FuncPtrs* callbacks) { gLoader_Grenchler = callbacks; }

// Guessed loader name.
CEntity* LoadGrenchler(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Grenchler->mLoader(mgr, input, info);
}
