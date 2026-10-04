#include "MetroidPrime/ScriptLoaderRel.hpp"

SSwampBossStage1_FuncPtrs* gLoader_SwampBossStage1; // Guessed global name.

void SetSSwampBossStage1_FuncPtrs(SSwampBossStage1_FuncPtrs* callbacks) {
  gLoader_SwampBossStage1 = callbacks;
}

// Guessed loader name.
CEntity* LoadSwampBossStage1(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_SwampBossStage1->mLoader(mgr, input, info);
}
