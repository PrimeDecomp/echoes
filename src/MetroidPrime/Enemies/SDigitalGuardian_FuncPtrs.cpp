#include "MetroidPrime/ScriptLoaderRel.hpp"

SDigitalGuardian_FuncPtrs* gLoader_DigitalGuardian; // Guessed global name.

void SetSDigitalGuardian_FuncPtrs(SDigitalGuardian_FuncPtrs* callbacks) {
  gLoader_DigitalGuardian = callbacks;
}

// Guessed loader name.
CEntity* LoadDigitalGuardian(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_DigitalGuardian->mLoadDigitalGuardian(mgr, input, info);
}

// Guessed loader name.
CEntity* LoadDigitalGuardianHead(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_DigitalGuardian->mLoadDigitalGuardianHead(mgr, input, info);
}
