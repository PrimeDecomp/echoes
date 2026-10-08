#include "MetroidPrime/ScriptLoaderRel.hpp"

SGunTurretBase_FuncPtrs* gLoader_GunTurret; // Guessed global name.

void SetSGunTurretBase_FuncPtrs(SGunTurretBase_FuncPtrs* callbacks) {
  gLoader_GunTurret = callbacks;
}

// Guessed loader name.
CEntity* RelProxy_LoadGunTurretBase(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_GunTurret->mLoadBase(mgr, input, info);
}

// Guessed loader name.
CEntity* RelProxy_LoadGunTurretTop(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_GunTurret->mLoadTop(mgr, input, info);
}
