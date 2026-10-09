#include "MetroidPrime/Enemies/CSplitterMainChassis.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"

SSplitterMainChassis_FuncPtrs* gLoader_Splitter; // Guessed name.

void SetSSplitterMainChassis_FuncPtrs(SSplitterMainChassis_FuncPtrs* callbacks) {
  gLoader_Splitter = callbacks;
}

CEntity* RelProxy_LoadSplitterMainChassis(CStateManager& mgr, CInputStream& input,
                                          CEntityInfo& info) {
  return gLoader_Splitter->mLoadMainChassis(mgr, input, info);
}

CEntity* RelProxy_LoadSplitterCommandModule(CStateManager& mgr, CInputStream& input,
                                            CEntityInfo& info) {
  return gLoader_Splitter->mLoadCommandModule(mgr, input, info);
}

// A monolithic DOL links the class's own definition.
#ifndef MONOLITHIC
void CSplitterMainChassis::AutoDestruct(float time) {
  (this->*gLoader_Splitter->mAutoDestruct)(time);
}
#endif
