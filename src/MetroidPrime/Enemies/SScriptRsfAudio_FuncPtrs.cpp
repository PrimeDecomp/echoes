#include "MetroidPrime/ScriptLoaderRel.hpp"

SScriptRsfAudio_FuncPtrs* gLoader_ScriptRsfAudio; // Guessed global name.

void SetSScriptRsfAudio_FuncPtrs(SScriptRsfAudio_FuncPtrs* callbacks) {
  gLoader_ScriptRsfAudio = callbacks;
}

// Guessed loader name.
CEntity* RelProxy_LoadRsfAudio(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_ScriptRsfAudio->mLoader(mgr, input, info);
}
