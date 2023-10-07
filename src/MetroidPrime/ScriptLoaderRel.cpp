#include "MetroidPrime/ScriptLoaderRel.hpp"

FScriptLoader* gLoader_IngSnatchingSwarm;
SSnakeWeedSwarm_FuncPtrs* gLoader_SnakeWeed;
SFishCloud_FuncPtrs* gLoader_FishCloud;
FScriptLoader* gLoader_AtomicAlpha;
FScriptLoader* gLoader_Ripper;
FScriptLoader* gLoader_Puffer;
FScriptLoader* gLoader_Metaree;
SPlayerActor_FuncPtrs* gLoader_PlayerActor;
SPlayerTurret_FuncPtrs* gLoader_PlayerTurret;
FScriptLoader* gLoader_RiftPortal;
SSafeZone_FuncPtrs* gLoader_SafeZone;
SGuiWidget_FuncPtrs* gLoader_GUI;
FScriptLoader* gLoader_PlayerController;
FScriptLoader* gLoader_WallWalker;

void SetLoader_WallWalker(FScriptLoader* loader) { gLoader_WallWalker = loader; }
CEntity* Load_WallWalker(CStateManager& mgr, CInputStream& input, const CEntityInfo& info) {
  return (*gLoader_WallWalker)(mgr, input, info);
}
void SetLoader_PlayerController(FScriptLoader* loader) { gLoader_PlayerController = loader; }
CEntity* LoadPlayerController(CStateManager& mgr, CInputStream& input, const CEntityInfo& info) {
  return (*gLoader_PlayerController)(mgr, input, info);
}
void SetSGuiWidget_FuncPtrs(SGuiWidget_FuncPtrs* loaders) { gLoader_GUI = loaders; }

CEntity* LoadGuiWidget(CStateManager& mgr, CInputStream& input, const CEntityInfo& info) {
  return gLoader_GUI->guiWidget(mgr, input, info);
}
CEntity* LoadGuiScreen(CStateManager& mgr, CInputStream& input, const CEntityInfo& info) {
  return gLoader_GUI->guiScreen(mgr, input, info);
}
CEntity* LoadGuiSlider(CStateManager& mgr, CInputStream& input, const CEntityInfo& info) {
  return gLoader_GUI->guiSlider(mgr, input, info);
}
CEntity* LoadGuiMenu(CStateManager& mgr, CInputStream& input, const CEntityInfo& info) {
  return gLoader_GUI->guiMenu(mgr, input, info);
}
CEntity* LoadGuiPlayerJoinManager(CStateManager& mgr, CInputStream& input,
                                  const CEntityInfo& info) {
  return gLoader_GUI->guiPlayerJoinManager(mgr, input, info);
}

void SetSSafeZone_FuncPtrs(SSafeZone_FuncPtrs* loader) { gLoader_SafeZone = loader; }
CEntity* LoadSafeZone(CStateManager& mgr, CInputStream& input, const CEntityInfo& info) {
  return gLoader_SafeZone->safeZone(mgr, input, info);
}
void SafeZone_ActOn(CEntity& entity, CStateManager& mgr) {
  (entity.*(gLoader_SafeZone->method))(mgr);
}

CEntity* LoadSafeZoneCrystal(CStateManager& mgr, CInputStream& input, const CEntityInfo& info) {
  return gLoader_SafeZone->safeZoneCrystal(mgr, input, info);
}
void SetLoader_RiftPortal(FScriptLoader* loader) { gLoader_RiftPortal = loader; }
CEntity* LoadRiftPortal(CStateManager& mgr, CInputStream& input, const CEntityInfo& info) {
  return (*gLoader_RiftPortal)(mgr, input, info);
}

void SetLoader_PlayerTurret(SPlayerTurret_FuncPtrs* loader) { gLoader_PlayerTurret = loader; }
CEntity* LoadPlayerTurret(CStateManager& mgr, CInputStream& input, const CEntityInfo& info) {
  return gLoader_PlayerTurret->loader(mgr, input, info);
}
CTransform4f PlayerTurret_GetTransform1(CEntity& entity, CStateManager& mgr) {
  return (entity.*(gLoader_PlayerTurret->GetTransform1))(mgr);
}

CTransform4f PlayerTurret_GetTransform2(CEntity& entity, CStateManager& mgr) {
  return (entity.*(gLoader_PlayerTurret->GetTransform2))(mgr);
}

void PlayerTurret_SendSomeMsg(CEntity& entity, CStateManager& mgr) {
  return (entity.*(gLoader_PlayerTurret->SendSomeMsg))(mgr);
}

void PlayerTurret_CheckInput(CEntity& entity, float dt, CFinalInput& input, CStateManager& mgr) {
  return (entity.*(gLoader_PlayerTurret->CheckInput))(dt, input, mgr);
}

TUniqueId PlayerTurret_GetSomeId(CEntity& entity) {
  return (entity.*(gLoader_PlayerTurret->GetSomeId))();
}

void SetLoader_PlayerActor(SPlayerActor_FuncPtrs* loader) { gLoader_PlayerActor = loader; }
CEntity* LoadPlayerActor(CStateManager& mgr, CInputStream& input, const CEntityInfo& info) {
  return gLoader_PlayerActor->loader(mgr, input, info);
}
void TouchPlayerActor(CEntity& ent, CStateManager& mgr) {
  return (ent.*(gLoader_PlayerActor->method))(mgr);
}

void SetLoader_Metaree(FScriptLoader* loader) { gLoader_Metaree = loader; }
CEntity* LoadMetaree(CStateManager& mgr, CInputStream& input, const CEntityInfo& info) {
  return (*gLoader_Metaree)(mgr, input, info);
}
void SetLoader_Puffer(FScriptLoader* loader) { gLoader_Puffer = loader; }
CEntity* LoadPuffer(CStateManager& mgr, CInputStream& input, const CEntityInfo& info) {
  return (*gLoader_Puffer)(mgr, input, info);
}
void SetLoader_Ripper(FScriptLoader* loader) { gLoader_Ripper = loader; }
CEntity* LoadRipper(CStateManager& mgr, CInputStream& input, const CEntityInfo& info) {
  return (*gLoader_Ripper)(mgr, input, info);
}
void SetLoader_AtomicAlpha(FScriptLoader* loader) { gLoader_AtomicAlpha = loader; }
CEntity* LoadAtomicAlpha(CStateManager& mgr, CInputStream& input, const CEntityInfo& info) {
  return (*gLoader_AtomicAlpha)(mgr, input, info);
}

void SetLoader_FishCloud(SFishCloud_FuncPtrs* loader) { gLoader_FishCloud = loader; }
CEntity* LoadFishCloud(CStateManager& mgr, CInputStream& input, const CEntityInfo& info) {
  return gLoader_FishCloud->fishCloud(mgr, input, info);
}
CEntity* LoadFishCloudModifier(CStateManager& mgr, CInputStream& input, const CEntityInfo& info) {
  return gLoader_FishCloud->fishCloudModifier(mgr, input, info);
}

void SetLoader_SnakeWeedSwarm(SSnakeWeedSwarm_FuncPtrs* loader) { gLoader_SnakeWeed = loader; }
CEntity* LoadSnakeWeedSwarm(CStateManager& mgr, CInputStream& input, const CEntityInfo& info) {
  return gLoader_SnakeWeed->swarm(mgr, input, info);
}

void SnakeWeedAlt_8021BA94(CEntity& ent, const CVector3f& v, const CDamageInfo& dmgInfo, CStateManager& mgr) {
  return (ent.*(gLoader_SnakeWeed->method))(v, dmgInfo, mgr);
}

void SetLoader_IngSnatchingSwarm(FScriptLoader* loader) { gLoader_IngSnatchingSwarm = loader; }
CEntity* LoadIngSnatchingSwarm(CStateManager& mgr, CInputStream& input, const CEntityInfo& info) {
  return (*gLoader_IngSnatchingSwarm)(mgr, input, info);
}
