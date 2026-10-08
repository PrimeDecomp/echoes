#include "MetroidPrime/ScriptLoaderRel.hpp"

SCommandoPirate_FuncPtrs* gLoader_CommandoPirate; // Guessed global name.

void SetSCommandoPirate_FuncPtrs(SCommandoPirate_FuncPtrs* callbacks) {
  gLoader_CommandoPirate = callbacks;
}

// Guessed compatibility loader name.
CEntity* RelProxy_LoadCommandPirate(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_CommandoPirate->mLoader(mgr, input, info);
}
