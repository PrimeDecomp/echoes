#include "MetroidPrime/ScriptLoaderRel.hpp"

SSwarmBasics_FuncPtrs* gFactory_SwarmBasics; // Guessed global name.

void SetSSwarmBasics_FuncPtrs(SSwarmBasics_FuncPtrs* callbacks) {
  gFactory_SwarmBasics = callbacks;
}
