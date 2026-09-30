#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Cameras/CCinematicCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCamera.hpp"
#include "MetroidPrime/Cameras/CSpindleCamera.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPathCamera.hpp"
#include "MetroidPrime/Cameras/CPathCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpindleCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraShaker.hpp"
#include "MetroidPrime/ScriptObjects/CScriptColorModulate.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSound.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpecialFunction.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActorRotate.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDock.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDebris.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDoor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptForgottenObject.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPickup.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPickupGenerator.hpp"
#include "MetroidPrime/ScriptObjects/CScriptRepulsor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSequenceTimer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpawnPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptStreamedMusic.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CBeamProjectile.hpp"
#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "MetroidPrime/Weapons/CBomb.hpp"

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

// The remaining cast and class overrides in the original TU are still unimplemented.
CGameLight::~CGameLight() {}

CEnergyProjectile::~CEnergyProjectile() {}

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
TYPES_MATCH_IMPL(CWeapon, CActor, kET_Weapon)
TYPES_MATCH_IMPL(CBomb, CWeapon, kET_Bomb)
TYPES_MATCH_IMPL(CGameProjectile, CWeapon, kET_GameProjectile)
TYPES_MATCH_IMPL(CEnergyProjectile, CGameProjectile, kET_EnergyProjectile)
CAST_TO_REF_IMPL(CEnergyProjectile, kET_EnergyProjectile)
TYPES_MATCH_IMPL(CBeamProjectile, CGameProjectile, kET_BeamProjectile)
TYPES_MATCH_IMPL(CPlasmaProjectile, CBeamProjectile, kET_PlasmaProjectile)
TYPES_MATCH_IMPL(CGameCamera, CActor, kET_GameCamera)
TYPES_MATCH_IMPL(CCinematicCamera, CGameCamera, kET_CinematicCamera)
TYPES_MATCH_IMPL(CScriptCamera, CActor, kET_ScriptCamera)
TYPES_MATCH_IMPL(CBallCamera, CGameCamera, kET_BallCamera)
TYPES_MATCH_IMPL(CFirstPersonCamera, CGameCamera, kET_FirstPersonCamera)
TYPES_MATCH_IMPL(CSpindleCamera, CGameCamera, kET_SpindleCamera)
CAST_TO_PTR_IMPL(CGameCamera, kET_GameCamera)
CAST_TO_REF_IMPL(CGameCamera, kET_GameCamera)
CAST_TO_PTR_IMPL(CGameProjectile, kET_GameProjectile)
CAST_TO_REF_IMPL(CGameProjectile, kET_GameProjectile)
TYPES_MATCH_IMPL(CPhysicsActor, CActor, kET_PhysicsActor)
TYPES_MATCH_IMPL(CCollisionActor, CPhysicsActor, kET_CollisionActor)
TYPES_MATCH_IMPL(CAi, CPhysicsActor, kET_Ai)
TYPES_MATCH_IMPL(CPatterned, CAi, kET_Patterned)
TYPES_MATCH_IMPL(CScriptWaypoint, CActor, kET_ScriptWaypoint)
TYPES_MATCH_IMPL(CScriptSpindleCamera, CActor, kET_ScriptSpindleCamera)
TYPES_MATCH_IMPL(CScriptPathCamera, CEntity, kET_ScriptPathCamera)
TYPES_MATCH_IMPL(CPathCamera, CGameCamera, kET_PathCamera)
TYPES_MATCH_IMPL(CScriptSequenceTimer, CEntity, kET_ScriptSequenceTimer)
TYPES_MATCH_IMPL(CGameLight, CActor, kET_GameLight)
TYPES_MATCH_IMPL(CPlayer, CPhysicsActor, kET_Player)
TYPES_MATCH_IMPL(CGameHint, CActor, kET_GameHint)
TYPES_MATCH_IMPL(CScriptCameraHint, CGameHint, kET_ScriptCameraHint)
TYPES_MATCH_IMPL(CScriptActor, CPhysicsActor, kET_ScriptActor)
TYPES_MATCH_IMPL(CScriptActorRotate, CEntity, kET_ScriptActorRotate)
TYPES_MATCH_IMPL(CScriptCameraShaker, CEntity, kET_ScriptCameraShaker)
TYPES_MATCH_IMPL(CScriptCameraWaypoint, CScriptWaypoint, kET_ScriptCameraWaypoint)
TYPES_MATCH_IMPL(CScriptColorModulate, CEntity, kET_ScriptColorModulate)
TYPES_MATCH_IMPL(CScriptDock, CPhysicsActor, kET_ScriptDock)
TYPES_MATCH_IMPL(CScriptDoor, CPhysicsActor, kET_ScriptDoor)
TYPES_MATCH_IMPL(CScriptEffect, CActor, kET_ScriptEffect)
TYPES_MATCH_IMPL(CScriptPickup, CActor, kET_ScriptPickup)
TYPES_MATCH_IMPL(CScriptDebris, CPhysicsActor, kET_ScriptDebris)
TYPES_MATCH_IMPL(CScriptPickupGenerator, CEntity, kET_ScriptPickupGenerator)
TYPES_MATCH_IMPL(CScriptPlatform, CPhysicsActor, kET_ScriptPlatform)
TYPES_MATCH_IMPL(CScriptRepulsor, CActor, kET_ScriptRepulsor)
TYPES_MATCH_IMPL(CScriptSound, CActor, kET_ScriptSound)
TYPES_MATCH_IMPL(CScriptSpecialFunction, CActor, kET_ScriptSpecialFunction)
TYPES_MATCH_IMPL(CScriptTeamAiMgr, CEntity, kET_ScriptTeamAi)
TYPES_MATCH_IMPL(CScriptTrigger, CActor, kET_ScriptTrigger)
TYPES_MATCH_IMPL(CScriptWater, CScriptTrigger, kET_ScriptWater)
TYPES_MATCH_IMPL(CScriptSpawnPoint, CEntity, kET_ScriptSpawnPoint)
TYPES_MATCH_IMPL(CScriptStreamedMusic, CEntity, kET_ScriptStreamedMusic)
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
CAST_TO_PTR_IMPL(CScriptPathCamera, kET_ScriptPathCamera)
CAST_TO_PTR_IMPL(CScriptCamera, kET_ScriptCamera)
CAST_TO_PTR_IMPL(CScriptDock, kET_ScriptDock)
CAST_TO_REF_IMPL(CScriptDoor, kET_ScriptDoor)
CAST_TO_PTR_IMPL(CScriptDoor, kET_ScriptDoor)
CAST_TO_REF_IMPL(CScriptEffect, kET_ScriptEffect)
CAST_TO_PTR_IMPL(CScriptEffect, kET_ScriptEffect)
CAST_TO_REF_IMPL(CScriptPickup, kET_ScriptPickup)
CAST_TO_PTR_IMPL(CScriptPickup, kET_ScriptPickup)
CAST_TO_REF_IMPL(CScriptSpawnPoint, kET_ScriptSpawnPoint)
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
