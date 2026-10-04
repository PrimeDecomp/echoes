#include "MetroidPrime/ScriptLoaderRel.hpp"

SStoneToad_FuncPtrs* gLoader_StoneToad; // Guessed global name.

void SetSStoneToad_FuncPtrs(SStoneToad_FuncPtrs* callbacks) { gLoader_StoneToad = callbacks; }

// Guessed loader name.
CEntity* LoadStoneToad(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_StoneToad->mLoader(mgr, input, info);
}
