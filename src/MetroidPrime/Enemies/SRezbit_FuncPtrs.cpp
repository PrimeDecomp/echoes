#include "MetroidPrime/ScriptLoaderRel.hpp"

SRezbit_FuncPtrs* gLoader_Rezbit; // Guessed global name.

void SetSRezbit_FuncPtrs(SRezbit_FuncPtrs* callbacks) { gLoader_Rezbit = callbacks; }

// Guessed loader name.
CEntity* RelProxy_LoadRezbit(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Rezbit->mLoader(mgr, input, info);
}
