#include "MetroidPrime/ScriptLoaderRel.hpp"

SDarkCommando_FuncPtrs* gLoader_DarkCommando; // Guessed global name.

void SetSDarkCommando_FuncPtrs(SDarkCommando_FuncPtrs* callbacks) {
  gLoader_DarkCommando = callbacks;
}

// Guessed loader name.
CEntity* LoadDarkCommando(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_DarkCommando->mLoader(mgr, input, info);
}
