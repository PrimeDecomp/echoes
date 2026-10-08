#include "MetroidPrime/ScriptLoaderRel.hpp"

SKrocuss_FuncPtrs* gLoader_Krocuss; // Guessed global name.

void SetSKrocuss_FuncPtrs(SKrocuss_FuncPtrs* callbacks) { gLoader_Krocuss = callbacks; }

// Guessed loader name.
CEntity* RelProxy_LoadKrocus(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Krocuss->mLoader(mgr, input, info);
}
