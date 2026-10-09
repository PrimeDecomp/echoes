#include "MetroidPrime/ScriptLoaderRel.hpp"

SParasite_FuncPtrs* gLoader_Parasite; // Guessed global name.

void SetSParasite_FuncPtrs(SParasite_FuncPtrs* callbacks) { gLoader_Parasite = callbacks; }

// Guessed loader name.
CEntity* RelProxy_LoadParasite(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Parasite->mLoadParasite(mgr, input, info);
}

// Guessed loader name.
CEntity* RelProxy_LoadBrizgee(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Parasite->mLoadBrizgee(mgr, input, info);
}

// Guessed loader name.
CEntity* RelProxy_LoadCrystallite(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Parasite->mLoadCrystallite(mgr, input, info);
}
