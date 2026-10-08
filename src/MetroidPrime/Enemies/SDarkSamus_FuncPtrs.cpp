#include "MetroidPrime/ScriptLoaderRel.hpp"

SDarkSamus_FuncPtrs* gLoader_DarkSamus; // Guessed global name.

void SetSDarkSamus_FuncPtrs(SDarkSamus_FuncPtrs* callbacks) { gLoader_DarkSamus = callbacks; }

// Guessed loader name.
CEntity* RelProxy_LoadDarkSamus(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_DarkSamus->mLoader(mgr, input, info);
}
