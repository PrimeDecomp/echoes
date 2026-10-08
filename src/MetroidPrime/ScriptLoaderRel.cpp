#include "MetroidPrime/ScriptLoaderRel.hpp"

#include "MetroidPrime/ScriptObjects/CScriptSafeZone.hpp"

// Guessed global names; records remain borrowed from the currently loaded REL.
SIngSnatchingSwarm_FuncPtrs* gLoader_IngSnatchingSwarm;
SSnakeWeedSwarm_FuncPtrs* gLoader_SnakeWeed;
SFishCloud_FuncPtrs* gLoader_FishCloud;
SAtomicAlpha_FuncPtrs* gLoader_AtomicAlpha;
SRipper_FuncPtrs* gLoader_Ripper;
SPuffer_FuncPtrs* gLoader_Puffer;
SMetaree_FuncPtrs* gLoader_Metaree;
SPlayerActor_FuncPtrs* gLoader_PlayerActor;
SPlayerTurret_FuncPtrs* gLoader_PlayerTurret;
SRiftPortal_FuncPtrs* gLoader_RiftPortal;
SSafeZone_FuncPtrs* gLoader_SafeZone;
SGuiWidget_FuncPtrs* gLoader_GUI;
SPlayerController_FuncPtrs* gLoader_PlayerController;
SWallWalker_FuncPtrs* gLoader_WallWalker;

void SetSWallWalker_FuncPtrs(SWallWalker_FuncPtrs* callbacks) { gLoader_WallWalker = callbacks; }

CEntity* RelProxy_LoadWallWalker(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_WallWalker->mLoadWallWalker(mgr, input, info);
}

void SetSPlayerController_FuncPtrs(SPlayerController_FuncPtrs* callbacks) {
  gLoader_PlayerController = callbacks;
}

CEntity* RelProxy_LoadPlayerController(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_PlayerController->mLoadPlayerController(mgr, input, info);
}

void SetSGuiWidget_FuncPtrs(SGuiWidget_FuncPtrs* loaders) { gLoader_GUI = loaders; }

CEntity* RelProxy_LoadGuiWidget(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_GUI->mLoadGuiWidget(mgr, input, info);
}

CEntity* RelProxy_LoadGuiScreen(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_GUI->mLoadGuiScreen(mgr, input, info);
}

CEntity* RelProxy_LoadGuiSlider(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_GUI->mLoadGuiSlider(mgr, input, info);
}

CEntity* RelProxy_LoadGuiMenu(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_GUI->mLoadGuiMenu(mgr, input, info);
}

CEntity* RelProxy_LoadGuiPlayerJoinManager(CStateManager& mgr, CInputStream& input,
                                           CEntityInfo& info) {
  return gLoader_GUI->mLoadGuiPlayerJoinManager(mgr, input, info);
}

void SetSSafeZone_FuncPtrs(SSafeZone_FuncPtrs* loader) { gLoader_SafeZone = loader; }

CEntity* RelProxy_LoadSafeZone(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_SafeZone->mLoadSafeZone(mgr, input, info);
}

void SafeZone_ApplyRenderEffect(CEntity& entity, CStateManager& mgr) {
#ifdef MONOLITHIC
  static_cast< CScriptSafeZone& >(entity).ApplyRenderEffect(mgr);
#else
  (entity.*(gLoader_SafeZone->mApplyRenderEffect))(mgr);
#endif
}

CEntity* RelProxy_LoadSafeZoneCrystal(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_SafeZone->mLoadSafeZoneCrystal(mgr, input, info);
}

void SetSRiftPortal_FuncPtrs(SRiftPortal_FuncPtrs* callbacks) { gLoader_RiftPortal = callbacks; }

CEntity* RelProxy_LoadRiftPortal(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_RiftPortal->mLoadRiftPortal(mgr, input, info);
}

void SetSPlayerTurret_FuncPtrs(SPlayerTurret_FuncPtrs* callbacks) {
  gLoader_PlayerTurret = callbacks;
}

CEntity* RelProxy_LoadPlayerTurret(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_PlayerTurret->mLoadPlayerTurret(mgr, input, info);
}

CTransform4f PlayerTurret_GetCameraTransform(CEntity& entity, CStateManager& mgr) {
  return (entity.*(gLoader_PlayerTurret->mGetCameraTransform))(mgr);
}

CTransform4f PlayerTurret_GetTurretTransform(CEntity& entity, CStateManager& mgr) {
  return (entity.*(gLoader_PlayerTurret->mGetTurretTransform))(mgr);
}

void PlayerTurret_ExitTurret(CEntity& entity, CStateManager& mgr) {
  (entity.*(gLoader_PlayerTurret->mExitTurret))(mgr);
}

void PlayerTurret_ProcessInput(CEntity& entity, const CFinalInput& input, CStateManager& mgr) {
  (entity.*(gLoader_PlayerTurret->mProcessInput))(input, mgr);
}

TUniqueId PlayerTurret_GetHullActorId(CEntity& entity) {
  return (entity.*(gLoader_PlayerTurret->mGetHullActorId))();
}

void SetSPlayerActor_FuncPtrs(SPlayerActor_FuncPtrs* callbacks) { gLoader_PlayerActor = callbacks; }

CEntity* RelProxy_LoadPlayerActor(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_PlayerActor->mLoadPlayerActor(mgr, input, info);
}

void PlayerActor_TouchModels(CEntity& ent, CStateManager& mgr) {
  (ent.*(gLoader_PlayerActor->mTouchModels))(mgr);
}

void SetSMetaree_FuncPtrs(SMetaree_FuncPtrs* callbacks) { gLoader_Metaree = callbacks; }

CEntity* RelProxy_LoadMetaree(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Metaree->mLoadMetaree(mgr, input, info);
}

void SetSPuffer_FuncPtrs(SPuffer_FuncPtrs* callbacks) { gLoader_Puffer = callbacks; }

CEntity* RelProxy_LoadPuffer(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Puffer->mLoadPuffer(mgr, input, info);
}

void SetSRipper_FuncPtrs(SRipper_FuncPtrs* callbacks) { gLoader_Ripper = callbacks; }

CEntity* RelProxy_LoadRipper(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_Ripper->mLoadRipper(mgr, input, info);
}

void SetSAtomicAlpha_FuncPtrs(SAtomicAlpha_FuncPtrs* callbacks) { gLoader_AtomicAlpha = callbacks; }

CEntity* RelProxy_LoadAtomicAlpha(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_AtomicAlpha->mLoadAtomicAlpha(mgr, input, info);
}

void SetSFishCloud_FuncPtrs(SFishCloud_FuncPtrs* callbacks) { gLoader_FishCloud = callbacks; }

CEntity* RelProxy_LoadFishCloud(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_FishCloud->mLoadFishCloud(mgr, input, info);
}

CEntity* RelProxy_LoadFishCloudModifier(CStateManager& mgr, CInputStream& input,
                                        CEntityInfo& info) {
  return gLoader_FishCloud->mLoadFishCloudModifier(mgr, input, info);
}

void SetSSnakeWeedSwarm_FuncPtrs(SSnakeWeedSwarm_FuncPtrs* callbacks) {
  gLoader_SnakeWeed = callbacks;
}

CEntity* RelProxy_LoadSnakeWeedSwarm(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  return gLoader_SnakeWeed->mLoadSnakeWeedSwarm(mgr, input, info);
}

void SnakeWeed_ApplyRadiusDamage(CEntity& entity, CVector3f position, const CDamageInfo& damage,
                                 CStateManager& mgr) {
  (entity.*(gLoader_SnakeWeed->mApplyRadiusDamage))(position, damage, mgr);
}

void SetSIngSnatchingSwarm_FuncPtrs(SIngSnatchingSwarm_FuncPtrs* callbacks) {
  gLoader_IngSnatchingSwarm = callbacks;
}

CEntity* RelProxy_LoadIngSnatchingSwarm(CStateManager& mgr, CInputStream& input,
                                        CEntityInfo& info) {
  return gLoader_IngSnatchingSwarm->mLoadIngSnatchingSwarm(mgr, input, info);
}
