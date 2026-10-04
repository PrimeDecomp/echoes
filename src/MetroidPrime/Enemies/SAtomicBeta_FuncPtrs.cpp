#include "MetroidPrime/ScriptLoaderRel.hpp"

SAtomicBeta_FuncPtrs* gLoader_AtomicBeta; // Guessed global name.

void SetSAtomicBeta_FuncPtrs(SAtomicBeta_FuncPtrs* callbacks) { gLoader_AtomicBeta = callbacks; }

// Guessed loader name.
CEntity* LoadAtomicBeta(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_AtomicBeta->mLoader(mgr, input, info);
}
