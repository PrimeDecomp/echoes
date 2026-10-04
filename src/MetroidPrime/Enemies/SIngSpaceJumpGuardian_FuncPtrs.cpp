#include "MetroidPrime/ScriptLoaderRel.hpp"

SIngSpaceJumpGuardian_FuncPtrs* gLoader_IngSpaceJumpGuardian; // Guessed global name.

void SetSIngSpaceJumpGuardian_FuncPtrs(SIngSpaceJumpGuardian_FuncPtrs* callbacks) {
  gLoader_IngSpaceJumpGuardian = callbacks;
}

// Guessed loader name.
CEntity* LoadIngSpaceJumpGuardian(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_IngSpaceJumpGuardian->mLoader(mgr, input, info);
}
