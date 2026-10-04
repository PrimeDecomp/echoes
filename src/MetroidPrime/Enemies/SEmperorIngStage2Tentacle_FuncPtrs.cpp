#include "MetroidPrime/ScriptLoaderRel.hpp"

SEmperorIngStage2Tentacle_FuncPtrs* gLoader_EmperorIngStage2Tentacle; // Guessed global name.

void SetSEmperorIngStage2Tentacle_FuncPtrs(SEmperorIngStage2Tentacle_FuncPtrs* callbacks) {
  gLoader_EmperorIngStage2Tentacle = callbacks;
}

// Guessed loader name.
CEntity* LoadEmperorIngStage2Tentacle(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_EmperorIngStage2Tentacle->mLoader(mgr, input, info);
}
