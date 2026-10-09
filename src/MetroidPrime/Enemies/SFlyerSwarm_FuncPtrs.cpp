#include "MetroidPrime/ScriptLoaderRel.hpp"

SFlyerSwarm_FuncPtrs* gLoader_FlyerSwarm; // Guessed global name.

void SetSFlyerSwarm_FuncPtrs(SFlyerSwarm_FuncPtrs* callbacks) { gLoader_FlyerSwarm = callbacks; }

// Guessed loader name.
CEntity* RelProxy_LoadFlyerSwarm(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_FlyerSwarm->mLoader(mgr, input, info);
}
