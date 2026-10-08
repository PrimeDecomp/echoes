#include "MetroidPrime/Enemies/CMetroid.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"

SMetroid_FuncPtrs* gLoader_Metroid; // Guessed name.

void SetSMetroid_FuncPtrs(SMetroid_FuncPtrs* callbacks) { gLoader_Metroid = callbacks; }

CEntity* RelProxy_LoadMetroidAlpha(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Metroid->mLoadMetroid(mgr, input, info);
}

// A monolithic DOL links the class's own definition.
#ifndef MONOLITHIC
void CMetroid::OnDockTouch(CStateManager& mgr) { (this->*gLoader_Metroid->mOnDockTouch)(mgr); }
#endif
