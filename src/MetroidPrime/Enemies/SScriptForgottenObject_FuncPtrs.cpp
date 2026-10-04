#include "MetroidPrime/ScriptLoaderRel.hpp"

SScriptForgottenObject_FuncPtrs* gLoader_ForgottenObject; // Guessed global name.

void SetSScriptForgottenObject_FuncPtrs(SScriptForgottenObject_FuncPtrs* callbacks) {
  gLoader_ForgottenObject = callbacks;
}

// Guessed loader name.
CEntity* LoadForgottenObject(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_ForgottenObject->loader(mgr, input, info);
}
