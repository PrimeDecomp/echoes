#include "MetroidPrime/ScriptLoaderRel.hpp"

SSporb_FuncPtrs* gLoader_Sporb; // Guessed global name.

void SetSSporb_FuncPtrs(SSporb_FuncPtrs* callbacks) { gLoader_Sporb = callbacks; }

// Guessed loader name.
CEntity* LoadSporbNeedle(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Sporb->mLoadNeedle(mgr, input, info);
}

// Guessed loader name.
CEntity* LoadSporbBase(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Sporb->mLoadBase(mgr, input, info);
}

// Guessed loader name.
CEntity* LoadSporbTop(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Sporb->mLoadTop(mgr, input, info);
}

// Guessed loader name.
CEntity* LoadSporbProjectile(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Sporb->mLoadProjectile(mgr, input, info);
}
