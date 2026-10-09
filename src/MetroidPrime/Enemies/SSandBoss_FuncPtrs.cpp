#include "MetroidPrime/ScriptLoaderRel.hpp"

SSandBoss_FuncPtrs* gLoader_SandBoss; // Guessed global name.

void SetSSandBoss_FuncPtrs(SSandBoss_FuncPtrs* callbacks) { gLoader_SandBoss = callbacks; }

// Guessed loader name.
CEntity* RelProxy_LoadSandBoss(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_SandBoss->mLoader(mgr, input, info);
}
