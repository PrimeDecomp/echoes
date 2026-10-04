#include "MetroidPrime/ScriptLoaderRel.hpp"

SMysteryFlyer_FuncPtrs* gLoader_MysteryFlyer; // Guessed global name.

void SetSMysteryFlyer_FuncPtrs(SMysteryFlyer_FuncPtrs* callbacks) {
  gLoader_MysteryFlyer = callbacks;
}

// Guessed loader name.
CEntity* LoadMysteryFlyer(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_MysteryFlyer->mLoader(mgr, input, info);
}
