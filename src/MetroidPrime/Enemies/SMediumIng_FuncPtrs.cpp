#include "MetroidPrime/ScriptLoaderRel.hpp"

SMediumIng_FuncPtrs* gLoader_MediumIng; // Guessed global name.

void SetSMediumIng_FuncPtrs(SMediumIng_FuncPtrs* callbacks) { gLoader_MediumIng = callbacks; }

// Guessed loader name.
CEntity* RelProxy_LoadMediumIng(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_MediumIng->mLoader(mgr, input, info);
}
