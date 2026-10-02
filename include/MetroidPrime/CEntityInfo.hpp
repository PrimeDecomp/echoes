#ifndef _CENTITYINFO
#define _CENTITYINFO

#include "MetroidPrime/TGameTypes.hpp"

#include "dolphin/types.h"
#include "rstl/vector.hpp"

enum EEntityType {
  kET_Entity = 0,
  kET_Actor = 1,
  kET_PhysicsActor = 2,
  kET_Ai = 3,
  kET_Patterned = 4,
  kET_GameCamera = 5,
  kET_Weapon = 6,
  kET_Effect = 7,
  kET_GameProjectile = 8,
  kET_ScriptWaypoint = 9,
  kET_ScriptSequenceTimer = 12,
  kET_BallCamera = 13,
  kET_Bomb = 14,
  kET_CinematicCamera = 17,
  kET_CollisionActor = 18,
  kET_EnergyProjectile = 19,
  kET_Explosion = 22,
  kET_FirstPersonCamera = 23,
  kET_GameLight = 26,
  kET_HUDBillboardEffect = 28,
  kET_PathCamera = 31,
  kET_Player = 32,
  kET_GameHint = 33, // Guessed name.
  kET_ScriptActor = 34,
  kET_ScriptActorKeyframe = 35,
  kET_ScriptActorRotate = 36,
  kET_ScriptCameraHint = 40,
  kET_ScriptCameraShaker = 41,
  kET_ScriptCameraWaypoint = 43,
  kET_ScriptCamera = 44,
  kET_ScriptColorModulate = 45,
  kET_ScriptCounter = 47,
  kET_DarkSamusBattleStage = 51,
  kET_ScriptDebris = 52,
  kET_ScriptDock = 55,
  kET_ScriptDoor = 56,
  kET_ScriptEffect = 58,
  kET_ScriptLayerController = 64,
  kET_ScriptPathCamera = 65,
  kET_ScriptPickup = 66,
  kET_ScriptPickupGenerator = 67,
  kET_ScriptPlayerProxy = 69,
  kET_ScriptPlatform = 70,
  kET_ScriptPortalTransition = 72,
  kET_Relay = 73,
  kET_ScriptRepulsor = 74,
  kET_ScriptSound = 77,
  kET_ScriptSpawnPoint = 79,
  kET_ScriptSpecialFunction = 80,
  kET_ScriptSpindleCamera = 83,
  kET_ScriptStreamedMusic = 84,
  kET_ScriptTeamAi = 88,
  kET_ScriptSwitch = 86,
  kET_ScriptTimer = 91,
  kET_ScriptTrigger = 92,
  kET_ScriptTriggerEllipsoid = 93,
  kET_ScriptSafeZone = 95, // Guessed name; REL ScriptSafeZone.
  kET_ScriptVisorFlare = 96,
  kET_ScriptWater = 97,
  kET_ScriptWorldTeleporter = 98,
  kET_SpindleCamera = 100,
  kET_BeamProjectile = 109,
  kET_PlasmaProjectile = 110,
  kET_DarkSamus = 111,
  kET_PowerBomb = 156,
  kET_ScriptForgottenObject = 160,
};

enum EScriptObjectState {
  kSS_Active = 0x41435456,
  kSS_Arrived = 0x41525256,
  kSS_Inactive = 0x49435456,
  kSS_Entered = 0x454e5452,
  kSS_Inside = 0x494e5344,
  kSS_Exited = 0x45584954,
  kSS_Zero = 0x5a45524f,
  kSS_NonZero = 0x215a4552,
  kSS_DefaultState = 0x44465354,
  kSS_MaxReached = 0x4d415852,
  kSS_ScanStart = 0x4553434e,
  kSS_ScanProcessing = 0x4253434e,
  kSS_ScanDone = 0x53434e44,
  kSS_Patrol = 0x5054524c,
  kSS_Play = 0x504c4159,
  kSS_Connect = 0x434f4e4e,
  kSS_XINF = 0x58494e46, // Guessed name: first-pass elevator camera.
  kSS_XINB = 0x58494e42, // Guessed name: second-pass elevator camera.
  kSS_Slave = 0x534c4156,
  kSS_Opened = 0x4f50454e,
  kSS_Closed = 0x434c4f53,
  kSS_CameraTarget = 0x43544754,
  kSS_CameraPath = 0x43505448,
  kSS_CameraPlayer = 0x43504c52,
  kSS_CameraTime = 0x4354494d,
  kSS_UnFrozen = 0x5546525a,
  kSS_Dead = 0x44454144,
  kSS_Generate = 0x47454e52,
  kSS_ReflectedDamage = 0x52454644,
  kSS_InheritBounds = 0x49424e44,
  kSS_ScanSource = 0x53434e53,
  kSS_InvalidState = 0xffffffff,
};

enum EScriptObjectMessage {
  kSM_Action = 0x4143544e,
  kSM_Next = 0x4e455854,
  kSM_Start = 0x53545254,
  kSM_Stop = 0x53544f50,
  kSM_Play = 0x504c4159,
  kSM_Load = 0x4c4f4144,
  kSM_Unload = 0x554c4f44,
  kSM_Activate = 0x41435456,
  kSM_Deactivate = 0x44435456,
  kSM_ToggleActive = 0x54435456,
  kSM_SetToZero = 0x5a45524f,
  kSM_SetToMax = 0x534d4158,
  kSM_Reset = 0x52534554,
  kSM_ResetAndStart = 0x52535453,
  kSM_StopAndReset = 0x53545052,
  kSM_Follow = 0x464f4c57,
  kSM_Attach = 0x41544348,
  kSM_Open = 0x4f50454e,
  kSM_Close = 0x434c4f53,
  kSM_Lock = 0x4c4f434b,
  kSM_Unlock = 0x554c434b,

  kSM_Increment = 0x494e4352,
  kSM_Decrement = 0x44454352,
  kSM_Kill = 0x4b494c4c,
  kSM_InternalMessage00 = 0x494d3030,

  kSM_XCRT = 0x58435254,
  kSM_XClear = 0x58434c52, // Guessed name: clear an effect's particles.
  kSM_XALD = 0x58414c44,
  kSM_XWLD = 0x58574c44,
  kSM_XDelete = 0x5844454c,
  kSM_XHIT = 0x58484954,
  kSM_SuspendedMove =
      0x58415544, // Guessed name, sent when a patterned actor's movement is suspended.
  kSM_XXDG = 0x58584447,
  kSM_LandOnNotFloor = 0x5846414c,
  kSM_Falling = 0x584f4646,
  kSM_Jumped = 0x584c4155,
  kSM_OnFloor = 0x584c4e44,

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
  TAreaId mAreaId;
  rstl::vector< SConnection > mConnections;
  TEditorId mEditorId;
  bool mActive : 1;
  // Guessed names, based on the runtime update dispatch.
  bool mUpdateWhileOccluded : 1;
  bool mUpdateDuringCinematicSkip : 1;

public:
  CEntityInfo(TAreaId aid, const rstl::vector< SConnection >& connections, bool active,
              TEditorId eid = kInvalidEditorId);
  CEntityInfo(const CEntityInfo&);
  ~CEntityInfo();

  TAreaId GetAreaId() const { return mAreaId; }
  const rstl::vector< SConnection >& GetConnectionList() const { return mConnections; }
  TEditorId GetEditorId() const { return mEditorId; }
  bool GetActive() const { return mActive; }
  bool GetUpdateWhileOccluded() const { return mUpdateWhileOccluded; }
  bool GetUpdateDuringCinematicSkip() const { return mUpdateDuringCinematicSkip; }
};

class CScriptMsg {
public:
  CScriptMsg()
  : m_unk(kInvalidUniqueId)
  , m_originator(kInvalidUniqueId)
  , m_id(kInvalidUniqueId)
  , m_msg(kSM_None)
  , m_state(kSS_InvalidState) {}

  CScriptMsg(TUniqueId unk, TUniqueId originator, TUniqueId id, EScriptObjectMessage msg,
             EScriptObjectState state)
  : m_unk(unk), m_originator(originator), m_id(id), m_msg(msg), m_state(state) {}

  TUniqueId GetUnk() const { return m_unk; }
  TUniqueId GetOriginator() const { return m_originator; }
  TUniqueId GetId() const { return m_id; }
  EScriptObjectMessage GetMessage() const { return m_msg; }
  EScriptObjectState GetState() const { return m_state; }

  void SetMessage(EScriptObjectMessage msg) { m_msg = msg; }

public:
  TUniqueId m_unk;
  TUniqueId m_originator;
  TUniqueId m_id;
  EScriptObjectMessage m_msg;
  EScriptObjectState m_state;
};

struct SLdrEditorProperties;
CEntityInfo& LdrToEntityInfo(CEntityInfo&, const SLdrEditorProperties&);

#endif // _CENTITYINFO
