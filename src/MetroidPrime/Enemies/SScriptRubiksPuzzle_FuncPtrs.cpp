#include "MetroidPrime/ScriptLoaderRel.hpp"

SScriptRubiksPuzzle_FuncPtrs* gLoader_RubiksPuzzle; // Guessed global name.

void SetSScriptRubiksPuzzle_FuncPtrs(SScriptRubiksPuzzle_FuncPtrs* callbacks) {
  gLoader_RubiksPuzzle = callbacks;
}

// Guessed loader name.
CEntity* LoadRubiksPuzzle(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_RubiksPuzzle->mLoader(mgr, input, info);
}
