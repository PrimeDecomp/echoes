#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSound.hpp"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptForgottenObject.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPickup.hpp"
#include "MetroidPrime/ScriptObjects/CScriptRepulsor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSequenceTimer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpawnPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptStreamedMusic.hpp"
#include "MetroidPrime/TCastTo.hpp"

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

TYPES_MATCH_IMPL(CScriptWaypoint, CActor, kET_ScriptWaypoint)

TYPES_MATCH_IMPL(CScriptSequenceTimer, CEntity, kET_ScriptSequenceTimer)

TYPES_MATCH_IMPL(CGameLight, CActor, kET_GameLight)

TYPES_MATCH_IMPL(CPlayer, CPhysicsActor, kET_Player)

TYPES_MATCH_IMPL(CScriptActor, CPhysicsActor, kET_ScriptActor)

TYPES_MATCH_IMPL(CScriptCameraWaypoint, CScriptWaypoint, kET_ScriptCameraWaypoint)

TYPES_MATCH_IMPL(CScriptEffect, CActor, kET_ScriptEffect)

TYPES_MATCH_IMPL(CScriptPickup, CActor, kET_ScriptPickup)

TYPES_MATCH_IMPL(CScriptPlatform, CPhysicsActor, kET_ScriptPlatform)

TYPES_MATCH_IMPL(CScriptRepulsor, CActor, kET_ScriptRepulsor)

TYPES_MATCH_IMPL(CScriptSound, CActor, kET_ScriptSound)

TYPES_MATCH_IMPL(CScriptSpawnPoint, CEntity, kET_ScriptSpawnPoint)

TYPES_MATCH_IMPL(CScriptStreamedMusic, CEntity, kET_ScriptStreamedMusic)

TYPES_MATCH_IMPL(CScriptForgottenObject, CEntity, kET_ScriptForgottenObject)

CAST_TO_REF_IMPL(CEntity, kET_Entity)

CAST_TO_PTR_IMPL(CEntity, kET_Entity)

CAST_TO_REF_IMPL(CScriptSequenceTimer, kET_ScriptSequenceTimer)

CAST_TO_PTR_IMPL(CScriptSequenceTimer, kET_ScriptSequenceTimer)

CAST_TO_PTR_IMPL(CGameLight, kET_GameLight)

CAST_TO_REF_IMPL(CPlayer, kET_Player)

CAST_TO_PTR_IMPL(CPlayer, kET_Player)

CAST_TO_REF_IMPL(CScriptActor, kET_ScriptActor)

CAST_TO_PTR_IMPL(CScriptActor, kET_ScriptActor)

CAST_TO_PTR_IMPL(CScriptCameraWaypoint, kET_ScriptCameraWaypoint)

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

#undef TYPES_MATCH_IMPL
#undef CAST_TO_PTR_IMPL
#undef CAST_TO_REF_IMPL
