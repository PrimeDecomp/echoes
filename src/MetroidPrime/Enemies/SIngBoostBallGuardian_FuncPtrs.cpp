#include "MetroidPrime/ScriptLoaderRel.hpp"

SIngBoostBallGuardian_FuncPtrs* gLoader_IngBoostBallGuardian; // Guessed global name.

void SetSIngBoostBallGuardian_FuncPtrs(SIngBoostBallGuardian_FuncPtrs* callbacks) {
  gLoader_IngBoostBallGuardian = callbacks;
}

// Guessed loader name.
CEntity* RelProxy_LoadIngBoostBallGuardian(CStateManager& mgr, CInputStream& input,
                                           CEntityInfo& info) {
  return gLoader_IngBoostBallGuardian->mLoader(mgr, input, info);
}
