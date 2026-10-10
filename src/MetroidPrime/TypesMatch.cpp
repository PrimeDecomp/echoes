#include "Collision/CCollidableAABox.hpp"
#include "Collision/CCollidableSphere.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CEffect.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGameHint.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Cameras/CCinematicCamera.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Cameras/CFixedCamera.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Cameras/CPathCamera.hpp"
#include "MetroidPrime/Cameras/CSpindleCamera.hpp"
#include "MetroidPrime/Cameras/CSurfaceCamera.hpp"
#include "MetroidPrime/Enemies/CAIMannedTurret.hpp"
#include "MetroidPrime/Enemies/CAi.hpp"
#include "MetroidPrime/Enemies/CBabyMetroid.hpp"
#include "MetroidPrime/Enemies/CBacteriaSwarm.hpp"
#include "MetroidPrime/Enemies/CBouncyGrenade.hpp"
#include "MetroidPrime/Enemies/CDarkSamusBattleStage.hpp"
#include "MetroidPrime/Enemies/CFlyerSwarm.hpp"
#include "MetroidPrime/Enemies/CGlowbug.hpp"
#include "MetroidPrime/Enemies/CGrenchler.hpp"
#include "MetroidPrime/Enemies/CGunTurretBase.hpp"
#include "MetroidPrime/Enemies/CGunTurretTop.hpp"
#include "MetroidPrime/Enemies/CIngBlobSwarm.hpp"
#include "MetroidPrime/Enemies/CIngSnatchingSwarm.hpp"
#include "MetroidPrime/Enemies/CIngPuddle.hpp"
#include "MetroidPrime/Enemies/CIngSpaceJumpGuardian.hpp"
#include "MetroidPrime/Enemies/CIngSpiderballGuardian.hpp"
#include "MetroidPrime/Enemies/CBlogg.hpp"
#include "MetroidPrime/Enemies/CKralee.hpp"
#include "MetroidPrime/Enemies/CKrocuss.hpp"
#include "MetroidPrime/Enemies/CLumite.hpp"
#include "MetroidPrime/Enemies/CMetaree.hpp"
#include "MetroidPrime/Enemies/CMetareeSwarm.hpp"
#include "MetroidPrime/Enemies/CMetroid.hpp"
#include "MetroidPrime/Enemies/COctapedeSegment.hpp"
#include "MetroidPrime/Enemies/CParasite.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CPillBug.hpp"
#include "MetroidPrime/Enemies/CPlantScarabSwarm.hpp"
#include "MetroidPrime/Enemies/CPuddleSpore.hpp"
#include "MetroidPrime/Enemies/CPuffer.hpp"
#include "MetroidPrime/Enemies/CRezbit.hpp"
#include "MetroidPrime/Enemies/CRipper.hpp"
#include "MetroidPrime/Enemies/CSandworm.hpp"
#include "MetroidPrime/Enemies/CShredder.hpp"
#include "MetroidPrime/Enemies/CSnakeWeedSwarm.hpp"
#include "MetroidPrime/Enemies/CSpacePirate.hpp"
#include "MetroidPrime/Enemies/CSpankWeed.hpp"
#include "MetroidPrime/Enemies/CSporbBase.hpp"
#include "MetroidPrime/Enemies/CSporbNeedle.hpp"
#include "MetroidPrime/Enemies/CSporbProjectile.hpp"
#include "MetroidPrime/Enemies/CSporbTop.hpp"
#include "MetroidPrime/Enemies/CStoneToad.hpp"
#include "MetroidPrime/Enemies/CSwarmBasics.hpp"
#include "MetroidPrime/Enemies/CWallCrawler.hpp"
#include "MetroidPrime/Enemies/CWallWalker.hpp"
#include "MetroidPrime/Enemies/CWispTentacle.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CFishCloud.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActorKeyframe.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActorRotate.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAiJumpPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraPitch.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraShaker.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptColorModulate.hpp"
#include "MetroidPrime/ScriptObjects/CScriptControlHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCounter.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDamageableTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDamageableTriggerOrientated.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDebris.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDestructibleBarrier.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDistanceFog.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDock.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDoor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDynamicLight.hpp"
#include "MetroidPrime/ScriptObjects/CScriptEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptForgottenObject.hpp"
#include "MetroidPrime/ScriptObjects/CScriptFrontEndDataNetwork.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGrapplePoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGuiMenu.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGuiScreen.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGuiSlider.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGuiWidget.hpp"
#include "MetroidPrime/ScriptObjects/CScriptHUDHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptLayerController.hpp"
#include "MetroidPrime/ScriptObjects/CScriptMemoryRelay.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPathCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPathMeshCtrl.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPickup.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPickupGenerator.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlayerHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlayerTurret.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPointOfInterest.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPortalTransition.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlayerProxy.hpp"
#include "MetroidPrime/ScriptObjects/CScriptRelay.hpp"
#include "MetroidPrime/ScriptObjects/CScriptRepulsor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptRiftPortal.hpp"
#include "MetroidPrime/ScriptObjects/CScriptRoomAcoustics.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSafeZone.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSequenceTimer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSound.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSoundModifier.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpawnPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpecialFunction.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpiderBallAttractionSurface.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpiderBallWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpindleCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptStreamedMusic.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSurfaceCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSwitch.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTargetingPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTextPane.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTimeKeyframe.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTimer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTriggerEllipsoid.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTriggerOrientated.hpp"
#include "MetroidPrime/ScriptObjects/CScriptVisorFlare.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWorldTeleporter.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CBeamProjectile.hpp"
#include "MetroidPrime/Weapons/CBomb.hpp"
#include "MetroidPrime/Weapons/CBouncingBomb.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CHomingBlob.hpp"
#include "MetroidPrime/Weapons/CLightComboProjectile.hpp"
#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"
#include "MetroidPrime/Weapons/CPowerBomb.hpp"
#include "MetroidPrime/Weapons/CTargetableProjectile.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "WorldFormat/CCollidableOBBTreeGroup.hpp"
#include "WorldFormat/COBBTreeGroup.hpp"

#define TYPES_MATCH_IMPL(cls, parent, id)                                                          \
  CEntity* cls::TypesMatch(int typeId) const {                                                     \
    if (typeId == id) {                                                                            \
      return const_cast< cls* >(this);                                                             \
    }                                                                                              \
    if (typeId > id) {                                                                             \
      return nullptr;                                                                              \
    }                                                                                              \
    return parent::TypesMatch(typeId);                                                             \
  }

#define CAST_TO_PTR_IMPL(cls, id)                                                                  \
  template <>                                                                                      \
  cls* TCastToPtr< cls >(CEntity * entity) {                                                       \
    return static_cast< cls* >(TryCast(entity, id));                                               \
  }

#define CAST_TO_REF_IMPL(cls, id)                                                                  \
  template <>                                                                                      \
  cls* TCastToPtr< cls >(CEntity & entity) {                                                       \
    return static_cast< cls* >(entity.TypesMatch(id));                                             \
  }

CCollisionActor::~CCollisionActor() {}

CEnergyProjectile::~CEnergyProjectile() {}

CScriptAiJumpPoint::~CScriptAiJumpPoint() {}

CScriptCoverPoint::~CScriptCoverPoint() {}

CScriptDamageableTriggerOrientated::~CScriptDamageableTriggerOrientated() {}

CScriptHUDHint::~CScriptHUDHint() {}

CScriptPortalTransition::~CScriptPortalTransition() {}

CScriptRoomAcoustics::~CScriptRoomAcoustics() {}

CScriptTargetingPoint::~CScriptTargetingPoint() {}

CEntity* TryCast(CEntity* entity, int typeId) {
  if (entity != nullptr) {
    return entity->TypesMatch(typeId);
  }
  return nullptr;
}

CEntity* CEntity::TypesMatch(int typeId) const {
  return typeId == kET_Entity ? const_cast< CEntity* >(this) : nullptr;
}

TYPES_MATCH_IMPL(CActor, CEntity, kET_Actor)
TYPES_MATCH_IMPL(CPhysicsActor, CActor, kET_PhysicsActor)
TYPES_MATCH_IMPL(CAi, CPhysicsActor, kET_Ai)
TYPES_MATCH_IMPL(CPatterned, CAi, kET_Patterned)
TYPES_MATCH_IMPL(CGameCamera, CActor, kET_GameCamera)
TYPES_MATCH_IMPL(CWeapon, CActor, kET_Weapon)
TYPES_MATCH_IMPL(CEffect, CActor, kET_Effect)
TYPES_MATCH_IMPL(CGameProjectile, CWeapon, kET_GameProjectile)
TYPES_MATCH_IMPL(CScriptWaypoint, CActor, kET_ScriptWaypoint)
TYPES_MATCH_IMPL(CScriptPathMeshCtrl, CActor, kET_ScriptPathMeshCtrl)
TYPES_MATCH_IMPL(CScriptGuiWidget, CEntity, kET_ScriptGuiWidget)
TYPES_MATCH_IMPL(CScriptSequenceTimer, CEntity, kET_ScriptSequenceTimer)
TYPES_MATCH_IMPL(CBallCamera, CGameCamera, kET_BallCamera)
TYPES_MATCH_IMPL(CBomb, CWeapon, kET_Bomb)
TYPES_MATCH_IMPL(CBouncingBomb, CWeapon, kET_BouncingBomb)
TYPES_MATCH_IMPL(CBouncyGrenade, CPhysicsActor, kET_BouncyGrenade)
TYPES_MATCH_IMPL(CCinematicCamera, CGameCamera, kET_CinematicCamera)
TYPES_MATCH_IMPL(CCollisionActor, CPhysicsActor, kET_CollisionActor)
TYPES_MATCH_IMPL(CEnergyProjectile, CGameProjectile, kET_EnergyProjectile)
TYPES_MATCH_IMPL(CLightComboProjectile, CEnergyProjectile, kET_LightComboProjectile)
// 21: class not declared yet (DarkSamus REL); parent CWeapon
TYPES_MATCH_IMPL(CExplosion, CEffect, kET_Explosion)
TYPES_MATCH_IMPL(CFirstPersonCamera, CGameCamera, kET_FirstPersonCamera)
TYPES_MATCH_IMPL(CFixedCamera, CGameCamera, kET_FixedCamera)
TYPES_MATCH_IMPL(CFishCloud, CActor, kET_FishCloud)
TYPES_MATCH_IMPL(CGameLight, CActor, kET_GameLight)
TYPES_MATCH_IMPL(CHomingBlob, CWeapon, kET_HomingBlob)
TYPES_MATCH_IMPL(CHUDBillboardEffect, CEffect, kET_HUDBillboardEffect)
TYPES_MATCH_IMPL(CIngPuddle, CPhysicsActor, kET_IngPuddle)
TYPES_MATCH_IMPL(CIngSnatchingSwarm, CActor, kET_IngSnatchingSwarm)
TYPES_MATCH_IMPL(CPathCamera, CGameCamera, kET_PathCamera)
TYPES_MATCH_IMPL(CPlayer, CPhysicsActor, kET_Player)
TYPES_MATCH_IMPL(CGameHint, CActor, kET_GameHint)
TYPES_MATCH_IMPL(CScriptActor, CPhysicsActor, kET_ScriptActor)
TYPES_MATCH_IMPL(CScriptActorKeyframe, CEntity, kET_ScriptActorKeyframe)
TYPES_MATCH_IMPL(CScriptActorRotate, CEntity, kET_ScriptActorRotate)
TYPES_MATCH_IMPL(CScriptAIHint, CActor, kET_ScriptAIHint)
TYPES_MATCH_IMPL(CScriptAiJumpPoint, CActor, kET_ScriptAiJumpPoint)
TYPES_MATCH_IMPL(CScriptAIWaypoint, CScriptWaypoint, kET_ScriptAIWaypoint)
TYPES_MATCH_IMPL(CScriptCameraHint, CGameHint, kET_ScriptCameraHint)
TYPES_MATCH_IMPL(CScriptCameraShaker, CEntity, kET_ScriptCameraShaker)
TYPES_MATCH_IMPL(CScriptCameraPitch, CActor, kET_ScriptCameraPitch)
TYPES_MATCH_IMPL(CScriptCameraWaypoint, CScriptWaypoint, kET_ScriptCameraWaypoint)
TYPES_MATCH_IMPL(CScriptCamera, CActor, kET_ScriptCamera)
TYPES_MATCH_IMPL(CScriptColorModulate, CEntity, kET_ScriptColorModulate)
TYPES_MATCH_IMPL(CScriptControlHint, CGameHint, kET_ScriptControlHint)
TYPES_MATCH_IMPL(CScriptCounter, CEntity, kET_ScriptCounter)
TYPES_MATCH_IMPL(CScriptCoverPoint, CActor, kET_ScriptCoverPoint)
TYPES_MATCH_IMPL(CScriptDamageableTrigger, CActor, kET_ScriptDamageableTrigger)
TYPES_MATCH_IMPL(CScriptDamageableTriggerOrientated, CScriptDamageableTrigger,
                 kET_ScriptDamageableTriggerOrientated)
TYPES_MATCH_IMPL(CDarkSamusBattleStage, CEntity, kET_DarkSamusBattleStage)
TYPES_MATCH_IMPL(CScriptDebris, CPhysicsActor, kET_ScriptDebris)
TYPES_MATCH_IMPL(CScriptDestructibleBarrier, CPhysicsActor, kET_ScriptDestructibleBarrier)
TYPES_MATCH_IMPL(CScriptDistanceFog, CEntity, kET_ScriptDistanceFog)
TYPES_MATCH_IMPL(CScriptDock, CPhysicsActor, kET_ScriptDock)
TYPES_MATCH_IMPL(CScriptDoor, CPhysicsActor, kET_ScriptDoor)
TYPES_MATCH_IMPL(CScriptDynamicLight, CGameLight, kET_ScriptDynamicLight)
TYPES_MATCH_IMPL(CScriptEffect, CActor, kET_ScriptEffect)
TYPES_MATCH_IMPL(CScriptGrapplePoint, CActor, kET_ScriptGrapplePoint)
TYPES_MATCH_IMPL(CScriptGuiMenu, CScriptGuiWidget, kET_ScriptGuiMenu)
TYPES_MATCH_IMPL(CScriptGuiScreen, CActor, kET_ScriptGuiScreen)
TYPES_MATCH_IMPL(CScriptGuiSlider, CScriptGuiWidget, kET_ScriptGuiSlider)
TYPES_MATCH_IMPL(CScriptHUDHint, CActor, kET_ScriptHUDHint)
TYPES_MATCH_IMPL(CScriptLayerController, CEntity, kET_ScriptLayerController)
TYPES_MATCH_IMPL(CScriptPathCamera, CEntity, kET_ScriptPathCamera)
TYPES_MATCH_IMPL(CScriptPickup, CActor, kET_ScriptPickup)
TYPES_MATCH_IMPL(CScriptPickupGenerator, CEntity, kET_ScriptPickupGenerator)
TYPES_MATCH_IMPL(CScriptPlayerHint, CGameHint, kET_ScriptPlayerHint)
TYPES_MATCH_IMPL(CScriptPlayerProxy, CActor, kET_ScriptPlayerProxy)
TYPES_MATCH_IMPL(CScriptPlatform, CPhysicsActor, kET_ScriptPlatform)
TYPES_MATCH_IMPL(CScriptPointOfInterest, CActor, kET_ScriptPointOfInterest)
TYPES_MATCH_IMPL(CScriptPortalTransition, CEntity, kET_ScriptPortalTransition)
TYPES_MATCH_IMPL(CScriptRelay, CEntity, kET_ScriptRelay)
TYPES_MATCH_IMPL(CScriptRepulsor, CActor, kET_ScriptRepulsor)
TYPES_MATCH_IMPL(CScriptRiftPortal, CActor, kET_ScriptRiftPortal)
TYPES_MATCH_IMPL(CScriptRoomAcoustics, CEntity, kET_ScriptRoomAcoustics)
TYPES_MATCH_IMPL(CScriptSound, CActor, kET_ScriptSound)
TYPES_MATCH_IMPL(CScriptSoundModifier, CEntity, kET_ScriptSoundModifier)
TYPES_MATCH_IMPL(CScriptSpawnPoint, CEntity, kET_ScriptSpawnPoint)
TYPES_MATCH_IMPL(CScriptSpecialFunction, CActor, kET_ScriptSpecialFunction)
TYPES_MATCH_IMPL(CScriptSpiderBallAttractionSurface, CActor, kET_ScriptSpiderBallAttractionSurface)
TYPES_MATCH_IMPL(CScriptSpiderBallWaypoint, CScriptWaypoint, kET_ScriptSpiderBallWaypoint)
TYPES_MATCH_IMPL(CScriptSpindleCamera, CActor, kET_ScriptSpindleCamera)
TYPES_MATCH_IMPL(CScriptStreamedMusic, CEntity, kET_ScriptStreamedMusic)
TYPES_MATCH_IMPL(CScriptSurfaceCamera, CActor, kET_ScriptSurfaceCamera)
TYPES_MATCH_IMPL(CScriptSwitch, CEntity, kET_ScriptSwitch)
TYPES_MATCH_IMPL(CScriptTargetingPoint, CActor, kET_ScriptTargetingPoint)
TYPES_MATCH_IMPL(CScriptTeamAiMgr, CEntity, kET_ScriptTeamAi)
TYPES_MATCH_IMPL(CScriptTextPane, CActor, kET_ScriptTextPane)
TYPES_MATCH_IMPL(CScriptTimeKeyframe, CEntity, kET_ScriptTimeKeyframe)
TYPES_MATCH_IMPL(CScriptTimer, CEntity, kET_ScriptTimer)
TYPES_MATCH_IMPL(CScriptTrigger, CActor, kET_ScriptTrigger)
TYPES_MATCH_IMPL(CScriptTriggerEllipsoid, CScriptTrigger, kET_ScriptTriggerEllipsoid)
TYPES_MATCH_IMPL(CScriptTriggerOrientated, CScriptTrigger, kET_ScriptTriggerOrientated)
TYPES_MATCH_IMPL(CScriptSafeZone, CScriptTriggerEllipsoid, kET_ScriptSafeZone)
TYPES_MATCH_IMPL(CScriptVisorFlare, CActor, kET_ScriptVisorFlare)
TYPES_MATCH_IMPL(CScriptWater, CScriptTrigger, kET_ScriptWater)
TYPES_MATCH_IMPL(CScriptWorldTeleporter, CEntity, kET_ScriptWorldTeleporter)
TYPES_MATCH_IMPL(CSnakeWeedSwarm, CActor, kET_SnakeWeedSwarm)
TYPES_MATCH_IMPL(CSpindleCamera, CGameCamera, kET_SpindleCamera)
TYPES_MATCH_IMPL(CSurfaceCamera, CGameCamera, kET_SurfaceCamera)
TYPES_MATCH_IMPL(CSwarmBasics, CActor, kET_SwarmBasics)
TYPES_MATCH_IMPL(CFlyerSwarm, CSwarmBasics, kET_FlyerSwarm)
TYPES_MATCH_IMPL(CWallCrawler, CPatterned, kET_WallCrawler)
TYPES_MATCH_IMPL(CBacteriaSwarm, CActor, kET_BacteriaSwarm)
TYPES_MATCH_IMPL(CMetareeSwarm, CSwarmBasics, kET_MetareeSwarm)
TYPES_MATCH_IMPL(CIngBlobSwarm, CSwarmBasics, kET_IngBlobSwarm)
TYPES_MATCH_IMPL(CPlantScarabSwarm, CSwarmBasics, kET_PlantScarabSwarm)
TYPES_MATCH_IMPL(CBeamProjectile, CGameProjectile, kET_BeamProjectile)
TYPES_MATCH_IMPL(CPlasmaProjectile, CBeamProjectile, kET_PlasmaProjectile)
// kET_DarkSamus (111): class not declared yet (DarkSamus REL); parent CPatterned
// 112: class not declared yet (DigitalGuardian REL); parent CPatterned
// 113: class not declared yet (DigitalGuardian REL); parent CPatterned
// 114: class not declared yet (ElitePirate REL); parent CPatterned
TYPES_MATCH_IMPL(CGrenchler, CPatterned, kET_Grenchler)
// 116: class not declared yet (Ing REL); parent CPatterned
// 117: class not declared yet (IngBoostBallGuardian REL); parent CPatterned
TYPES_MATCH_IMPL(CIngSpaceJumpGuardian, CPatterned, kET_IngSpaceJumpGuardian)
TYPES_MATCH_IMPL(CIngSpiderballGuardian, CPatterned, kET_IngSpiderballGuardian)
TYPES_MATCH_IMPL(CLumite, CPatterned, kET_Lumite)
TYPES_MATCH_IMPL(CMetaree, CPatterned, kET_Metaree)
TYPES_MATCH_IMPL(CMetroid, CPatterned, kET_Metroid)
TYPES_MATCH_IMPL(CBabyMetroid, CMetroid, kET_BabyMetroid)
TYPES_MATCH_IMPL(CParasite, CWallCrawler, kET_Parasite)
TYPES_MATCH_IMPL(CPillBug, CWallCrawler, kET_PillBug)
TYPES_MATCH_IMPL(CPuffer, CPatterned, kET_Puffer)
TYPES_MATCH_IMPL(CRezbit, CPatterned, kET_Rezbit)
TYPES_MATCH_IMPL(CRipper, CPatterned, kET_Ripper)
// 129: class not declared yet (SandBoss REL); parent CPatterned
TYPES_MATCH_IMPL(CSandworm, CPatterned, kET_Sandworm)
TYPES_MATCH_IMPL(CSandwormEye, CActor, kET_SandwormEye)
TYPES_MATCH_IMPL(CSpacePirate, CPatterned, kET_SpacePirate)
TYPES_MATCH_IMPL(CSpankWeed, CPatterned, kET_SpankWeed)
// 134: class not declared yet (Splitter REL); parent CPatterned
// 135: class not declared yet (Splitter REL); parent CPatterned
TYPES_MATCH_IMPL(CWispTentacle, CPatterned, kET_WispTentacle)
// 137: class not declared yet (no vtable found); parent CActor
TYPES_MATCH_IMPL(CScriptPlayerTurret, CActor, kET_ScriptPlayerTurret)
TYPES_MATCH_IMPL(CGunTurretBase, CPatterned, kET_GunTurretBase)
TYPES_MATCH_IMPL(CGunTurretTop, CPatterned, kET_GunTurretTop)
TYPES_MATCH_IMPL(CKralee, CWallCrawler, kET_Kralee)
TYPES_MATCH_IMPL(CGlowbug, CPatterned, kET_Glowbug)
TYPES_MATCH_IMPL(CSporbBase, CPatterned, kET_SporbBase)
TYPES_MATCH_IMPL(CSporbNeedle, CPhysicsActor, kET_SporbNeedle)
TYPES_MATCH_IMPL(CSporbTop, CPatterned, kET_SporbTop)
TYPES_MATCH_IMPL(CSporbProjectile, CPatterned, kET_SporbProjectile)
// 147: class not declared yet (MinorIng REL); parent CPatterned
// 148: class not declared yet (IngBoostBallGuardian REL); parent CPhysicsActor
TYPES_MATCH_IMPL(CBlogg, CPatterned, kET_Blogg)
TYPES_MATCH_IMPL(CWallWalker, CWallCrawler, kET_WallWalker)
TYPES_MATCH_IMPL(CShredder, CPatterned, kET_Shredder)
TYPES_MATCH_IMPL(CTargetableProjectile, CEnergyProjectile, kET_TargetableProjectile)
TYPES_MATCH_IMPL(CAIMannedTurret, CAi, kET_AIMannedTurret)
TYPES_MATCH_IMPL(CStoneToad, CPatterned, kET_StoneToad)
TYPES_MATCH_IMPL(CScriptFrontEndDataNetwork, CActor, kET_ScriptFrontEndDataNetwork)
TYPES_MATCH_IMPL(CPowerBomb, CWeapon, kET_PowerBomb)
TYPES_MATCH_IMPL(CKrocuss, CPatterned, kET_Krocuss)
TYPES_MATCH_IMPL(COctapedeSegment, CWallCrawler, kET_OctapedeSegment)
TYPES_MATCH_IMPL(CPuddleSpore, CPatterned, kET_PuddleSpore)
TYPES_MATCH_IMPL(CScriptForgottenObject, CEntity, kET_ScriptForgottenObject)

CAST_TO_REF_IMPL(CEntity, kET_Entity)
CAST_TO_PTR_IMPL(CEntity, kET_Entity)
CAST_TO_REF_IMPL(CGameCamera, kET_GameCamera)
CAST_TO_PTR_IMPL(CGameCamera, kET_GameCamera)
CAST_TO_REF_IMPL(CWeapon, kET_Weapon)
CAST_TO_PTR_IMPL(CWeapon, kET_Weapon)
CAST_TO_REF_IMPL(CEffect, kET_Effect)
CAST_TO_PTR_IMPL(CEffect, kET_Effect)
CAST_TO_REF_IMPL(CGameProjectile, kET_GameProjectile)
CAST_TO_PTR_IMPL(CGameProjectile, kET_GameProjectile)
CAST_TO_REF_IMPL(CScriptWaypoint, kET_ScriptWaypoint)
CAST_TO_PTR_IMPL(CScriptWaypoint, kET_ScriptWaypoint)
CAST_TO_REF_IMPL(CScriptPathMeshCtrl, kET_ScriptPathMeshCtrl)
CAST_TO_PTR_IMPL(CScriptPathMeshCtrl, kET_ScriptPathMeshCtrl)
CAST_TO_REF_IMPL(CScriptGuiWidget, kET_ScriptGuiWidget)
CAST_TO_PTR_IMPL(CScriptGuiWidget, kET_ScriptGuiWidget)
CAST_TO_REF_IMPL(CScriptSequenceTimer, kET_ScriptSequenceTimer)
CAST_TO_PTR_IMPL(CScriptSequenceTimer, kET_ScriptSequenceTimer)
CAST_TO_REF_IMPL(CBallCamera, kET_BallCamera)
CAST_TO_PTR_IMPL(CBallCamera, kET_BallCamera)
CAST_TO_REF_IMPL(CBomb, kET_Bomb)
CAST_TO_PTR_IMPL(CBomb, kET_Bomb)
CAST_TO_REF_IMPL(CBouncingBomb, kET_BouncingBomb)
CAST_TO_PTR_IMPL(CBouncingBomb, kET_BouncingBomb)
CAST_TO_REF_IMPL(CBouncyGrenade, kET_BouncyGrenade)
CAST_TO_PTR_IMPL(CBouncyGrenade, kET_BouncyGrenade)
CAST_TO_REF_IMPL(CCinematicCamera, kET_CinematicCamera)
CAST_TO_PTR_IMPL(CCinematicCamera, kET_CinematicCamera)
CAST_TO_REF_IMPL(CCollisionActor, kET_CollisionActor)
CAST_TO_PTR_IMPL(CCollisionActor, kET_CollisionActor)
CAST_TO_REF_IMPL(CEnergyProjectile, kET_EnergyProjectile)
CAST_TO_PTR_IMPL(CEnergyProjectile, kET_EnergyProjectile)
CAST_TO_REF_IMPL(CLightComboProjectile, kET_LightComboProjectile)
CAST_TO_PTR_IMPL(CLightComboProjectile, kET_LightComboProjectile)
// 21: class not declared yet (DarkSamus REL)
CAST_TO_REF_IMPL(CExplosion, kET_Explosion)
CAST_TO_PTR_IMPL(CExplosion, kET_Explosion)

const CGameCamera* CCameraManager::CastGameCameratoFirstPersonCamera(const CGameCamera* camera) {
  return static_cast< const CFirstPersonCamera* >(camera->TypesMatch(kET_FirstPersonCamera));
}

CAST_TO_PTR_IMPL(CFirstPersonCamera, kET_FirstPersonCamera)
CAST_TO_REF_IMPL(CFixedCamera, kET_FixedCamera)
CAST_TO_PTR_IMPL(CFixedCamera, kET_FixedCamera)
CAST_TO_REF_IMPL(CFishCloud, kET_FishCloud)
CAST_TO_PTR_IMPL(CFishCloud, kET_FishCloud)
CAST_TO_REF_IMPL(CGameLight, kET_GameLight)
CAST_TO_PTR_IMPL(CGameLight, kET_GameLight)
CAST_TO_REF_IMPL(CHomingBlob, kET_HomingBlob)
CAST_TO_PTR_IMPL(CHomingBlob, kET_HomingBlob)
CAST_TO_REF_IMPL(CHUDBillboardEffect, kET_HUDBillboardEffect)
CAST_TO_PTR_IMPL(CHUDBillboardEffect, kET_HUDBillboardEffect)
// 29: class not declared yet (IngPuddle REL)
CAST_TO_REF_IMPL(CIngSnatchingSwarm, kET_IngSnatchingSwarm)
CAST_TO_PTR_IMPL(CIngSnatchingSwarm, kET_IngSnatchingSwarm)
CAST_TO_REF_IMPL(CPathCamera, kET_PathCamera)
CAST_TO_PTR_IMPL(CPathCamera, kET_PathCamera)
CAST_TO_REF_IMPL(CPlayer, kET_Player)
CAST_TO_PTR_IMPL(CPlayer, kET_Player)
CAST_TO_REF_IMPL(CGameHint, kET_GameHint)
CAST_TO_PTR_IMPL(CGameHint, kET_GameHint)
CAST_TO_REF_IMPL(CScriptActor, kET_ScriptActor)
CAST_TO_PTR_IMPL(CScriptActor, kET_ScriptActor)
CAST_TO_REF_IMPL(CScriptActorKeyframe, kET_ScriptActorKeyframe)
CAST_TO_PTR_IMPL(CScriptActorKeyframe, kET_ScriptActorKeyframe)
CAST_TO_REF_IMPL(CScriptActorRotate, kET_ScriptActorRotate)
CAST_TO_PTR_IMPL(CScriptActorRotate, kET_ScriptActorRotate)
CAST_TO_REF_IMPL(CScriptAIHint, kET_ScriptAIHint)
CAST_TO_PTR_IMPL(CScriptAIHint, kET_ScriptAIHint)
CAST_TO_REF_IMPL(CScriptAiJumpPoint, kET_ScriptAiJumpPoint)
CAST_TO_PTR_IMPL(CScriptAiJumpPoint, kET_ScriptAiJumpPoint)
CAST_TO_REF_IMPL(CScriptAIWaypoint, kET_ScriptAIWaypoint)
CAST_TO_PTR_IMPL(CScriptAIWaypoint, kET_ScriptAIWaypoint)
CAST_TO_REF_IMPL(CScriptCameraHint, kET_ScriptCameraHint)
CAST_TO_PTR_IMPL(CScriptCameraHint, kET_ScriptCameraHint)
CAST_TO_REF_IMPL(CScriptCameraShaker, kET_ScriptCameraShaker)
CAST_TO_PTR_IMPL(CScriptCameraShaker, kET_ScriptCameraShaker)
CAST_TO_REF_IMPL(CScriptCameraPitch, kET_ScriptCameraPitch)
CAST_TO_PTR_IMPL(CScriptCameraPitch, kET_ScriptCameraPitch)
CAST_TO_REF_IMPL(CScriptCameraWaypoint, kET_ScriptCameraWaypoint)
CAST_TO_PTR_IMPL(CScriptCameraWaypoint, kET_ScriptCameraWaypoint)
CAST_TO_REF_IMPL(CScriptCamera, kET_ScriptCamera)
CAST_TO_PTR_IMPL(CScriptCamera, kET_ScriptCamera)
CAST_TO_REF_IMPL(CScriptColorModulate, kET_ScriptColorModulate)
CAST_TO_PTR_IMPL(CScriptColorModulate, kET_ScriptColorModulate)
CAST_TO_REF_IMPL(CScriptControlHint, kET_ScriptControlHint)
CAST_TO_PTR_IMPL(CScriptControlHint, kET_ScriptControlHint)
CAST_TO_REF_IMPL(CScriptCounter, kET_ScriptCounter)
CAST_TO_PTR_IMPL(CScriptCounter, kET_ScriptCounter)
CAST_TO_REF_IMPL(CScriptCoverPoint, kET_ScriptCoverPoint)
CAST_TO_PTR_IMPL(CScriptCoverPoint, kET_ScriptCoverPoint)
CAST_TO_REF_IMPL(CScriptDamageableTrigger, kET_ScriptDamageableTrigger)
CAST_TO_PTR_IMPL(CScriptDamageableTrigger, kET_ScriptDamageableTrigger)
CAST_TO_REF_IMPL(CScriptDamageableTriggerOrientated, kET_ScriptDamageableTriggerOrientated)
CAST_TO_PTR_IMPL(CScriptDamageableTriggerOrientated, kET_ScriptDamageableTriggerOrientated)
CAST_TO_REF_IMPL(CDarkSamusBattleStage, kET_DarkSamusBattleStage)
CAST_TO_PTR_IMPL(CDarkSamusBattleStage, kET_DarkSamusBattleStage)
CAST_TO_REF_IMPL(CScriptDebris, kET_ScriptDebris)
CAST_TO_PTR_IMPL(CScriptDebris, kET_ScriptDebris)
CAST_TO_REF_IMPL(CScriptDestructibleBarrier, kET_ScriptDestructibleBarrier)
CAST_TO_PTR_IMPL(CScriptDestructibleBarrier, kET_ScriptDestructibleBarrier)
CAST_TO_REF_IMPL(CScriptDistanceFog, kET_ScriptDistanceFog)
CAST_TO_PTR_IMPL(CScriptDistanceFog, kET_ScriptDistanceFog)
CAST_TO_REF_IMPL(CScriptDock, kET_ScriptDock)
CAST_TO_PTR_IMPL(CScriptDock, kET_ScriptDock)
CAST_TO_REF_IMPL(CScriptDoor, kET_ScriptDoor)
CAST_TO_PTR_IMPL(CScriptDoor, kET_ScriptDoor)
CAST_TO_REF_IMPL(CScriptDynamicLight, kET_ScriptDynamicLight)
CAST_TO_PTR_IMPL(CScriptDynamicLight, kET_ScriptDynamicLight)
CAST_TO_REF_IMPL(CScriptEffect, kET_ScriptEffect)
CAST_TO_PTR_IMPL(CScriptEffect, kET_ScriptEffect)
CAST_TO_REF_IMPL(CScriptGrapplePoint, kET_ScriptGrapplePoint)
CAST_TO_PTR_IMPL(CScriptGrapplePoint, kET_ScriptGrapplePoint)
CAST_TO_REF_IMPL(CScriptGuiMenu, kET_ScriptGuiMenu)
CAST_TO_PTR_IMPL(CScriptGuiMenu, kET_ScriptGuiMenu)
CAST_TO_REF_IMPL(CScriptGuiScreen, kET_ScriptGuiScreen)
CAST_TO_PTR_IMPL(CScriptGuiScreen, kET_ScriptGuiScreen)
CAST_TO_REF_IMPL(CScriptGuiSlider, kET_ScriptGuiSlider)
CAST_TO_PTR_IMPL(CScriptGuiSlider, kET_ScriptGuiSlider)
CAST_TO_REF_IMPL(CScriptHUDHint, kET_ScriptHUDHint)
CAST_TO_PTR_IMPL(CScriptHUDHint, kET_ScriptHUDHint)
CAST_TO_REF_IMPL(CScriptLayerController, kET_ScriptLayerController)
CAST_TO_PTR_IMPL(CScriptLayerController, kET_ScriptLayerController)
CAST_TO_REF_IMPL(CScriptPathCamera, kET_ScriptPathCamera)
CAST_TO_PTR_IMPL(CScriptPathCamera, kET_ScriptPathCamera)
CAST_TO_REF_IMPL(CScriptPickup, kET_ScriptPickup)
CAST_TO_PTR_IMPL(CScriptPickup, kET_ScriptPickup)
CAST_TO_REF_IMPL(CScriptPickupGenerator, kET_ScriptPickupGenerator)
CAST_TO_PTR_IMPL(CScriptPickupGenerator, kET_ScriptPickupGenerator)
CAST_TO_REF_IMPL(CScriptPlayerHint, kET_ScriptPlayerHint)
CAST_TO_PTR_IMPL(CScriptPlayerHint, kET_ScriptPlayerHint)
CAST_TO_REF_IMPL(CScriptPlayerProxy, kET_ScriptPlayerProxy)
CAST_TO_PTR_IMPL(CScriptPlayerProxy, kET_ScriptPlayerProxy)
CAST_TO_REF_IMPL(CScriptPlatform, kET_ScriptPlatform)
CAST_TO_PTR_IMPL(CScriptPlatform, kET_ScriptPlatform)
CAST_TO_REF_IMPL(CScriptPointOfInterest, kET_ScriptPointOfInterest)
CAST_TO_PTR_IMPL(CScriptPointOfInterest, kET_ScriptPointOfInterest)
CAST_TO_REF_IMPL(CScriptPortalTransition, kET_ScriptPortalTransition)
CAST_TO_PTR_IMPL(CScriptPortalTransition, kET_ScriptPortalTransition)
CAST_TO_REF_IMPL(CScriptRelay, kET_ScriptRelay)
CAST_TO_PTR_IMPL(CScriptRelay, kET_ScriptRelay)
CAST_TO_REF_IMPL(CScriptRepulsor, kET_ScriptRepulsor)
CAST_TO_PTR_IMPL(CScriptRepulsor, kET_ScriptRepulsor)
// 75: class not declared yet (ScriptRiftPortal REL)
CAST_TO_REF_IMPL(CScriptRoomAcoustics, kET_ScriptRoomAcoustics)
CAST_TO_PTR_IMPL(CScriptRoomAcoustics, kET_ScriptRoomAcoustics)
CAST_TO_REF_IMPL(CScriptSound, kET_ScriptSound)
CAST_TO_PTR_IMPL(CScriptSound, kET_ScriptSound)
CAST_TO_REF_IMPL(CScriptSoundModifier, kET_ScriptSoundModifier)
CAST_TO_PTR_IMPL(CScriptSoundModifier, kET_ScriptSoundModifier)
CAST_TO_REF_IMPL(CScriptSpawnPoint, kET_ScriptSpawnPoint)
CAST_TO_PTR_IMPL(CScriptSpawnPoint, kET_ScriptSpawnPoint)
CAST_TO_REF_IMPL(CScriptSpecialFunction, kET_ScriptSpecialFunction)
CAST_TO_PTR_IMPL(CScriptSpecialFunction, kET_ScriptSpecialFunction)
CAST_TO_REF_IMPL(CScriptSpiderBallAttractionSurface, kET_ScriptSpiderBallAttractionSurface)
CAST_TO_PTR_IMPL(CScriptSpiderBallAttractionSurface, kET_ScriptSpiderBallAttractionSurface)
CAST_TO_REF_IMPL(CScriptSpiderBallWaypoint, kET_ScriptSpiderBallWaypoint)
CAST_TO_PTR_IMPL(CScriptSpiderBallWaypoint, kET_ScriptSpiderBallWaypoint)
CAST_TO_REF_IMPL(CScriptSpindleCamera, kET_ScriptSpindleCamera)
CAST_TO_PTR_IMPL(CScriptSpindleCamera, kET_ScriptSpindleCamera)
CAST_TO_REF_IMPL(CScriptStreamedMusic, kET_ScriptStreamedMusic)
CAST_TO_PTR_IMPL(CScriptStreamedMusic, kET_ScriptStreamedMusic)
CAST_TO_REF_IMPL(CScriptSurfaceCamera, kET_ScriptSurfaceCamera)
CAST_TO_PTR_IMPL(CScriptSurfaceCamera, kET_ScriptSurfaceCamera)
CAST_TO_REF_IMPL(CScriptSwitch, kET_ScriptSwitch)
CAST_TO_PTR_IMPL(CScriptSwitch, kET_ScriptSwitch)
CAST_TO_REF_IMPL(CScriptTargetingPoint, kET_ScriptTargetingPoint)
CAST_TO_PTR_IMPL(CScriptTargetingPoint, kET_ScriptTargetingPoint)
CAST_TO_REF_IMPL(CScriptTeamAiMgr, kET_ScriptTeamAi)
CAST_TO_PTR_IMPL(CScriptTeamAiMgr, kET_ScriptTeamAi)
CAST_TO_REF_IMPL(CScriptTextPane, kET_ScriptTextPane)
CAST_TO_PTR_IMPL(CScriptTextPane, kET_ScriptTextPane)
CAST_TO_REF_IMPL(CScriptTimeKeyframe, kET_ScriptTimeKeyframe)
CAST_TO_PTR_IMPL(CScriptTimeKeyframe, kET_ScriptTimeKeyframe)
CAST_TO_REF_IMPL(CScriptTimer, kET_ScriptTimer)
CAST_TO_PTR_IMPL(CScriptTimer, kET_ScriptTimer)
CAST_TO_REF_IMPL(CScriptTrigger, kET_ScriptTrigger)
CAST_TO_PTR_IMPL(CScriptTrigger, kET_ScriptTrigger)
CAST_TO_REF_IMPL(CScriptTriggerEllipsoid, kET_ScriptTriggerEllipsoid)
CAST_TO_PTR_IMPL(CScriptTriggerEllipsoid, kET_ScriptTriggerEllipsoid)
CAST_TO_REF_IMPL(CScriptTriggerOrientated, kET_ScriptTriggerOrientated)
CAST_TO_PTR_IMPL(CScriptTriggerOrientated, kET_ScriptTriggerOrientated)
CAST_TO_REF_IMPL(CScriptSafeZone, kET_ScriptSafeZone)
CAST_TO_PTR_IMPL(CScriptSafeZone, kET_ScriptSafeZone)
CAST_TO_REF_IMPL(CScriptVisorFlare, kET_ScriptVisorFlare)
CAST_TO_PTR_IMPL(CScriptVisorFlare, kET_ScriptVisorFlare)
CAST_TO_REF_IMPL(CScriptWater, kET_ScriptWater)
CAST_TO_PTR_IMPL(CScriptWater, kET_ScriptWater)
CAST_TO_REF_IMPL(CScriptWorldTeleporter, kET_ScriptWorldTeleporter)
CAST_TO_PTR_IMPL(CScriptWorldTeleporter, kET_ScriptWorldTeleporter)
CAST_TO_REF_IMPL(CSnakeWeedSwarm, kET_SnakeWeedSwarm)
CAST_TO_PTR_IMPL(CSnakeWeedSwarm, kET_SnakeWeedSwarm)
CAST_TO_REF_IMPL(CSpindleCamera, kET_SpindleCamera)
CAST_TO_PTR_IMPL(CSpindleCamera, kET_SpindleCamera)
CAST_TO_REF_IMPL(CSurfaceCamera, kET_SurfaceCamera)
CAST_TO_PTR_IMPL(CSurfaceCamera, kET_SurfaceCamera)
CAST_TO_REF_IMPL(CSwarmBasics, kET_SwarmBasics)
CAST_TO_PTR_IMPL(CSwarmBasics, kET_SwarmBasics)
CAST_TO_REF_IMPL(CFlyerSwarm, kET_FlyerSwarm)
CAST_TO_PTR_IMPL(CFlyerSwarm, kET_FlyerSwarm)
CAST_TO_REF_IMPL(CWallCrawler, kET_WallCrawler)
CAST_TO_PTR_IMPL(CWallCrawler, kET_WallCrawler)
CAST_TO_REF_IMPL(CBacteriaSwarm, kET_BacteriaSwarm)
CAST_TO_PTR_IMPL(CBacteriaSwarm, kET_BacteriaSwarm)
// 107: class not declared yet (IngBlobSwarm REL)
// 108: class not declared yet (PlantScarabSwarm REL)
CAST_TO_REF_IMPL(CBeamProjectile, kET_BeamProjectile)
CAST_TO_PTR_IMPL(CBeamProjectile, kET_BeamProjectile)
CAST_TO_REF_IMPL(CPlasmaProjectile, kET_PlasmaProjectile)
CAST_TO_PTR_IMPL(CPlasmaProjectile, kET_PlasmaProjectile)
// kET_DarkSamus (111): class not declared yet (DarkSamus REL)
// 112: class not declared yet (DigitalGuardian REL)
// 113: class not declared yet (DigitalGuardian REL)
// 114: class not declared yet (ElitePirate REL)
// 115: class not declared yet (Grenchler REL)
// 116: class not declared yet (Ing REL)
// 117: class not declared yet (IngBoostBallGuardian REL)
CAST_TO_REF_IMPL(CIngSpaceJumpGuardian, kET_IngSpaceJumpGuardian)
CAST_TO_PTR_IMPL(CIngSpaceJumpGuardian, kET_IngSpaceJumpGuardian)
CAST_TO_REF_IMPL(CIngSpiderballGuardian, kET_IngSpiderballGuardian)
CAST_TO_PTR_IMPL(CIngSpiderballGuardian, kET_IngSpiderballGuardian)
CAST_TO_REF_IMPL(CLumite, kET_Lumite)
CAST_TO_PTR_IMPL(CLumite, kET_Lumite)
CAST_TO_REF_IMPL(CMetaree, kET_Metaree)
CAST_TO_PTR_IMPL(CMetaree, kET_Metaree)
CAST_TO_REF_IMPL(CMetroid, kET_Metroid)
CAST_TO_PTR_IMPL(CMetroid, kET_Metroid)
CAST_TO_REF_IMPL(CBabyMetroid, kET_BabyMetroid)
CAST_TO_PTR_IMPL(CBabyMetroid, kET_BabyMetroid)
CAST_TO_REF_IMPL(CParasite, kET_Parasite)
CAST_TO_PTR_IMPL(CParasite, kET_Parasite)
CAST_TO_REF_IMPL(CPillBug, kET_PillBug)
CAST_TO_PTR_IMPL(CPillBug, kET_PillBug)
CAST_TO_REF_IMPL(CPuffer, kET_Puffer)
CAST_TO_PTR_IMPL(CPuffer, kET_Puffer)
CAST_TO_REF_IMPL(CRezbit, kET_Rezbit)
CAST_TO_PTR_IMPL(CRezbit, kET_Rezbit)
CAST_TO_REF_IMPL(CRipper, kET_Ripper)
CAST_TO_PTR_IMPL(CRipper, kET_Ripper)
// 129: class not declared yet (SandBoss REL)
CAST_TO_REF_IMPL(CSandworm, kET_Sandworm)
CAST_TO_PTR_IMPL(CSandworm, kET_Sandworm)
CAST_TO_REF_IMPL(CSandwormEye, kET_SandwormEye)
CAST_TO_PTR_IMPL(CSandwormEye, kET_SandwormEye)
CAST_TO_REF_IMPL(CSpacePirate, kET_SpacePirate)
CAST_TO_PTR_IMPL(CSpacePirate, kET_SpacePirate)
CAST_TO_REF_IMPL(CSpankWeed, kET_SpankWeed)
CAST_TO_PTR_IMPL(CSpankWeed, kET_SpankWeed)
// 134: class not declared yet (Splitter REL)
// 135: class not declared yet (Splitter REL)
// 136: class not declared yet (WispTentacle REL)
// 137: class not declared yet (no vtable found)
CAST_TO_REF_IMPL(CScriptPlayerTurret, kET_ScriptPlayerTurret)
CAST_TO_PTR_IMPL(CScriptPlayerTurret, kET_ScriptPlayerTurret)
CAST_TO_REF_IMPL(CGunTurretBase, kET_GunTurretBase)
CAST_TO_PTR_IMPL(CGunTurretBase, kET_GunTurretBase)
CAST_TO_REF_IMPL(CGunTurretTop, kET_GunTurretTop)
CAST_TO_PTR_IMPL(CGunTurretTop, kET_GunTurretTop)
// 141: class not declared yet (Kralee REL)
// 142: class not declared yet (Glowbug REL)
CAST_TO_REF_IMPL(CSporbBase, kET_SporbBase)
CAST_TO_PTR_IMPL(CSporbBase, kET_SporbBase)
CAST_TO_REF_IMPL(CSporbNeedle, kET_SporbNeedle)
CAST_TO_PTR_IMPL(CSporbNeedle, kET_SporbNeedle)
CAST_TO_REF_IMPL(CSporbTop, kET_SporbTop)
CAST_TO_PTR_IMPL(CSporbTop, kET_SporbTop)
CAST_TO_REF_IMPL(CSporbProjectile, kET_SporbProjectile)
CAST_TO_PTR_IMPL(CSporbProjectile, kET_SporbProjectile)
// 147: class not declared yet (MinorIng REL)
// 148: class not declared yet (IngBoostBallGuardian REL)
CAST_TO_REF_IMPL(CBlogg, kET_Blogg)
CAST_TO_PTR_IMPL(CBlogg, kET_Blogg)
CAST_TO_REF_IMPL(CWallWalker, kET_WallWalker)
CAST_TO_PTR_IMPL(CWallWalker, kET_WallWalker)
// 151: class not declared yet (Shredder REL)
CAST_TO_REF_IMPL(CTargetableProjectile, kET_TargetableProjectile)
CAST_TO_PTR_IMPL(CTargetableProjectile, kET_TargetableProjectile)
CAST_TO_REF_IMPL(CAIMannedTurret, kET_AIMannedTurret)
CAST_TO_PTR_IMPL(CAIMannedTurret, kET_AIMannedTurret)
CAST_TO_REF_IMPL(CStoneToad, kET_StoneToad)
CAST_TO_PTR_IMPL(CStoneToad, kET_StoneToad)
CAST_TO_REF_IMPL(CScriptFrontEndDataNetwork, kET_ScriptFrontEndDataNetwork)
CAST_TO_PTR_IMPL(CScriptFrontEndDataNetwork, kET_ScriptFrontEndDataNetwork)
CAST_TO_REF_IMPL(CPowerBomb, kET_PowerBomb)
CAST_TO_PTR_IMPL(CPowerBomb, kET_PowerBomb)
// 157: class not declared yet (Krocuss REL)
CAST_TO_REF_IMPL(COctapedeSegment, kET_OctapedeSegment)
CAST_TO_PTR_IMPL(COctapedeSegment, kET_OctapedeSegment)
// 159: class not declared yet (PuddleSpore REL)
CAST_TO_REF_IMPL(CScriptForgottenObject, kET_ScriptForgottenObject)
CAST_TO_PTR_IMPL(CScriptForgottenObject, kET_ScriptForgottenObject)

template <>
CActor* TCastToPtr< CActor >(CEntity* entity) {
  if (entity != nullptr && (entity->GetCastFlags() & 1) != 0) {
    return static_cast< CActor* >(entity);
  }
  return nullptr;
}

template <>
CActor* TCastToPtr< CActor >(CEntity& entity) {
  if ((entity.GetCastFlags() & 1) != 0) {
    return static_cast< CActor* >(&entity);
  }
  return nullptr;
}

template <>
CPhysicsActor* TCastToPtr< CPhysicsActor >(CEntity* entity) {
  if (entity != nullptr && (entity->GetCastFlags() & 2) != 0) {
    return static_cast< CPhysicsActor* >(entity);
  }
  return nullptr;
}

template <>
CPhysicsActor* TCastToPtr< CPhysicsActor >(CEntity& entity) {
  if ((entity.GetCastFlags() & 2) != 0) {
    return static_cast< CPhysicsActor* >(&entity);
  }
  return nullptr;
}

template <>
CPatterned* TCastToPtr< CPatterned >(CEntity* entity) {
  if (entity != nullptr && (entity->GetCastFlags() & 4) != 0) {
    return static_cast< CPatterned* >(entity);
  }
  return nullptr;
}

template <>
CPatterned* TCastToPtr< CPatterned >(CEntity& entity) {
  if ((entity.GetCastFlags() & 4) != 0) {
    return static_cast< CPatterned* >(&entity);
  }
  return nullptr;
}

// Casts for cast flag 8 (pointer, then reference form): class not identified yet

CBeamProjectile::~CBeamProjectile() {}

CPlasmaProjectile::~CPlasmaProjectile() {}

// Raw decompiler output (mwdec) for TypesMatch/TCastToPtr instances of classes not declared yet.

extern "C" bool fn_8009D63C() {
    return false;
}

extern "C" void fn_80097520(int arg0) {
    rstl::destroy<CImpactVisorEffect::SParticleEffect>((CImpactVisorEffect::SParticleEffect*)arg0);
}

extern "C" void fn_80097654(int arg0) {
    TryCast((CEntity*)arg0, 159);
}

extern "C" void fn_800976FC(int arg0) {
    TryCast((CEntity*)arg0, 157);
}

extern "C" void fn_8009784C(int arg0) {
    TryCast((CEntity*)arg0, 153);
}

extern "C" void fn_800977F8(int arg0) {
    TryCast((CEntity*)arg0, 154);
}

extern "C" void fn_800978F4(int arg0) {
    TryCast((CEntity*)arg0, 151);
}

extern "C" void fn_80097A44(int arg0) {
    TryCast((CEntity*)arg0, 147);
}

extern "C" void fn_800979F0(int arg0) {
    TryCast((CEntity*)arg0, 148);
}

extern "C" void fn_80097A98(int arg0) {
    TryCast((CEntity*)arg0, 146);
}

extern "C" void fn_80097AEC(int arg0) {
    TryCast((CEntity*)arg0, 145);
}

extern "C" void fn_80097B94(int arg0) {
    TryCast((CEntity*)arg0, 143);
}

extern "C" void fn_80097B40(int arg0) {
    TryCast((CEntity*)arg0, 144);
}

extern "C" void fn_80097BE8(int arg0) {
    TryCast((CEntity*)arg0, 142);
}

extern "C" void fn_80097C3C(int arg0) {
    TryCast((CEntity*)arg0, 141);
}

extern "C" void fn_80097D8C(int arg0) {
    TryCast((CEntity*)arg0, 137);
}

extern "C" void fn_80097DE0(int arg0) {
    TryCast((CEntity*)arg0, 136);
}

extern "C" void fn_80097E34(int arg0) {
    TryCast((CEntity*)arg0, 135);
}

extern "C" void fn_80097E88(int arg0) {
    TryCast((CEntity*)arg0, 134);
}

extern "C" void fn_8009802C(int arg0) {
    TryCast((CEntity*)arg0, 129);
}

extern "C" void fn_800980D4(int arg0) {
    TryCast((CEntity*)arg0, 127);
}

extern "C" void fn_8009817C(int arg0) {
    TryCast((CEntity*)arg0, 125);
}

extern "C" void fn_80098320(int arg0) {
    TryCast((CEntity*)arg0, 120);
}

extern "C" void fn_800983C8(int arg0) {
    TryCast((CEntity*)arg0, 118);
}

extern "C" void fn_80098374(int arg0) {
    TryCast((CEntity*)arg0, 119);
}

extern "C" void fn_8009841C(int arg0) {
    TryCast((CEntity*)arg0, 117);
}

extern "C" void fn_80098518(int arg0) {
    TryCast((CEntity*)arg0, 114);
}

extern "C" void fn_80098470(int arg0) {
    TryCast((CEntity*)arg0, 116);
}

extern "C" void fn_800984C4(int arg0) {
    TryCast((CEntity*)arg0, 115);
}

extern "C" void fn_800985C0(int arg0) {
    TryCast((CEntity*)arg0, 112);
}

extern "C" void fn_8009856C(int arg0) {
    TryCast((CEntity*)arg0, 113);
}

extern "C" void fn_80098614(int arg0) {
    TryCast((CEntity*)arg0, 111);
}

extern "C" void fn_80098710(int arg0) {
    TryCast((CEntity*)arg0, 108);
}

extern "C" void fn_80098764(int arg0) {
    TryCast((CEntity*)arg0, 107);
}

extern "C" void fn_800987B8(int arg0) {
    TryCast((CEntity*)arg0, 106);
}

extern "C" void fn_8009880C(int arg0) {
    TryCast((CEntity*)arg0, 105);
}

extern "C" void fn_800991E4(int arg0) {
    TryCast((CEntity*)arg0, 75);
}


extern "C" void fn_8009A0FC(int arg0) {
    TryCast((CEntity*)arg0, 29);
}

extern "C" void fn_8009A39C(int arg0) {
    TryCast((CEntity*)arg0, 21);
}

struct __mwdec_vt_0 { virtual void _0(); virtual void _1(int); };
extern "C" void fn_80097678(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(159);
}

extern "C" void fn_80097720(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(157);
}

extern "C" void fn_8009781C(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(154);
}

extern "C" void fn_80097870(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(153);
}

extern "C" void fn_80097918(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(151);
}

extern "C" void fn_80097A14(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(148);
}

extern "C" void fn_80097A68(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(147);
}

extern "C" void fn_80097ABC(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(146);
}

extern "C" void fn_80097B10(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(145);
}

extern "C" void fn_80097B64(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(144);
}

extern "C" void fn_80097BB8(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(143);
}

extern "C" void fn_80097C60(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(141);
}

extern "C" void fn_80097C0C(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(142);
}

extern "C" void fn_80097DB0(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(137);
}

extern "C" void fn_80097E04(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(136);
}

extern "C" void fn_80097E58(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(135);
}

extern "C" void fn_80097EAC(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(134);
}

extern "C" void fn_80098050(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(129);
}

extern "C" void fn_800980F8(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(127);
}

extern "C" void fn_800981A0(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(125);
}

extern "C" void fn_80098344(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(120);
}

extern "C" void fn_80098398(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(119);
}

extern "C" void fn_800983EC(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(118);
}

extern "C" void fn_80098440(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(117);
}

extern "C" void fn_80098494(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(116);
}

extern "C" void fn_800984E8(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(115);
}

extern "C" void fn_8009853C(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(114);
}

extern "C" void fn_800985E4(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(112);
}

extern "C" void fn_80098590(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(113);
}

extern "C" void fn_80098638(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(111);
}

extern "C" void fn_80098734(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(108);
}

extern "C" void fn_80098788(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(107);
}

extern "C" void fn_800987DC(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(106);
}

extern "C" void fn_80098830(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(105);
}

extern "C" void fn_80099208(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(75);
}


extern "C" void fn_8009A120(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(29);
}

extern "C" void fn_8009A3C0(int arg0) {
    ((__mwdec_vt_0*)arg0)->_1(21);
}

extern "C" int fn_8009ABEC(int arg0, int arg1) {
    if (arg1 == 149) {
        return arg0;
    }
    if (arg1 > 149) {
        return 0;
    }
    return (int)((CPatterned*)arg0)->CPatterned::TypesMatch(arg1);
}

extern "C" int fn_8009AC24(int arg0, int arg1) {
    if (arg1 == 148) {
        return arg0;
    }
    if (arg1 > 148) {
        return 0;
    }
    return (int)((CPhysicsActor*)arg0)->CPhysicsActor::TypesMatch(arg1);
}

extern "C" int fn_8009AC5C(int arg0, int arg1) {
    if (arg1 == 147) {
        return arg0;
    }
    if (arg1 > 147) {
        return 0;
    }
    return (int)((CPatterned*)arg0)->CPatterned::TypesMatch(arg1);
}

extern "C" int fn_8009AC94(int arg0, int arg1) {
    if (arg1 == 146) {
        return arg0;
    }
    if (arg1 > 146) {
        return 0;
    }
    return (int)((CPatterned*)arg0)->CPatterned::TypesMatch(arg1);
}

extern "C" int fn_8009ACCC(int arg0, int arg1) {
    if (arg1 == 145) {
        return arg0;
    }
    if (arg1 > 145) {
        return 0;
    }
    return (int)((CPatterned*)arg0)->CPatterned::TypesMatch(arg1);
}

extern "C" int fn_8009AD04(int arg0, int arg1) {
    if (arg1 == 144) {
        return arg0;
    }
    if (arg1 > 144) {
        return 0;
    }
    return (int)((CPhysicsActor*)arg0)->CPhysicsActor::TypesMatch(arg1);
}

extern "C" int fn_8009AD3C(int arg0, int arg1) {
    if (arg1 == 143) {
        return arg0;
    }
    if (arg1 > 143) {
        return 0;
    }
    return (int)((CPatterned*)arg0)->CPatterned::TypesMatch(arg1);
}

extern "C" int fn_8009AE8C(int arg0, int arg1) {
    if (arg1 == 137) {
        return arg0;
    }
    if (arg1 > 137) {
        return 0;
    }
    return (int)((CActor*)arg0)->CActor::TypesMatch(arg1);
}

extern "C" int fn_8009AEFC(int arg0, int arg1) {
    if (arg1 == 135) {
        return arg0;
    }
    if (arg1 > 135) {
        return 0;
    }
    return (int)((CPatterned*)arg0)->CPatterned::TypesMatch(arg1);
}

extern "C" int fn_8009AF34(int arg0, int arg1) {
    if (arg1 == 134) {
        return arg0;
    }
    if (arg1 > 134) {
        return 0;
    }
    return (int)((CPatterned*)arg0)->CPatterned::TypesMatch(arg1);
}


extern "C" int fn_8009B04C(int arg0, int arg1) {
    if (arg1 == 129) {
        return arg0;
    }
    if (arg1 > 129) {
        return 0;
    }
    return (int)((CPatterned*)arg0)->CPatterned::TypesMatch(arg1);
}

extern "C" int fn_8009B2EC(int arg0, int arg1) {
    if (arg1 == 117) {
        return arg0;
    }
    if (arg1 > 117) {
        return 0;
    }
    return (int)((CPatterned*)arg0)->CPatterned::TypesMatch(arg1);
}

extern "C" int fn_8009B324(int arg0, int arg1) {
    if (arg1 == 116) {
        return arg0;
    }
    if (arg1 > 116) {
        return 0;
    }
    return (int)((CPatterned*)arg0)->CPatterned::TypesMatch(arg1);
}

extern "C" int fn_8009B394(int arg0, int arg1) {
    if (arg1 == 114) {
        return arg0;
    }
    if (arg1 > 114) {
        return 0;
    }
    return (int)((CPatterned*)arg0)->CPatterned::TypesMatch(arg1);
}

extern "C" int fn_8009B3CC(int arg0, int arg1) {
    if (arg1 == 113) {
        return arg0;
    }
    if (arg1 > 113) {
        return 0;
    }
    return (int)((CPatterned*)arg0)->CPatterned::TypesMatch(arg1);
}

extern "C" int fn_8009B634();

extern "C" int fn_8009B404(int arg0, int arg1) {
    if (arg1 == 112) {
        return arg0;
    }
    if (arg1 > 112) {
        return 0;
    }
    return (int)((CPatterned*)arg0)->CPatterned::TypesMatch(arg1);
}

extern "C" int fn_8009B58C(int arg0, int arg1) {
    if (arg1 == 105) {
        return arg0;
    }
    if (arg1 > 105) {
        return 0;
    }
    return (int)((CActor*)arg0)->CActor::TypesMatch(arg1);
}

extern "C" int fn_8009C7EC(int arg0, int arg1) {
    if (arg1 == 21) {
        return arg0;
    }
    if (arg1 > 21) {
        return 0;
    }
    return (int)((CWeapon*)arg0)->CWeapon::TypesMatch(arg1);
}

extern "C" int fn_8009B43C(int arg0, int arg1) {
    if (arg1 == 111) {
        return arg0;
    }
    if (arg1 > 111) {
        return 0;
    }
    return (int)((CPatterned*)arg0)->CPatterned::TypesMatch(arg1);
}

extern "C" int fn_8009D4C8(int arg0, int arg1) {
    if (arg0) {
        CMemory::Free((const void*)*(int*)(arg0 + 0xc));
        if ((short)arg1 > 0) {
            CMemory::Free((const void*)arg0);
        }
    }
    return arg0;
}

extern "C" int fn_8009D5C8(int arg0, int arg1) {
    int var_r31;
    int temp_r3;
    if (arg0) {
        var_r31 = *(int*)(arg0 + 0x4);
        while ((unsigned int)var_r31 != (*(int*)(arg0 + 0x8))) {
            temp_r3 = var_r31;
            var_r31 = *(int*)(var_r31 + 0x4);
            CMemory::Free((const void*)temp_r3);
        }
        if ((short)arg1 > 0) {
            CMemory::Free((const void*)arg0);
        }
    }
    return arg0;
}

extern unsigned char lbl_803B317C[20];
extern "C" int fn_8009D580(int arg0, int arg1) {
    if (arg0) {
        *(int*)arg0 = (int)lbl_803B317C;
        if ((short)arg1 > 0) {
            CMemory::Free((const void*)arg0);
        }
    }
    return arg0;
}

extern "C" int fn_80097540(int obj) {
    int result = obj;
    if (!((*(unsigned char*)((char*)obj + 0x20)) >> 2 & 8)) {
        result = 0;
    }
    return result;
}

extern "C" int fn_80097554(int obj) {
    if ((unsigned int)obj != 0) {
        if ((*(unsigned char*)((char*)obj + 0x20)) >> 2 & 8) {
            return obj;
        }
    }
    return 0;
}


#undef TYPES_MATCH_IMPL
#undef CAST_TO_PTR_IMPL
#undef CAST_TO_REF_IMPL
