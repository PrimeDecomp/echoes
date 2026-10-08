#include "MetroidPrime/ScriptLoaderRel.hpp"

SLumite_FuncPtrs* gLoader_Lumite; // Guessed global name.

void SetSLumite_FuncPtrs(SLumite_FuncPtrs* callbacks) { gLoader_Lumite = callbacks; }

// Guessed loader name.
CEntity* RelProxy_LoadLumite(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Lumite->mLoader(mgr, input, info);
}
