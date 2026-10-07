#include "MetroidPrime/Enemies/CMetroid.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"

SMetroid_FuncPtrs* gLoader_Metroid; // Guessed name.

void SetSMetroid_FuncPtrs(SMetroid_FuncPtrs* callbacks) { gLoader_Metroid = callbacks; }

CEntity* LoadMetroidAlpha(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Metroid->mLoadMetroid(mgr, input, info);
}

void CMetroid::OnDockTouch(CStateManager& mgr) { (this->*gLoader_Metroid->mOnDockTouch)(mgr); }
