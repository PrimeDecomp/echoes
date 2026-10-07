#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/Enemies/CSwarmBasics.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Cameras/CFixedCamera.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Cameras/CCinematicCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCamera.hpp"
#include "MetroidPrime/Cameras/CSpindleCamera.hpp"
#include "MetroidPrime/Cameras/CSurfaceCamera.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/Weapons/CHomingBlob.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDynamicLight.hpp"
#include "MetroidPrime/CEffect.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSafeZone.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCounter.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGrapplePoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDistanceFog.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDamageableTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDamageableTriggerOrientated.hpp"
#include "MetroidPrime/ScriptObjects/CScriptMemoryRelay.hpp"
#include "MetroidPrime/ScriptObjects/CScriptRelay.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTimer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTriggerEllipsoid.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlayerHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptControlHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptRoomAcoustics.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPathCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptLayerController.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPortalTransition.hpp"
#include "MetroidPrime/ScriptObjects/CScriptHUDHint.hpp"
#include "MetroidPrime/Cameras/CPathCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpindleCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSurfaceCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraShaker.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraPitch.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPathMeshCtrl.hpp"
#include "MetroidPrime/ScriptObjects/CScriptColorModulate.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSound.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSoundModifier.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpecialFunction.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTriggerOrientated.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTargetingPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpiderBallAttractionSurface.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpiderBallWaypoint.hpp"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActorKeyframe.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActorRotate.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAiJumpPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDock.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDebris.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDoor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptForgottenObject.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPickup.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPickupGenerator.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPointOfInterest.hpp"
#include "MetroidPrime/ScriptObjects/CScriptRepulsor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSequenceTimer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpawnPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptStreamedMusic.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSwitch.hpp"
#include "MetroidPrime/ScriptObjects/CScriptVisorFlare.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWorldTeleporter.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Enemies/CBouncyGrenade.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CLightComboProjectile.hpp"
#include "MetroidPrime/Weapons/CBeamProjectile.hpp"
#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "MetroidPrime/Weapons/CBomb.hpp"
#include "MetroidPrime/Weapons/CPowerBomb.hpp"

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

CAST_TO_REF_IMPL(CSwarmBasics, kET_SwarmBasics)
CAST_TO_PTR_IMPL(CSwarmBasics, kET_SwarmBasics)

CEntity* CastToSnakeWeedSwarm(CEntity* entity) { return TryCast(entity, kET_SnakeWeedSwarm); }

// The remaining cast and class overrides in the original TU are still unimplemented.
CPlasmaProjectile::~CPlasmaProjectile() {}

CBeamProjectile::~CBeamProjectile() {}

CScriptTargetingPoint::~CScriptTargetingPoint() {}

CScriptRoomAcoustics::~CScriptRoomAcoustics() {}

CScriptDamageableTriggerOrientated::~CScriptDamageableTriggerOrientated() {}

CScriptCoverPoint::~CScriptCoverPoint() {}

CScriptAiJumpPoint::~CScriptAiJumpPoint() {}

CEnergyProjectile::~CEnergyProjectile() {}

CScriptPortalTransition::~CScriptPortalTransition() {}

CScriptHUDHint::~CScriptHUDHint() {}

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
TYPES_MATCH_IMPL(CEffect, CActor, kET_Effect)
TYPES_MATCH_IMPL(CExplosion, CEffect, kET_Explosion)
TYPES_MATCH_IMPL(CWeapon, CActor, kET_Weapon)
CAST_TO_REF_IMPL(CWeapon, kET_Weapon)
TYPES_MATCH_IMPL(CBomb, CWeapon, kET_Bomb)
TYPES_MATCH_IMPL(CPowerBomb, CWeapon, kET_PowerBomb)
TYPES_MATCH_IMPL(CGameProjectile, CWeapon, kET_GameProjectile)
TYPES_MATCH_IMPL(CEnergyProjectile, CGameProjectile, kET_EnergyProjectile)
TYPES_MATCH_IMPL(CLightComboProjectile, CEnergyProjectile, kET_LightComboProjectile)
CAST_TO_REF_IMPL(CEnergyProjectile, kET_EnergyProjectile)
TYPES_MATCH_IMPL(CBeamProjectile, CGameProjectile, kET_BeamProjectile)
TYPES_MATCH_IMPL(CPlasmaProjectile, CBeamProjectile, kET_PlasmaProjectile)
TYPES_MATCH_IMPL(CGameCamera, CActor, kET_GameCamera)
TYPES_MATCH_IMPL(CCinematicCamera, CGameCamera, kET_CinematicCamera)
TYPES_MATCH_IMPL(CBouncyGrenade, CPhysicsActor, kET_BouncyGrenade)
TYPES_MATCH_IMPL(CScriptCamera, CActor, kET_ScriptCamera)
TYPES_MATCH_IMPL(CBallCamera, CGameCamera, kET_BallCamera)
TYPES_MATCH_IMPL(CFirstPersonCamera, CGameCamera, kET_FirstPersonCamera)
TYPES_MATCH_IMPL(CFixedCamera, CGameCamera, kET_FixedCamera)
TYPES_MATCH_IMPL(CSpindleCamera, CGameCamera, kET_SpindleCamera)
TYPES_MATCH_IMPL(CSurfaceCamera, CGameCamera, kET_SurfaceCamera)
CAST_TO_PTR_IMPL(CGameCamera, kET_GameCamera)
CAST_TO_PTR_IMPL(CBouncyGrenade, kET_BouncyGrenade)
CAST_TO_REF_IMPL(CGameCamera, kET_GameCamera)
CAST_TO_PTR_IMPL(CGameProjectile, kET_GameProjectile)
CAST_TO_REF_IMPL(CGameProjectile, kET_GameProjectile)
TYPES_MATCH_IMPL(CPhysicsActor, CActor, kET_PhysicsActor)
TYPES_MATCH_IMPL(CCollisionActor, CPhysicsActor, kET_CollisionActor)
TYPES_MATCH_IMPL(CAi, CPhysicsActor, kET_Ai)
TYPES_MATCH_IMPL(CPatterned, CAi, kET_Patterned)
TYPES_MATCH_IMPL(CScriptWaypoint, CActor, kET_ScriptWaypoint)
TYPES_MATCH_IMPL(CScriptPathMeshCtrl, CActor, kET_ScriptPathMeshCtrl)
TYPES_MATCH_IMPL(CScriptSpindleCamera, CActor, kET_ScriptSpindleCamera)
TYPES_MATCH_IMPL(CScriptSurfaceCamera, CActor, kET_ScriptSurfaceCamera)
CAST_TO_PTR_IMPL(CScriptSurfaceCamera, kET_ScriptSurfaceCamera)
TYPES_MATCH_IMPL(CScriptLayerController, CEntity, kET_ScriptLayerController)
TYPES_MATCH_IMPL(CScriptPathCamera, CEntity, kET_ScriptPathCamera)
TYPES_MATCH_IMPL(CScriptPortalTransition, CEntity, kET_ScriptPortalTransition)
TYPES_MATCH_IMPL(CScriptHUDHint, CActor, kET_ScriptHUDHint)
CAST_TO_REF_IMPL(CScriptHUDHint, kET_ScriptHUDHint)
CAST_TO_PTR_IMPL(CScriptHUDHint, kET_ScriptHUDHint)
TYPES_MATCH_IMPL(CPathCamera, CGameCamera, kET_PathCamera)
TYPES_MATCH_IMPL(CScriptSequenceTimer, CEntity, kET_ScriptSequenceTimer)
TYPES_MATCH_IMPL(CGameLight, CActor, kET_GameLight)
TYPES_MATCH_IMPL(CHomingBlob, CWeapon, kET_HomingBlob)
TYPES_MATCH_IMPL(CHUDBillboardEffect, CEffect, kET_HUDBillboardEffect)
TYPES_MATCH_IMPL(CPlayer, CPhysicsActor, kET_Player)
TYPES_MATCH_IMPL(CGameHint, CActor, kET_GameHint)
TYPES_MATCH_IMPL(CScriptCameraHint, CGameHint, kET_ScriptCameraHint)
TYPES_MATCH_IMPL(CScriptPlayerHint, CGameHint, kET_ScriptPlayerHint)
TYPES_MATCH_IMPL(CScriptRoomAcoustics, CEntity, kET_ScriptRoomAcoustics)
TYPES_MATCH_IMPL(CScriptCameraPitch, CActor, kET_ScriptCameraPitch)
CAST_TO_PTR_IMPL(CScriptCameraPitch, kET_ScriptCameraPitch)
CAST_TO_PTR_IMPL(CScriptPlayerHint, kET_ScriptPlayerHint)
CAST_TO_PTR_IMPL(CScriptCameraHint, kET_ScriptCameraHint)
CAST_TO_PTR_IMPL(CGameHint, kET_GameHint)
TYPES_MATCH_IMPL(CScriptActor, CPhysicsActor, kET_ScriptActor)
TYPES_MATCH_IMPL(CScriptActorRotate, CEntity, kET_ScriptActorRotate)
TYPES_MATCH_IMPL(CScriptCameraShaker, CEntity, kET_ScriptCameraShaker)
TYPES_MATCH_IMPL(CScriptCameraWaypoint, CScriptWaypoint, kET_ScriptCameraWaypoint)
TYPES_MATCH_IMPL(CScriptAIWaypoint, CScriptWaypoint, kET_ScriptAIWaypoint)
TYPES_MATCH_IMPL(CScriptActorKeyframe, CEntity, kET_ScriptActorKeyframe)
TYPES_MATCH_IMPL(CScriptColorModulate, CEntity, kET_ScriptColorModulate)
TYPES_MATCH_IMPL(CScriptDock, CPhysicsActor, kET_ScriptDock)
TYPES_MATCH_IMPL(CScriptDoor, CPhysicsActor, kET_ScriptDoor)
TYPES_MATCH_IMPL(CScriptDynamicLight, CGameLight, kET_ScriptDynamicLight)
TYPES_MATCH_IMPL(CScriptEffect, CActor, kET_ScriptEffect)
TYPES_MATCH_IMPL(CScriptPickup, CActor, kET_ScriptPickup)
TYPES_MATCH_IMPL(CScriptDebris, CPhysicsActor, kET_ScriptDebris)
TYPES_MATCH_IMPL(CScriptPickupGenerator, CEntity, kET_ScriptPickupGenerator)
TYPES_MATCH_IMPL(CScriptPointOfInterest, CActor, kET_ScriptPointOfInterest)
TYPES_MATCH_IMPL(CScriptPlatform, CPhysicsActor, kET_ScriptPlatform)
TYPES_MATCH_IMPL(CScriptRepulsor, CActor, kET_ScriptRepulsor)
TYPES_MATCH_IMPL(CScriptSound, CActor, kET_ScriptSound)
TYPES_MATCH_IMPL(CScriptSoundModifier, CEntity, kET_ScriptSoundModifier)
TYPES_MATCH_IMPL(CScriptSpecialFunction, CActor, kET_ScriptSpecialFunction)
TYPES_MATCH_IMPL(CScriptTeamAiMgr, CEntity, kET_ScriptTeamAi)
TYPES_MATCH_IMPL(CScriptCounter, CEntity, kET_ScriptCounter)
TYPES_MATCH_IMPL(CScriptControlHint, CGameHint, kET_ScriptControlHint)
CAST_TO_PTR_IMPL(CScriptControlHint, kET_ScriptControlHint)
CAST_TO_PTR_IMPL(CScriptAiJumpPoint, kET_ScriptAiJumpPoint)
CAST_TO_REF_IMPL(CScriptAiJumpPoint, kET_ScriptAiJumpPoint)
CAST_TO_REF_IMPL(CScriptAIWaypoint, kET_ScriptAIWaypoint)
CAST_TO_PTR_IMPL(CScriptAIWaypoint, kET_ScriptAIWaypoint)
TYPES_MATCH_IMPL(CScriptAiJumpPoint, CActor, kET_ScriptAiJumpPoint)
CAST_TO_PTR_IMPL(CScriptAIHint, kET_ScriptAIHint)
CAST_TO_REF_IMPL(CScriptAIHint, kET_ScriptAIHint)
TYPES_MATCH_IMPL(CScriptAIHint, CActor, kET_ScriptAIHint)
CAST_TO_PTR_IMPL(CScriptDamageableTriggerOrientated, kET_ScriptDamageableTriggerOrientated)
CAST_TO_REF_IMPL(CScriptDamageableTriggerOrientated, kET_ScriptDamageableTriggerOrientated)
CAST_TO_PTR_IMPL(CScriptDamageableTrigger, kET_ScriptDamageableTrigger)
CAST_TO_REF_IMPL(CScriptDamageableTrigger, kET_ScriptDamageableTrigger)
CAST_TO_PTR_IMPL(CScriptCoverPoint, kET_ScriptCoverPoint)
CAST_TO_REF_IMPL(CScriptCoverPoint, kET_ScriptCoverPoint)
TYPES_MATCH_IMPL(CScriptCoverPoint, CActor, kET_ScriptCoverPoint)
CAST_TO_PTR_IMPL(CScriptGrapplePoint, kET_ScriptGrapplePoint)
CAST_TO_REF_IMPL(CScriptGrapplePoint, kET_ScriptGrapplePoint)
TYPES_MATCH_IMPL(CScriptGrapplePoint, CActor, kET_ScriptGrapplePoint)
TYPES_MATCH_IMPL(CScriptDamageableTrigger, CActor, kET_ScriptDamageableTrigger)
TYPES_MATCH_IMPL(CScriptDamageableTriggerOrientated, CScriptDamageableTrigger,
                 kET_ScriptDamageableTriggerOrientated)
TYPES_MATCH_IMPL(CScriptDistanceFog, CEntity, kET_ScriptDistanceFog)
TYPES_MATCH_IMPL(CScriptRelay, CEntity, kET_ScriptRelay)
TYPES_MATCH_IMPL(CScriptTimer, CEntity, kET_ScriptTimer)
TYPES_MATCH_IMPL(CScriptTrigger, CActor, kET_ScriptTrigger)
TYPES_MATCH_IMPL(CScriptTriggerEllipsoid, CScriptTrigger, kET_ScriptTriggerEllipsoid)
TYPES_MATCH_IMPL(CScriptTriggerOrientated, CScriptTrigger, kET_ScriptTriggerOrientated)
CAST_TO_REF_IMPL(CScriptTriggerOrientated, kET_ScriptTriggerOrientated)
CAST_TO_PTR_IMPL(CScriptTriggerOrientated, kET_ScriptTriggerOrientated)
TYPES_MATCH_IMPL(CScriptSafeZone, CScriptTriggerEllipsoid, kET_ScriptSafeZone)
TYPES_MATCH_IMPL(CScriptVisorFlare, CActor, kET_ScriptVisorFlare)
TYPES_MATCH_IMPL(CScriptWater, CScriptTrigger, kET_ScriptWater)
TYPES_MATCH_IMPL(CScriptWorldTeleporter, CEntity, kET_ScriptWorldTeleporter)
TYPES_MATCH_IMPL(CScriptSpawnPoint, CEntity, kET_ScriptSpawnPoint)
TYPES_MATCH_IMPL(CScriptStreamedMusic, CEntity, kET_ScriptStreamedMusic)
TYPES_MATCH_IMPL(CScriptSwitch, CEntity, kET_ScriptSwitch)
TYPES_MATCH_IMPL(CScriptTargetingPoint, CActor, kET_ScriptTargetingPoint)
CAST_TO_REF_IMPL(CScriptTargetingPoint, kET_ScriptTargetingPoint)
CAST_TO_PTR_IMPL(CScriptTargetingPoint, kET_ScriptTargetingPoint)
CAST_TO_REF_IMPL(CScriptSpiderBallWaypoint, kET_ScriptSpiderBallWaypoint)
CAST_TO_PTR_IMPL(CScriptSpiderBallWaypoint, kET_ScriptSpiderBallWaypoint)
TYPES_MATCH_IMPL(CScriptSpiderBallAttractionSurface, CActor, kET_ScriptSpiderBallAttractionSurface)
CAST_TO_REF_IMPL(CScriptSpiderBallAttractionSurface, kET_ScriptSpiderBallAttractionSurface)
CAST_TO_PTR_IMPL(CScriptSpiderBallAttractionSurface, kET_ScriptSpiderBallAttractionSurface)
TYPES_MATCH_IMPL(CScriptForgottenObject, CEntity, kET_ScriptForgottenObject)

CAST_TO_REF_IMPL(CEntity, kET_Entity)
CAST_TO_PTR_IMPL(CEntity, kET_Entity)
CAST_TO_PTR_IMPL(CCollisionActor, kET_CollisionActor)
CAST_TO_PTR_IMPL(CScriptTeamAiMgr, kET_ScriptTeamAi)
CAST_TO_REF_IMPL(CScriptSequenceTimer, kET_ScriptSequenceTimer)
CAST_TO_PTR_IMPL(CScriptSequenceTimer, kET_ScriptSequenceTimer)
CAST_TO_PTR_IMPL(CGameLight, kET_GameLight)
CAST_TO_REF_IMPL(CPlayer, kET_Player)
CAST_TO_PTR_IMPL(CPlayer, kET_Player)
CAST_TO_REF_IMPL(CScriptActor, kET_ScriptActor)
CAST_TO_PTR_IMPL(CScriptActor, kET_ScriptActor)
CAST_TO_PTR_IMPL(CScriptCameraWaypoint, kET_ScriptCameraWaypoint)
CAST_TO_PTR_IMPL(CScriptWaypoint, kET_ScriptWaypoint)
CAST_TO_PTR_IMPL(CScriptActorKeyframe, kET_ScriptActorKeyframe)
CAST_TO_PTR_IMPL(CScriptSafeZone, kET_ScriptSafeZone)
CAST_TO_PTR_IMPL(CScriptPathCamera, kET_ScriptPathCamera)
CAST_TO_PTR_IMPL(CScriptCamera, kET_ScriptCamera)
CAST_TO_PTR_IMPL(CScriptPortalTransition, kET_ScriptPortalTransition)
CAST_TO_PTR_IMPL(CScriptDock, kET_ScriptDock)
CAST_TO_REF_IMPL(CScriptDoor, kET_ScriptDoor)
CAST_TO_PTR_IMPL(CScriptDoor, kET_ScriptDoor)
CAST_TO_REF_IMPL(CScriptDynamicLight, kET_ScriptDynamicLight)
CAST_TO_PTR_IMPL(CScriptDynamicLight, kET_ScriptDynamicLight)
CAST_TO_REF_IMPL(CScriptEffect, kET_ScriptEffect)
CAST_TO_PTR_IMPL(CScriptEffect, kET_ScriptEffect)
CAST_TO_REF_IMPL(CScriptPickup, kET_ScriptPickup)
CAST_TO_PTR_IMPL(CScriptPickup, kET_ScriptPickup)
CAST_TO_REF_IMPL(CScriptSpawnPoint, kET_ScriptSpawnPoint)
CAST_TO_PTR_IMPL(CScriptSound, kET_ScriptSound)
CAST_TO_REF_IMPL(CScriptWater, kET_ScriptWater)
CAST_TO_PTR_IMPL(CScriptWater, kET_ScriptWater)
CAST_TO_PTR_IMPL(CScriptSpawnPoint, kET_ScriptSpawnPoint)
CAST_TO_REF_IMPL(CScriptStreamedMusic, kET_ScriptStreamedMusic)
CAST_TO_PTR_IMPL(CScriptStreamedMusic, kET_ScriptStreamedMusic)
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

#undef TYPES_MATCH_IMPL
#undef CAST_TO_PTR_IMPL
#undef CAST_TO_REF_IMPL
