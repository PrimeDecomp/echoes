#include "MetroidPrime/ScriptLoaderRel.hpp"

STryclops_FuncPtrs* gLoader_Tryclops; // Guessed global name.

void SetSTryclops_FuncPtrs(STryclops_FuncPtrs* callbacks) { gLoader_Tryclops = callbacks; }

// Guessed loader name.
CEntity* RelProxy_LoadTryclops(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Tryclops->mLoader(mgr, input, info);
}
