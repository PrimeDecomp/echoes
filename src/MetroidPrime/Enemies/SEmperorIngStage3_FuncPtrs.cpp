#include "MetroidPrime/ScriptLoaderRel.hpp"

SEmperorIngStage3_FuncPtrs* gLoader_EmperorIngStage3; // Guessed global name.

void SetSEmperorIngStage3_FuncPtrs(SEmperorIngStage3_FuncPtrs* callbacks) {
  gLoader_EmperorIngStage3 = callbacks;
}

// Guessed loader name.
CEntity* RelProxy_LoadEmperorIngStage3(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_EmperorIngStage3->mLoader(mgr, input, info);
}
