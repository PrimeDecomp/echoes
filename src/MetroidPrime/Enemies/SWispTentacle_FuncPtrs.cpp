#include "MetroidPrime/ScriptLoaderRel.hpp"

SWispTentacle_FuncPtrs* gLoader_WispTentacle; // Guessed global name.

void SetSWispTentacle_FuncPtrs(SWispTentacle_FuncPtrs* callbacks) {
  gLoader_WispTentacle = callbacks;
}

// Guessed loader name.
CEntity* RelProxy_LoadWispTentacle(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_WispTentacle->mLoader(mgr, input, info);
}
