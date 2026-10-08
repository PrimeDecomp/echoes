#include "MetroidPrime/ScriptLoaderRel.hpp"

SEyeBall_FuncPtrs* gLoader_EyeBall; // Guessed global name.

void SetSEyeBall_FuncPtrs(SEyeBall_FuncPtrs* callbacks) { gLoader_EyeBall = callbacks; }

// Guessed loader name.
CEntity* RelProxy_LoadEyeBall(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_EyeBall->mLoader(mgr, input, info);
}
