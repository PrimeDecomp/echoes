#include "MetroidPrime/ScriptLoaderRel.hpp"

SElitePirate_FuncPtrs* gLoader_ElitePirate; // Guessed global name.

void SetSElitePirate_FuncPtrs(SElitePirate_FuncPtrs* callbacks) { gLoader_ElitePirate = callbacks; }

// Guessed loader name.
CEntity* LoadElitePirate(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_ElitePirate->mLoader(mgr, input, info);
}
