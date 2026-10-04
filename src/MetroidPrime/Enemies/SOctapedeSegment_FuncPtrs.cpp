#include "MetroidPrime/ScriptLoaderRel.hpp"

SOctapedeSegment_FuncPtrs* gLoader_OctapedeSegment; // Guessed global name.

void SetSOctapedeSegment_FuncPtrs(SOctapedeSegment_FuncPtrs* callbacks) {
  gLoader_OctapedeSegment = callbacks;
}

// Guessed loader name.
CEntity* LoadOctopedeSegment(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_OctapedeSegment->mLoader(mgr, input, info);
}
