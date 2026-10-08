#include "MetroidPrime/ScriptLoaderRel.hpp"

SChozoGhost_FuncPtrs* gLoader_ChozoGhost; // Guessed global name.

void SetSChozoGhost_FuncPtrs(SChozoGhost_FuncPtrs* callbacks) { gLoader_ChozoGhost = callbacks; }

// Guessed loader name.
CEntity* RelProxy_LoadChozoGhost(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_ChozoGhost->mLoader(mgr, input, info);
}
