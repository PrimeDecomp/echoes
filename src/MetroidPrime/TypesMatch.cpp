#include "MetroidPrime/CEntity.hpp"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPickup.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSequenceTimer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpawnPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptStreamedMusic.hpp"

// The remaining cast and class overrides in the original TU are still unimplemented.
CEntity* TryCast(CEntity* entity, int typeId) {
  if (entity != nullptr) {
    return entity->TypesMatch(typeId);
  }
  return nullptr;
}

CEntity* CEntity::TypesMatch(int typeId) const {
  return typeId == kET_Entity ? const_cast< CEntity* >(this) : nullptr;
}

#define TYPES_MATCH_IMPL(cls, parent, id)                                                        \
  CEntity* cls::TypesMatch(int typeId) const {                                                   \
    if (typeId == id) {                                                                          \
      return const_cast< cls* >(this);                                                           \
    }                                                                                            \
    if (typeId > id) {                                                                           \
      return nullptr;                                                                            \
    }                                                                                            \
    return parent::TypesMatch(typeId);                                                           \
  }

TYPES_MATCH_IMPL(CActor, CEntity, kET_Actor)
TYPES_MATCH_IMPL(CPhysicsActor, CActor, kET_PhysicsActor)
TYPES_MATCH_IMPL(CScriptSequenceTimer, CEntity, kET_ScriptSequenceTimer)
TYPES_MATCH_IMPL(CPlayer, CPhysicsActor, kET_Player)
TYPES_MATCH_IMPL(CScriptActor, CPhysicsActor, kET_ScriptActor)
TYPES_MATCH_IMPL(CScriptEffect, CActor, kET_ScriptEffect)
TYPES_MATCH_IMPL(CScriptPickup, CActor, kET_ScriptPickup)
TYPES_MATCH_IMPL(CScriptSpawnPoint, CEntity, kET_ScriptSpawnPoint)
TYPES_MATCH_IMPL(CScriptStreamedMusic, CEntity, kET_ScriptStreamedMusic)

#undef TYPES_MATCH_IMPL

#define CAST_TO_PTR_IMPL(cls, id)                     \
  template <>                                        \
  cls* TCastToPtr< cls >(CEntity* entity) {          \
    return static_cast< cls* >(TryCast(entity, id)); \
  }

CAST_TO_PTR_IMPL(CPlayer, kET_Player)
CAST_TO_PTR_IMPL(CScriptActor, kET_ScriptActor)
CAST_TO_PTR_IMPL(CScriptEffect, kET_ScriptEffect)

#undef CAST_TO_PTR_IMPL
