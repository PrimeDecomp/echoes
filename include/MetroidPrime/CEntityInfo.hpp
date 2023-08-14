#ifndef _CENTITYINFO
#define _CENTITYINFO

#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/vector.hpp"

enum EEntityType {
  kET_Entity = 0,
  kET_Actor = 1,
  kET_PhysicsActor = 2,
  kET_Ai = 3,
  kET_Patterned = 4,
  kET_Weapon = 6,
  kET_Effect = 7,
  kET_GameProjectile = 8,
  kET_Bomb = 14,
  kET_EnergyProjectile = 19,
  kET_Explosion = 22,
  kET_GameLight = 26,
  kET_HUDBillboardEffect = 28,
  kET_Player = 32,
  kET_ScriptPlayerActor = 34,
  kET_ScriptActorKeyframe = 35,
  kET_ScriptCameraShaker = 41,
  kET_ScriptCamera = 44,
  kET_ScriptColorModulate = 45,
  kET_DarkSamusBattleStage = 51,
  kET_ScriptDock = 55,
  kET_ScriptDoor = 56,
  kET_ScriptLayerController = 64,
  kET_ScriptPickup = 66,
  kET_ScriptPlayerProxy = 69,
  kET_ScriptPortalTransition = 72,
  kET_Relay = 73,
  kET_ScriptSpawnPoint = 79,
  kET_ScriptSpecialFunction = 80,
  kET_ScriptStreamedMusic = 84,
  kET_ScriptTeamAi = 88,
  kET_ScriptTrigger = 92,
  kET_ScriptWorldTeleporter = 98,
  kET_DarkSamus = 111,
  kET_PowerBomb = 156,
};

enum EScriptObjectState {
  kSS_Active = 0x41435456,
  kSS_Inactive = 0x49435456,
  kSS_InvalidState = 0xffffffff
};

enum EScriptObjectMessage {
  kSM_Activate = 0x41435456,
  kSM_Deactivate = 0x44435456,
  kSM_ToggleActive = 0x54435456,
  kSM_SetToZero = 0x5a45524f,
  kSM_None = 0xffffffff,
};

struct SConnection {
  EScriptObjectState state;
  EScriptObjectMessage msg;
  TEditorId objId;
  SConnection(EScriptObjectState state, EScriptObjectMessage msg, TEditorId id)
  : state(state), msg(msg), objId(id) {}
};

class CEntityInfo {
  TAreaId areaId;
  rstl::vector< SConnection > conns;
  TEditorId editorId;

public:
  CEntityInfo(TAreaId aid, const rstl::vector< SConnection >& conns,
              TEditorId eid = kInvalidEditorId);
  TAreaId GetAreaId() const { return areaId; }
  const rstl::vector< SConnection >& GetConnectionList() const { return conns; }
  TEditorId GetEditorId() const { return editorId; }
};

class CScriptMsg {
public:
  TUniqueId GetOriginator() { return originator; }
  TUniqueId GetId() const { return id; }
  EScriptObjectMessage GetMessage() const { return msg; }
  EScriptObjectState GetState() const { return state; }

public:
  TUniqueId unk;
  TUniqueId originator;
  TUniqueId id;
  EScriptObjectMessage msg;
  EScriptObjectState state;
};

#endif // _CENTITYINFO
