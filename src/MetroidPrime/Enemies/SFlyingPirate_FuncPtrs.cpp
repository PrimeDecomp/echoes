#include "MetroidPrime/ScriptLoaderRel.hpp"

SFlyingPirate_FuncPtrs* gLoader_FlyingPirate; // Guessed global name.

void SetSFlyingPirate_FuncPtrs(SFlyingPirate_FuncPtrs* callbacks) {
  gLoader_FlyingPirate = callbacks;
}

// Guessed loader name.
CEntity* RelProxy_LoadFlyingPirate(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_FlyingPirate->mLoader(mgr, input, info);
}
