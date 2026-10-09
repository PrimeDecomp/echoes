#include "MetroidPrime/ScriptLoaderRel.hpp"

SScriptDarkSamusBattleStage_FuncPtrs* gLoader_DarkSamusBattleStage; // Guessed global name.

void SetSScriptDarkSamusBattleStage_FuncPtrs(SScriptDarkSamusBattleStage_FuncPtrs* callbacks) {
  gLoader_DarkSamusBattleStage = callbacks;
}

// Guessed loader name.
CEntity* RelProxy_LoadDarkSamusBattleStage(CStateManager& mgr, CInputStream& input,
                                           CEntityInfo& info) {
  return gLoader_DarkSamusBattleStage->mLoader(mgr, input, info);
}
