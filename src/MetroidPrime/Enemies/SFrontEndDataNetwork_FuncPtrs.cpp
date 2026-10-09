#include "MetroidPrime/ScriptLoaderRel.hpp"

SFrontEndDataNetwork_FuncPtrs* gLoader_FrontEndDataNetwork; // Guessed global name.

void SetSFrontEndDataNetwork_FuncPtrs(SFrontEndDataNetwork_FuncPtrs* callbacks) {
  gLoader_FrontEndDataNetwork = callbacks;
}

// Guessed loader name.
CEntity* RelProxy_LoadFrontEndDataNetwork(CStateManager& mgr, CInputStream& input,
                                          CEntityInfo& info) {
  return gLoader_FrontEndDataNetwork->mLoader(mgr, input, info);
}
