#include "MetroidPrime/ScriptLoaderRel.hpp"

SCannonBall_FuncPtrs* gLoader_CannonBall; // Guessed global name.

void SetSCannonBall_FuncPtrs(SCannonBall_FuncPtrs* callbacks) { gLoader_CannonBall = callbacks; }

CEntity* LoadCannonBall(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_CannonBall->mLoadCannonBall(mgr, input, info);
}
