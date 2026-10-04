#include "MetroidPrime/Enemies/CSplitterMainChassis.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"

SSplitterMainChassis_FuncPtrs* gLoader_Splitter; // Guessed name.

void SetSSplitterMainChassis_FuncPtrs(SSplitterMainChassis_FuncPtrs* callbacks) {
  gLoader_Splitter = callbacks;
}

CEntity* LoadSplitterMainChassis(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Splitter->mLoadMainChassis(mgr, input, info);
}

CEntity* LoadSplitterCommandModule(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Splitter->mLoadCommandModule(mgr, input, info);
}

void CSplitterMainChassis::AutoDestruct(float time) {
  (this->*gLoader_Splitter->mAutoDestruct)(time);
}
