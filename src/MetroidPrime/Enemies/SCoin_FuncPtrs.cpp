#include "MetroidPrime/ScriptLoaderRel.hpp"

SCoin_FuncPtrs* gLoader_Coin; // Guessed global name.

void SetSCoin_FuncPtrs(SCoin_FuncPtrs* callbacks) { gLoader_Coin = callbacks; }

// Guessed loader name.
CEntity* RelProxy_LoadCoin(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Coin->mLoader(mgr, input, info);
}
