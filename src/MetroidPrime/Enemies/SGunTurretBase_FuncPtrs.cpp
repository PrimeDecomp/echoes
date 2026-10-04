#include "MetroidPrime/ScriptLoaderRel.hpp"

SGunTurretBase_FuncPtrs* gLoader_GunTurret; // Guessed global name.

void SetSGunTurretBase_FuncPtrs(SGunTurretBase_FuncPtrs* callbacks) {
  gLoader_GunTurret = callbacks;
}

// Guessed loader name.
CEntity* LoadGunTurretBase(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_GunTurret->mLoadBase(mgr, input, info);
}

// Guessed loader name.
CEntity* LoadGunTurretTop(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_GunTurret->mLoadTop(mgr, input, info);
}
