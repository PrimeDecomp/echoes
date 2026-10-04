#include "MetroidPrime/ScriptLoaderRel.hpp"

SPirateRagDoll_FuncPtrs* gFactory_PirateRagDoll; // Guessed global name.

void SetSPirateRagDoll_FuncPtrs(SPirateRagDoll_FuncPtrs* callbacks) {
  gFactory_PirateRagDoll = callbacks;
}
