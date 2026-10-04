#include "MetroidPrime/ScriptLoaderRel.hpp"

SStreamedMovie_FuncPtrs* gLoader_ScriptStreamedMovie; // Guessed global name.

void SetSStreamedMovie_FuncPtrs(SStreamedMovie_FuncPtrs* callbacks) {
  gLoader_ScriptStreamedMovie = callbacks;
}

// Guessed loader name.
CEntity* LoadStreamedMovie(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_ScriptStreamedMovie->mLoader(mgr, input, info);
}
