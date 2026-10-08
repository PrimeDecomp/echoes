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
  kET_ScriptGuiWidget = 11,    // Guessed name; REL ScriptGui base, derives CEntity.
  kET_ScriptPathMeshCtrl = 10, // Guessed class name; native path-mesh obstruction controller.
  kET_ScriptSequenceTimer = 12,
  kET_BallCamera = 13,
  kET_Bomb = 14,
  kET_BouncingBomb = 15, // Target-derived class tag.
  kET_BouncyGrenade = 16,
  kET_CinematicCamera = 17,
  kET_CollisionActor = 18,
  kET_EnergyProjectile = 19,
  kET_LightComboProjectile = 20, // Guessed class name.
  kET_Explosion = 22,
  kET_FirstPersonCamera = 23,
  kET_FixedCamera = 24, // Guessed name; runtime fixed camera.
  kET_FishCloud = 25,   // Guessed name; FishCloud REL TypesMatch tag.
  kET_GameLight = 26,
  kET_HomingBlob = 27, // Guessed name; Dark impact's multi-target particle weapon.
  kET_HUDBillboardEffect = 28,
  kET_IngPuddle = 29, // Target-derived class tag.
  kET_PathCamera = 31,
  kET_Player = 32,
  kET_GameHint = 33, // Guessed name.
  kET_ScriptActor = 34,
  kET_ScriptActorKeyframe = 35,
  kET_ScriptActorRotate = 36,
  kET_ScriptAIHint = 37, // Id of the Wii SEL's CScriptAIHint; cast target from CAiWaypointList.
  kET_ScriptAiJumpPoint = 38,
  kET_ScriptAIWaypoint = 39, // Wii SEL class name; native cast/table/loader correspondence.
  kET_ScriptCameraHint = 40,
  kET_ScriptCameraShaker = 41,
  kET_ScriptCameraPitch = 42, // Guessed name.
  kET_ScriptCameraWaypoint = 43,
  kET_ScriptCamera = 44,
  kET_ScriptColorModulate = 45,
  kET_ScriptControlHint = 46, // Guessed name; command-filter hint.
  kET_ScriptCounter = 47,
  kET_ScriptCoverPoint = 48,
  kET_ScriptDamageableTrigger = 49,
  kET_ScriptDamageableTriggerOrientated = 50, // Guessed name.
  kET_DarkSamusBattleStage = 51,
  kET_ScriptDebris = 52,
  kET_ScriptDestructibleBarrier = 53, // Guessed name.
  kET_ScriptDistanceFog = 54,         // Guessed name; Prime has CScriptDistanceFog.
  kET_ScriptDock = 55,
  kET_ScriptDoor = 56,
  kET_ScriptDynamicLight = 57,
  kET_ScriptEffect = 58,
  kET_ScriptGrapplePoint = 59,
  kET_ScriptGuiMenu = 60,   // Guessed name.
  kET_ScriptGuiScreen = 61, // Guessed name.
  kET_ScriptGuiSlider = 62, // Guessed name.
  kET_ScriptHUDHint = 63,   // Guessed name; HUD texture marker.
  kET_ScriptLayerController = 64,
  kET_ScriptPathCamera = 65,
  kET_ScriptPickup = 66,
  kET_ScriptPickupGenerator = 67,
  kET_ScriptPlayerHint = 68,
  kET_ScriptPlayerProxy = 69,
  kET_ScriptPointOfInterest = 71, // Guessed name; Prime has CScriptPointOfInterest.
  kET_ScriptPlatform = 70,
  kET_ScriptPortalTransition = 72,
  kET_ScriptRelay = 73,
  kET_ScriptRepulsor = 74,
  kET_ScriptRiftPortal = 75, // Target-derived class tag.
  kET_ScriptRoomAcoustics = 76,
  kET_ScriptSound = 77,
  kET_ScriptSoundModifier = 78, // Guessed name.
  kET_ScriptSpawnPoint = 79,
  kET_ScriptSpecialFunction = 80,
  kET_ScriptSpiderBallAttractionSurface = 81,
  kET_ScriptSpiderBallWaypoint = 82,
  kET_ScriptSpindleCamera = 83,
  kET_ScriptStreamedMusic = 84,
  kET_ScriptSurfaceCamera = 85, // Guessed name; script surface-camera provider.
  kET_ScriptTeamAi = 88,
  kET_ScriptSwitch = 86,
  kET_ScriptTargetingPoint = 87,
  kET_ScriptTextPane = 89,     // Target-derived class tag.
  kET_ScriptTimeKeyframe = 90, // Target-derived class tag.
  kET_ScriptTimer = 91,
  kET_ScriptTrigger = 92,
  kET_ScriptTriggerEllipsoid = 93,
  kET_ScriptTriggerOrientated = 94, // Guessed name; oriented-box trigger.
  kET_ScriptSafeZone = 95,          // Guessed name; REL ScriptSafeZone.
  kET_ScriptVisorFlare = 96,
  kET_ScriptWater = 97,
  kET_ScriptWorldTeleporter = 98,
  kET_SnakeWeedSwarm = 99, // Native REL type query; class spelling corroborated by Wii export.
  kET_SpindleCamera = 100,
  kET_SurfaceCamera = 101, // Guessed name; runtime surface camera.
  kET_SwarmBasics = 102,   // Native TypesMatch tag, correlated with swarm consumers.
  kET_FlyerSwarm = 103,    // Native REL TypesMatch tag; parent is the SwarmBasics tag.
  kET_WallCrawler = 104,   // Target-derived class tag.
  kET_MetareeSwarm = 106,  // Native REL TypesMatch tag; parent is the SwarmBasics tag.
  kET_IngBlobSwarm = 107, // Native REL TypesMatch tag; parent is the SwarmBasics tag.
  kET_PlantScarabSwarm = 108, // Native REL TypesMatch tag; parent is the SwarmBasics tag.
  kET_BeamProjectile = 109,
  kET_PlasmaProjectile = 110,
  kET_DarkSamus = 111,
  kET_Metaree = 121,                   // Target-derived class tag.
  kET_Metroid = 122,                   // Target-derived class tag.
  kET_BabyMetroid = 123,               // Target-derived class tag.
  kET_Parasite = 124,                  // Target-derived class tag.
  kET_PillBug = 125,                   // Target-derived class tag.
  kET_Puffer = 126,                    // Target-derived class tag.
  kET_Ripper = 128,                    // Target-derived class tag.
  kET_Sandworm = 130,                  // Target-derived class tag.
  kET_SandwormEye = 131,               // Target-derived class tag.
  kET_SpacePirate = 132,               // Target-derived class tag.
  kET_ScriptPlayerTurret = 138,        // Target-derived class tag; turret-HUD REL dispatch target.
  kET_GunTurretBase = 139,             // Target-derived class tag.
  kET_GunTurretTop = 140,              // Target-derived class tag.
  kET_Kralee = 141, // Target-derived class tag.
  kET_Glowbug = 142, // Target-derived class tag.
  kET_WallWalker = 150,                // Target-derived class tag.
  kET_Shredder = 151, // Target-derived class tag.
  kET_TargetableProjectile = 152,      // Target-derived class tag.
  kET_StoneToad = 154, // Target-derived class tag.
  kET_ScriptFrontEndDataNetwork = 155, // Target-derived class tag.
  kET_PowerBomb = 156,
  kET_Krocuss = 157, // Target-derived class tag.
  kET_PuddleSpore = 159, // Target-derived class tag.
  kET_ScriptForgottenObject = 160,
};

enum EScriptObjectState {
  kSS_Active = 0x41435456,
  kSS_Arrived = 0x41525256,
  kSS_Inactive = 0x49435456,
  kSS_Entered = 0x454e5452,
  kSS_Inside = 0x494e5344,
  kSS_Exited = 0x45584954,
  kSS_Footstep = 0x464f4f54,
  kSS_Zero = 0x5a45524f,
  kSS_NonZero = 0x215a4552,
  kSS_DefaultState = 0x44465354,
  kSS_MaxReached = 0x4d415852,
  kSS_ScanStart = 0x4553434e,
  kSS_ScanProcessing = 0x4253434e,
  kSS_ScanDone = 0x53434e44,
  kSS_Patrol = 0x5054524c,
  kSS_Attack = 0x4154544b,
  kSS_Retreat = 0x52545254, // Prime-correlated name; cover point's retreat connection.
  kSS_Play = 0x504c4159,
  kSS_Connect = 0x434f4e4e,
  // Guessed names from DKCR HD; first/second elevator-camera connections in Echoes.
  kSS_InFront = 0x58494e46,
  kSS_InBack = 0x58494e42,
  kSS_Slave = 0x534c4156,
  kSS_Opened = 0x4f50454e,
  kSS_Closed = 0x434c4f53,
  kSS_CameraTarget = 0x43544754,
  kSS_CameraPath = 0x43505448,
  kSS_CameraPlayer = 0x43504c52,
  kSS_CameraTime = 0x4354494d,
  kSS_UnFrozen = 0x5546525a,
  kSS_Dead = 0x44454144,
  kSS_DeathRattle = 0x5241544c,         // Guessed Prime-correlated name; native damage-death state.
  kSS_AboutToMassivelyDie = 0x52445545, // Guessed Prime name; native pre-massive-death state.
  // Guessed DKCR HD names; native Patterned massive-damage connections establish the tags.
  kSS_XDamage = 0x58444d47,
  kSS_DarkXDamage = 0x44524b58,
  kSS_IceXDamage = 0x49444d47, // Guessed DKCR HD name; native massive frozen death tag.
  // Guessed names; coin-denomination tags (100 and 50) sent beside the DAMG/XDMG/IDMG family.
  kSS_BIDG = 0x42494447,
  kSS_BXDG = 0x42584447,
  kSS_Generate = 0x47454e52,
  kSS_GeneratorConnection = 0x47524e54, // Guessed name; generator-to-spawned-object connections.
  kSS_ReflectedDamage = 0x52454644,
  kSS_Damage = 0x44414d47,         // Guessed name; DAMG damage notification state.
  kSS_ResistedDamage = 0x52455344, // Guessed DKCR HD name; native resisted-damage branch.
  // Guessed names; native weapon-to-damage-state dispatch establishes each tag.
  kSS_PowerDamage = 0x44505752,
  kSS_DarkDamage = 0x4444524b,
  kSS_LightDamage = 0x444c4754,
  kSS_AnnihilatorDamage = 0x44414e4e,
  kSS_BombDamage = 0x44424d42,
  kSS_PowerBombDamage = 0x4450424d,
  kSS_MissileDamage = 0x444d4953,
  kSS_BoostBallDamage = 0x4442414c,
  kSS_CannonBallDamage = 0x4443414e,
  kSS_ScrewAttackDamage = 0x44534357,
  kSS_PhazonDamage = 0x4450485a,
  kSS_AIDamage = 0x44424149,
  kSS_PoisonWaterDamage = 0x44505754,
  kSS_LavaDamage = 0x444c4156,
  kSS_HeatDamage = 0x44484f54,
  kSS_ColdDamage = 0x44434c44,
  kSS_AreaDarkDamage = 0x44414452,
  kSS_AreaLightDamage = 0x44414c47,
  kSS_UnknownSourceDamage = 0x44554e53,
  kSS_InheritBounds = 0x49424e44,
  kSS_Modify = 0x4d444659,          // Prime name; fish cloud modifier connections.
  kSS_InternalState00 = 0x49533030, // Guessed name: base of the ten counter-condition states.
  kSS_InternalState01 = 0x49533031, // Guessed name
  // Guessed names; portal-transition connections use these internal states.
  kSS_InternalState03 = 0x49533033,
  kSS_InternalState04 = 0x49533034,
  kSS_InternalState05 = 0x49533035,
  kSS_InternalState06 = 0x49533036,
  kSS_ScanSource = 0x53434e53,
  // Guessed names; GUI widget/menu states sent by the ScriptGui REL.
  kSS_InternalState02 = 0x49533032,
  kSS_InternalState07 = 0x49533037,
  kSS_InternalState08 = 0x49533038,
  kSS_InternalState09 = 0x49533039,
  kSS_InternalState10 = 0x49533130,
  kSS_InternalState11 = 0x49533131,
  kSS_InternalState12 = 0x49533132,
  kSS_InternalState13 = 0x49533133,
  kSS_InternalState14 = 0x49533134,
  kSS_InternalState15 = 0x49533135,
  kSS_InternalState16 = 0x49533136,
  kSS_InternalState17 = 0x49533137,
  kSS_InternalState18 = 0x49533138,
  kSS_InternalState19 = 0x49533139,
  kSS_Locked = 0x4c4f434b,
  kSS_Unlocked = 0x554c434b,
  kSS_Frozen = 0x4652455a,
  kSS_APRC = 0x41505243, // Native GUI accept-press tag, sent before kSS_PressA.
  kSS_PressA = 0x50525341,
  kSS_PressB = 0x50525342,
  kSS_PressX = 0x50525358,
  kSS_PressY = 0x50525359,
  kSS_PressZ = 0x5052535a,
  kSS_PressStart = 0x50525354,
  kSS_Left = 0x4c454654,
  kSS_Right = 0x52474854,
  kSS_AIS1 = 0x41495331, // Native GUI connection tags; meaning unresolved.
  kSS_AIS2 = 0x41495332,
  kSS_AIS3 = 0x41495333,
  kSS_DGNR = 0x44474e52, // Native save-screen failure tag; meaning unresolved.
  kSS_Up = 0x55502020,
  kSS_Down = 0x444f574e, // Guessed name; destructible barrier finished lowering.
  kSS_GRNT = 0x47524e54, // Destructible barrier: generator for falling sections.
  kSS_Approach = 0x41505243,
  kSS_InvalidState = 0xffffffff,
};

enum EScriptObjectMessage {
  kSM_Action = 0x4143544e,
  kSM_Arrived = 0x41525256, // Guessed name; sent to a waypoint when its follower reaches it.
  kSM_Next = 0x4e455854,
  kSM_Start = 0x53545254,
  kSM_Stop = 0x53544f50,
  kSM_Play = 0x504c4159,
  kSM_Load = 0x4c4f4144,
  kSM_Unload = 0x554c4f44,
  kSM_Activate = 0x41435456,
  kSM_Alert = 0x414c5254, // Guessed Prime name; sets the Metroid alert flag.
  kSM_Deactivate = 0x44435456,
  kSM_ToggleActive = 0x54435456,
  kSM_SetToZero = 0x5a45524f,
  kSM_SetToMax = 0x534d4158,
  kSM_SetOriginator = 0x534f5247,
  kSM_ClearOriginator = 0x434f5247,
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
  kSM_Left = 0x4c454654, // Guessed name; shows the grapple on a player actor.
  kSM_Kill = 0x4b494c4c,
  kSM_InternalMessage00 = 0x494d3030,
  kSM_InternalMessage01 = 0x494d3031,
  kSM_InternalMessage02 = 0x494d3032, // Guessed name; GUI menu item-state refresh.
  kSM_InternalMessage03 = 0x494d3033, // Guessed name.
  kSM_InternalMessage04 = 0x494d3034, // Guessed name.
  kSM_InternalMessage05 = 0x494d3035, // Guessed name.
  kSM_InternalMessage06 = 0x494d3036, // Guessed name.
  kSM_InternalMessage07 = 0x494d3037, // Guessed name.
  kSM_InternalMessage08 = 0x494d3038, // Guessed name.
  kSM_InternalMessage09 = 0x494d3039, // Guessed name.
  kSM_InternalMessage10 = 0x494d3130, // Guessed name.
  kSM_InternalMessage11 = 0x494d3131, // Guessed name.
  kSM_InternalMessage12 = 0x494d3132, // Guessed name.
  kSM_InternalMessage13 = 0x494d3133, // Guessed name.
  kSM_InternalMessage14 = 0x494d3134, // Guessed name.
  kSM_Escape = 0x45534350,            // Guessed name; clears a GUI widget's controllers.

  // Guessed lifecycle names from DKCR HD, corroborated by Echoes consumers.
  kSM_Create = 0x58435254,
  kSM_XEPZ = 0x5845505a,  // Guessed name; Phazon-pool entry.
  kSM_XIPZ = 0x5849505a,  // Guessed name; Phazon-pool update.
  kSM_XXPZ = 0x5858505a,  // Guessed name; Phazon-pool exit.
  kSM_XENZ = 0x58454e5a,  // Guessed name; makes a flagged bouncy grenade explode.
  kSM_XEXZ = 0x5845585a,  // Guessed name; paired with XENZ on safe-zone exit.
  kSM_Clear = 0x58434c52, // Guessed DKCR HD name; clears an effect's particles.
  kSM_AreaLoaded = 0x58414c44,
  kSM_WorldLoaded = 0x58574c44,
  kSM_Delete = 0x5844454c,
  kSM_XENF = 0x58454e46,   // Native fluid-entry tag.
  kSM_XINF = 0x58494e46,   // Native fluid-update tag.
  kSM_XEXF = 0x58455846,   // Native fluid-exit tag.
  kSM_XINS = 0x58494e53,   // Guessed name; sent to the player when an ice impact touches them.
  kSM_Damage = 0x58444d47, // Guessed DKCR HD name; damage notification.
  kSM_ResistedDamage = 0x58524447, // Guessed DKCR HD name; native resisted-damage branch.
  kSM_XHIT = 0x58484954,
  kSM_XAOV = 0x58414f56,             // Native projectile visor-impact tag.
  kSM_AIUpdateDisabled = 0x58415544, // Guessed DKCR HD name; patterned update disabled.
  kSM_XXDG = 0x58584447,
  kSM_LandOnNotFloor = 0x5846414c,
  kSM_Falling = 0x584f4646,
  kSM_Launching = 0x584c4155,            // Guessed DKCR HD name; jump/hurled launch notification.
  kSM_Landed = 0x584c4e44,               // Guessed DKCR HD name; landing notification.
  kSM_LandedOnStaticGround = 0x584c5347, // Guessed DKCR HD name; native static-ground notification.
  // Guessed Prime names, correlated with the native player message handler.
  kSM_OnIceSurface = 0x584f4e49,
  kSM_OnMudSlowSurface = 0x584f4e4f,
  kSM_OnNormalSurface = 0x584f4e44,
  kSM_AddPlatformRider = 0x584f4e50,

  kSM_None = 0xffffffff,
};

struct SConnection {
  EScriptObjectState state;
  EScriptObjectMessage msg;
  TEditorId objId;

  SConnection(EScriptObjectState state, EScriptObjectMessage msg, TEditorId id)
  : state(state), msg(msg), objId(id) {}
};
namespace rstl {
RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(SConnection)
} // namespace rstl

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

  TAreaId GetAreaId() const { return mAreaId; }
  const rstl::vector< SConnection >& GetConnectionList() const { return mConnections; }
  TEditorId GetEditorId() const { return mEditorId; }
  bool GetActive() const { return mActive; }
  void SetActive(bool active) { mActive = active; }
  bool GetUpdateWhileOccluded() const { return mUpdateWhileOccluded; }
  void SetUpdateWhileOccluded(bool update) { mUpdateWhileOccluded = update; }
  bool GetUpdateDuringCinematicSkip() const { return mUpdateDuringCinematicSkip; }
  void SetUpdateDuringCinematicSkip(bool update) { mUpdateDuringCinematicSkip = update; }
};

class CScriptMsg {
public:
  CScriptMsg()
  : mSenderId(kInvalidUniqueId)
  , m_originator(kInvalidUniqueId)
  , m_id(kInvalidUniqueId)
  , m_msg(kSM_None)
  , m_state(kSS_InvalidState) {}

  CScriptMsg(TUniqueId sender, TUniqueId id, EScriptObjectMessage msg,
             TUniqueId originator = kInvalidUniqueId, EScriptObjectState state = kSS_InvalidState)
  : mSenderId(sender), m_originator(originator), m_id(id), m_msg(msg), m_state(state) {}

  TUniqueId GetSenderId() const { return mSenderId; } // Guessed name; native sender UID.
  TUniqueId GetOriginator() const { return m_originator; }
  TUniqueId GetId() const { return m_id; }
  EScriptObjectMessage GetMessage() const { return m_msg; }
  EScriptObjectState GetState() const { return m_state; }

  void SetMessage(EScriptObjectMessage msg) { m_msg = msg; }

public:
  TUniqueId mSenderId;
  TUniqueId m_originator;
  TUniqueId m_id;
  EScriptObjectMessage m_msg;
  EScriptObjectState m_state;
};

struct SLdrEditorProperties;
CEntityInfo& LdrToEntityInfo(CEntityInfo&, const SLdrEditorProperties&);

#endif // _CENTITYINFO
