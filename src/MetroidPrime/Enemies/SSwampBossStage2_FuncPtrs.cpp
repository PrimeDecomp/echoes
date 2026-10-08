#include "MetroidPrime/ScriptLoaderRel.hpp"

SSwampBossStage2_FuncPtrs* gLoader_SwampBossStage2; // Guessed global name.

void SetSSwampBossStage2_FuncPtrs(SSwampBossStage2_FuncPtrs* callbacks) {
  gLoader_SwampBossStage2 = callbacks;
}

// Guessed loader name.
CEntity* RelProxy_LoadSwampBossStage2(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_SwampBossStage2->mLoader(mgr, input, info);
}
