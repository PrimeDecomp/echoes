#include "MetroidPrime/ScriptLoaderRel.hpp"

SIngSpiderballGuardian_FuncPtrs* gLoader_IngSpiderballGuardian; // Guessed global name.

void SetSIngSpiderballGuardian_FuncPtrs(SIngSpiderballGuardian_FuncPtrs* callbacks) {
  gLoader_IngSpiderballGuardian = callbacks;
}

// Guessed loader name.
CEntity* RelProxy_LoadIngSpiderBallGuardian(CStateManager& mgr, CInputStream& input,
                                            CEntityInfo& info) {
  return gLoader_IngSpiderballGuardian->mLoader(mgr, input, info);
}
