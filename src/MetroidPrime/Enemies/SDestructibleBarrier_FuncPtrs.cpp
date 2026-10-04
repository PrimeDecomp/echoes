#include "MetroidPrime/ScriptLoaderRel.hpp"

SDestructibleBarrier_FuncPtrs* gLoader_DestructibleBarrier; // Guessed global name.

void SetSDestructibleBarrier_FuncPtrs(SDestructibleBarrier_FuncPtrs* callbacks) {
  gLoader_DestructibleBarrier = callbacks;
}

// Guessed loader name.
CEntity* LoadDestructableBarrier(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_DestructibleBarrier->mLoader(mgr, input, info);
}
