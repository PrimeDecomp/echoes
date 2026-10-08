#include "MetroidPrime/ScriptLoaderRel.hpp"

SEmperorIngStage1_FuncPtrs* gLoader_EmperorIngStage1; // Guessed global name.

void SetSEmperorIngStage1_FuncPtrs(SEmperorIngStage1_FuncPtrs* callbacks) {
  gLoader_EmperorIngStage1 = callbacks;
}

// Guessed loader name.
CEntity* RelProxy_LoadEmperorIngStage1(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_EmperorIngStage1->mLoader(mgr, input, info);
}
