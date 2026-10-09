#include "MetroidPrime/ScriptLoaderRel.hpp"

SShrieker_FuncPtrs* gLoader_Shrieker; // Guessed global name.

void SetSShrieker_FuncPtrs(SShrieker_FuncPtrs* callbacks) { gLoader_Shrieker = callbacks; }

// Guessed loader name.
CEntity* RelProxy_LoadShrieker(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Shrieker->mLoader(mgr, input, info);
}
