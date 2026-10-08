#ifndef _SCRIPTLOADERREL
#define _SCRIPTLOADERREL

#include "MetroidPrime/ScriptLoader.hpp"

#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/reserved_vector.hpp"
#include "rstl/string.hpp"

class CVector3f;
class CDamageInfo;
class CFinalInput;
class CSpacePirate;
class CMetroid;
class CSplitterMainChassis;
class CEffect;
class CGenDescription;
class CPatterned;
class CRagDoll;

// DOL stand-ins for loaders that live in RELs: each forwards to the callback record the
// loaded REL registered. A monolithic build links the REL sources into the DOL instead, so
// REL_ENTRY binds straight to the real loader.
#ifdef MONOLITHIC
#define REL_ENTRY(name) name
#else
#define REL_ENTRY(name) RelProxy_##name
#endif

CEntity* REL_ENTRY(LoadAIMannedTurret)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadAtomicAlpha)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadAtomicBeta)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadBacteriaSwarm)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadBlogg)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadBrizgee)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadCannonBall)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadChozoGhost)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadCoin)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadCommandPirate)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadCrystallite)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadDarkCommando)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadDarkSamus)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadDarkSamusBattleStage)(CStateManager& mgr, CInputStream& input,
                                             CEntityInfo& info);
CEntity* REL_ENTRY(LoadDarkTrooper)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadDestructibleBarrier)(CStateManager& mgr, CInputStream& input,
                                            CEntityInfo& info);
CEntity* REL_ENTRY(LoadDigitalGuardian)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadDigitalGuardianHead)(CStateManager& mgr, CInputStream& input,
                                            CEntityInfo& info);
CEntity* REL_ENTRY(LoadElitePirate)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadEmperorIngStage1)(CStateManager& mgr, CInputStream& input,
                                         CEntityInfo& info);
CEntity* REL_ENTRY(LoadEmperorIngStage2Tentacle)(CStateManager& mgr, CInputStream& input,
                                                 CEntityInfo& info);
CEntity* REL_ENTRY(LoadEmperorIngStage3)(CStateManager& mgr, CInputStream& input,
                                         CEntityInfo& info);
CEntity* REL_ENTRY(LoadEyeBall)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadFishCloud)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadFishCloudModifier)(CStateManager& mgr, CInputStream& input,
                                          CEntityInfo& info);
CEntity* REL_ENTRY(LoadFlyerSwarm)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadFlyingPirate)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadFogOverlay)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadForgottenObject)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadFrontEndDataNetwork)(CStateManager& mgr, CInputStream& input,
                                            CEntityInfo& info);
CEntity* REL_ENTRY(LoadGlowbug)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadGrenchler)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadGuiMenu)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadGuiPlayerJoinManager)(CStateManager& mgr, CInputStream& input,
                                             CEntityInfo& info);
CEntity* REL_ENTRY(LoadGuiScreen)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadGuiSlider)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadGuiWidget)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadGunTurretBase)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadGunTurretTop)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadIngBlobSwarm)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadIngBoostBallGuardian)(CStateManager& mgr, CInputStream& input,
                                             CEntityInfo& info);
CEntity* REL_ENTRY(LoadIngPuddle)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadIngSnatchingSwarm)(CStateManager& mgr, CInputStream& input,
                                          CEntityInfo& info);
CEntity* REL_ENTRY(LoadIngSpaceJumpGuardian)(CStateManager& mgr, CInputStream& input,
                                             CEntityInfo& info);
CEntity* REL_ENTRY(LoadIngSpiderBallGuardian)(CStateManager& mgr, CInputStream& input,
                                              CEntityInfo& info);
CEntity* REL_ENTRY(LoadIngs)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadKralee)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadKrocuss)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadLumite)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadMediumIng)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadMetaree)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadMetareeSwarm)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadMetroidAlpha)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadMinorIng)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadMysteryFlyer)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadOctopedeSegment)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadParasite)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadPillBug)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadPlantScarabSwarm)(CStateManager& mgr, CInputStream& input,
                                         CEntityInfo& info);
CEntity* REL_ENTRY(LoadPlayerActor)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadPlayerController)(CStateManager& mgr, CInputStream& input,
                                         CEntityInfo& info);
CEntity* REL_ENTRY(LoadPlayerTurret)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadPuddleSpore)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadPuffer)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadRezbit)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadRiftPortal)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadRipper)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadRsfAudio)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadRubiksPuzzle)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadSafeZone)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadSafeZoneCrystal)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadSandBoss)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadSandworm)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadShredder)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadShrieker)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadSkyRipple)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadSnakeWeedSwarm)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadSpacePirate)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadSpankWeed)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadSplinter)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadSplitterCommandModule)(CStateManager& mgr, CInputStream& input,
                                              CEntityInfo& info);
CEntity* REL_ENTRY(LoadSplitterMainChassis)(CStateManager& mgr, CInputStream& input,
                                            CEntityInfo& info);
CEntity* REL_ENTRY(LoadSporbBase)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadSporbNeedle)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadSporbProjectile)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadSporbTop)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadStoneToad)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadStreamedMovie)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadSwampBossStage1)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadSwampBossStage2)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadTryclops)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadWallWalker)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* REL_ENTRY(LoadWispTentacle)(CStateManager& mgr, CInputStream& input, CEntityInfo& info);

struct SGeomBlobV2_FuncPtrs {
  // Guessed member name; compatible base return type.
  CEffect* (*mFactory)(const TLockedToken< CGenDescription >& token, TUniqueId uid, TAreaId area,
                       bool active, const rstl::string& name, const CTransform4f& transform,
                       TUniqueId relatedId, uint flags);
};
CHECK_SIZEOF(SGeomBlobV2_FuncPtrs, 0x4)
void SetSGeomBlobV2_FuncPtrs(SGeomBlobV2_FuncPtrs* callbacks);

// Guessed record and member names; compatible factory signature.
struct SSurfaceParticleEffect_FuncPtrs {
  CEffect* (*mFactory)(const TLockedToken< CGenDescription >& token, TUniqueId uid, TAreaId area,
                       bool active, const rstl::string& name, const CTransform4f& transform,
                       TUniqueId ignoredCollisionId, uint flags);
};
CHECK_SIZEOF(SSurfaceParticleEffect_FuncPtrs, 0x4)
void SetSSurfaceParticleEffect_FuncPtrs(SSurfaceParticleEffect_FuncPtrs* callbacks);

struct SPirateRagDoll_FuncPtrs {
  // Guessed member name; compatible base return type.
  CRagDoll* (*mFactory)(CStateManager& mgr, CPatterned* actor, ushort soundId, uint flags,
                        float first, float second,
                        const rstl::reserved_vector< float, 14 >& values);
};
CHECK_SIZEOF(SPirateRagDoll_FuncPtrs, 0x4)
void SetSPirateRagDoll_FuncPtrs(SPirateRagDoll_FuncPtrs* callbacks);

// The native record's callback signature is unresolved.
struct SSwarmBasics_FuncPtrs;
void SetSSwarmBasics_FuncPtrs(SSwarmBasics_FuncPtrs* callbacks);

struct SIngSnatchingSwarm_FuncPtrs {
  FScriptLoader mLoadIngSnatchingSwarm; // Guessed member name.
};
CHECK_SIZEOF(SIngSnatchingSwarm_FuncPtrs, 0x4)
void SetSIngSnatchingSwarm_FuncPtrs(SIngSnatchingSwarm_FuncPtrs* callbacks);

struct SAtomicAlpha_FuncPtrs {
  FScriptLoader mLoadAtomicAlpha; // Guessed member name.
};
CHECK_SIZEOF(SAtomicAlpha_FuncPtrs, 0x4)
void SetSAtomicAlpha_FuncPtrs(SAtomicAlpha_FuncPtrs* callbacks);

struct SRipper_FuncPtrs {
  FScriptLoader mLoadRipper; // Guessed member name.
};
CHECK_SIZEOF(SRipper_FuncPtrs, 0x4)
void SetSRipper_FuncPtrs(SRipper_FuncPtrs* callbacks);

struct SPuffer_FuncPtrs {
  FScriptLoader mLoadPuffer; // Guessed member name.
};
CHECK_SIZEOF(SPuffer_FuncPtrs, 0x4)
void SetSPuffer_FuncPtrs(SPuffer_FuncPtrs* callbacks);

struct SMetaree_FuncPtrs {
  FScriptLoader mLoadMetaree; // Guessed member name.
};
CHECK_SIZEOF(SMetaree_FuncPtrs, 0x4)
void SetSMetaree_FuncPtrs(SMetaree_FuncPtrs* callbacks);

struct SRiftPortal_FuncPtrs {
  FScriptLoader mLoadRiftPortal; // Guessed member name.
};
CHECK_SIZEOF(SRiftPortal_FuncPtrs, 0x4)
void SetSRiftPortal_FuncPtrs(SRiftPortal_FuncPtrs* callbacks);

struct SPlayerController_FuncPtrs {
  FScriptLoader mLoadPlayerController; // Guessed member name.
};
CHECK_SIZEOF(SPlayerController_FuncPtrs, 0x4)
void SetSPlayerController_FuncPtrs(SPlayerController_FuncPtrs* callbacks);

struct SWallWalker_FuncPtrs {
  FScriptLoader mLoadWallWalker; // Guessed member name.
};
CHECK_SIZEOF(SWallWalker_FuncPtrs, 0x4)
void SetSWallWalker_FuncPtrs(SWallWalker_FuncPtrs* callbacks);

struct SCannonBall_FuncPtrs {
  FScriptLoader mLoadCannonBall; // Guessed member name.
};
CHECK_SIZEOF(SCannonBall_FuncPtrs, 0x4)
void SetSCannonBall_FuncPtrs(SCannonBall_FuncPtrs* callbacks);

struct SMetroid_FuncPtrs {
  // Guessed member names; CMetroid is the module class.
  FScriptLoader mLoadMetroid;
  void (CMetroid::*mOnDockTouch)(CStateManager&);
};
CHECK_SIZEOF(SMetroid_FuncPtrs, 0x10)
void SetSMetroid_FuncPtrs(SMetroid_FuncPtrs* callbacks);

struct SSplitterMainChassis_FuncPtrs {
  // Guessed member names.
  FScriptLoader mLoadMainChassis;
  FScriptLoader mLoadCommandModule;
  void (CSplitterMainChassis::*mAutoDestruct)(float);
};
CHECK_SIZEOF(SSplitterMainChassis_FuncPtrs, 0x14)
void SetSSplitterMainChassis_FuncPtrs(SSplitterMainChassis_FuncPtrs* callbacks);

struct SSpacePirate_FuncPtrs {
  // Guessed member names.
  FScriptLoader mLoadSpacePirate;
  bool (CSpacePirate::*mAttachActor)(TUniqueId);
  void (CSpacePirate::*mDetachActor)();
};
CHECK_SIZEOF(SSpacePirate_FuncPtrs, 0x1c)
void SetSSpacePirate_FuncPtrs(SSpacePirate_FuncPtrs* callbacks);

struct SKralee_FuncPtrs {
  FScriptLoader mLoadKralee; // Guessed member name.
};
CHECK_SIZEOF(SKralee_FuncPtrs, 0x4)
void SetSKralee_FuncPtrs(SKralee_FuncPtrs* callbacks);

struct SParasite_FuncPtrs {
  // Guessed member names.
  FScriptLoader mLoadParasite;
  FScriptLoader mLoadBrizgee;
  FScriptLoader mLoadCrystallite;
};
CHECK_SIZEOF(SParasite_FuncPtrs, 0xc)
void SetSParasite_FuncPtrs(SParasite_FuncPtrs* callbacks);

struct SPillBug_FuncPtrs {
  FScriptLoader mLoadPillBug; // Guessed member name.
};
CHECK_SIZEOF(SPillBug_FuncPtrs, 0x4)
void SetSPillBug_FuncPtrs(SPillBug_FuncPtrs* callbacks);

struct SSporb_FuncPtrs {
  // Guessed member names.
  FScriptLoader mLoadBase;
  FScriptLoader mLoadProjectile;
  FScriptLoader mLoadNeedle;
  FScriptLoader mLoadTop;
};
CHECK_SIZEOF(SSporb_FuncPtrs, 0x10)
void SetSSporb_FuncPtrs(SSporb_FuncPtrs* callbacks);

class CSandworm;

struct SSandworm_FuncPtrs {
  // Guessed member names.
  FScriptLoader mLoadSandworm;
  CVector3f (CSandworm::*mGetRadarPointPosition)(int index) const;
  int (CSandworm::*mGetRadarPointCount)() const;
};
CHECK_SIZEOF(SSandworm_FuncPtrs, 0x1c)
void SetSSandworm_FuncPtrs(SSandworm_FuncPtrs* callbacks);

struct SCommandoPirate_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SCommandoPirate_FuncPtrs, 0x4)
void SetSCommandoPirate_FuncPtrs(SCommandoPirate_FuncPtrs* callbacks);

struct SDarkCommando_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SDarkCommando_FuncPtrs, 0x4)
void SetSDarkCommando_FuncPtrs(SDarkCommando_FuncPtrs* callbacks);

struct SDarkSamus_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SDarkSamus_FuncPtrs, 0x4)
void SetSDarkSamus_FuncPtrs(SDarkSamus_FuncPtrs* callbacks);

struct SIng_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SIng_FuncPtrs, 0x4)
void SetSIng_FuncPtrs(SIng_FuncPtrs* callbacks);

struct SIngSpaceJumpGuardian_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SIngSpaceJumpGuardian_FuncPtrs, 0x4)
void SetSIngSpaceJumpGuardian_FuncPtrs(SIngSpaceJumpGuardian_FuncPtrs* callbacks);

struct SDigitalGuardian_FuncPtrs {
  // Guessed member names.
  FScriptLoader mLoadDigitalGuardian;
  FScriptLoader mLoadDigitalGuardianHead;
};
CHECK_SIZEOF(SDigitalGuardian_FuncPtrs, 0x8)
void SetSDigitalGuardian_FuncPtrs(SDigitalGuardian_FuncPtrs* callbacks);

struct SShredder_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SShredder_FuncPtrs, 0x4)
void SetSShredder_FuncPtrs(SShredder_FuncPtrs* callbacks);

struct SFrontEndDataNetwork_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SFrontEndDataNetwork_FuncPtrs, 0x4)
void SetSFrontEndDataNetwork_FuncPtrs(SFrontEndDataNetwork_FuncPtrs* callbacks);

struct SStoneToad_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SStoneToad_FuncPtrs, 0x4)
void SetSStoneToad_FuncPtrs(SStoneToad_FuncPtrs* callbacks);

struct SCoin_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SCoin_FuncPtrs, 0x4)
void SetSCoin_FuncPtrs(SCoin_FuncPtrs* callbacks);

struct SKrocuss_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SKrocuss_FuncPtrs, 0x4)
void SetSKrocuss_FuncPtrs(SKrocuss_FuncPtrs* callbacks);

struct SAIMannedTurret_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SAIMannedTurret_FuncPtrs, 0x4)
void SetSAIMannedTurret_FuncPtrs(SAIMannedTurret_FuncPtrs* callbacks);

struct SEmperorIngStage1_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SEmperorIngStage1_FuncPtrs, 0x4)
void SetSEmperorIngStage1_FuncPtrs(SEmperorIngStage1_FuncPtrs* callbacks);

struct SOctapedeSegment_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SOctapedeSegment_FuncPtrs, 0x4)
void SetSOctapedeSegment_FuncPtrs(SOctapedeSegment_FuncPtrs* callbacks);

struct SMetareeSwarm_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SMetareeSwarm_FuncPtrs, 0x4)
void SetSMetareeSwarm_FuncPtrs(SMetareeSwarm_FuncPtrs* callbacks);

struct SGuiWidget_FuncPtrs {
  // Guessed member names.
  FScriptLoader mLoadGuiWidget;
  FScriptLoader mLoadGuiScreen;
  FScriptLoader mLoadGuiSlider;
  FScriptLoader mLoadGuiMenu;
  FScriptLoader mLoadGuiPlayerJoinManager;
};
CHECK_SIZEOF(SGuiWidget_FuncPtrs, 0x14)
void SetSGuiWidget_FuncPtrs(SGuiWidget_FuncPtrs*);

// These CEntity member pointers are type-erased REL subtype bridges, not CEntity methods.
struct SSafeZone_FuncPtrs {
  // Guessed member names.
  FScriptLoader mLoadSafeZone;
  FScriptLoader mLoadSafeZoneCrystal;
  void (CEntity::*mApplyRenderEffect)(CStateManager&);
};
CHECK_SIZEOF(SSafeZone_FuncPtrs, 0x14)
void SetSSafeZone_FuncPtrs(SSafeZone_FuncPtrs*);

struct SFishCloud_FuncPtrs {
  // Guessed member names.
  FScriptLoader mLoadFishCloud;
  FScriptLoader mLoadFishCloudModifier;
};
CHECK_SIZEOF(SFishCloud_FuncPtrs, 0x8)
void SetSFishCloud_FuncPtrs(SFishCloud_FuncPtrs*);

struct SSnakeWeedSwarm_FuncPtrs {
  // Guessed member names; member-pointer owners use the type-erased bridge convention.
  FScriptLoader mLoadSnakeWeedSwarm;
  void (CEntity::*mApplyRadiusDamage)(CVector3f, const CDamageInfo&, CStateManager&);
  void (CEntity::*mScareSnakeWeeds)(CStateManager&, const CVector3f&, float);
};
CHECK_SIZEOF(SSnakeWeedSwarm_FuncPtrs, 0x1c)
void SetSSnakeWeedSwarm_FuncPtrs(SSnakeWeedSwarm_FuncPtrs*);

struct SPlayerActor_FuncPtrs {
  // Guessed member names; member-pointer owner uses the type-erased bridge convention.
  FScriptLoader mLoadPlayerActor;
  void (CEntity::*mTouchModels)(CStateManager&);
};
CHECK_SIZEOF(SPlayerActor_FuncPtrs, 0x10)
void SetSPlayerActor_FuncPtrs(SPlayerActor_FuncPtrs*);

struct SPlayerTurret_FuncPtrs {
  // Guessed member names; member-pointer owners use the type-erased bridge convention.
  FScriptLoader mLoadPlayerTurret;
  CTransform4f (CEntity::*mGetCameraTransform)(CStateManager&);
  CTransform4f (CEntity::*mGetTurretTransform)(CStateManager&);
  void (CEntity::*mExitTurret)(CStateManager&);
  void (CEntity::*mProcessInput)(const CFinalInput&, CStateManager&);
  TUniqueId (CEntity::*mGetHullActorId)();
};
CHECK_SIZEOF(SPlayerTurret_FuncPtrs, 0x40)
void SetSPlayerTurret_FuncPtrs(SPlayerTurret_FuncPtrs*);

// Guessed descriptive names for the existing REL member bridges.
void SafeZone_ApplyRenderEffect(CEntity& entity, CStateManager& mgr);
void SnakeWeed_ApplyRadiusDamage(CEntity& entity, CVector3f position, const CDamageInfo& damage,
                                 CStateManager& mgr);
void PlayerActor_TouchModels(CEntity& entity, CStateManager& mgr);
CTransform4f PlayerTurret_GetCameraTransform(CEntity& entity, CStateManager& mgr);
CTransform4f PlayerTurret_GetTurretTransform(CEntity& entity, CStateManager& mgr);
void PlayerTurret_ExitTurret(CEntity& entity, CStateManager& mgr);
void PlayerTurret_ProcessInput(CEntity& entity, const CFinalInput& input, CStateManager& mgr);
TUniqueId PlayerTurret_GetHullActorId(CEntity& entity);

struct SScriptForgottenObject_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SScriptForgottenObject_FuncPtrs, 0x4)
void SetSScriptForgottenObject_FuncPtrs(SScriptForgottenObject_FuncPtrs*);

struct STweaks_FuncPtrs {
  // Guessed member names.
  void (*mLoadTweaks)(CInputStream&);
  void (*mCreateGlobals)();
  void (*mFreeTweaks)();
};
CHECK_SIZEOF(STweaks_FuncPtrs, 0xc)
void SetSTweaks_FuncPtrs(STweaks_FuncPtrs*);

// Guessed names; the Tweaks REL implements them and the proxies dispatch through its
// function table.
void REL_ENTRY(LoadTweaks)(CInputStream& input);
void REL_ENTRY(CreateTweakGlobals)();
void REL_ENTRY(FreeTweaks)();

struct SScriptRubiksPuzzle_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SScriptRubiksPuzzle_FuncPtrs, 0x4)
void SetSScriptRubiksPuzzle_FuncPtrs(SScriptRubiksPuzzle_FuncPtrs* callbacks);

struct SAtomicBeta_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SAtomicBeta_FuncPtrs, 0x4)
void SetSAtomicBeta_FuncPtrs(SAtomicBeta_FuncPtrs* callbacks);

struct SBacteriaSwarm_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SBacteriaSwarm_FuncPtrs, 0x4)
void SetSBacteriaSwarm_FuncPtrs(SBacteriaSwarm_FuncPtrs* callbacks);

struct SBlogg_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SBlogg_FuncPtrs, 0x4)
void SetSBlogg_FuncPtrs(SBlogg_FuncPtrs* callbacks);

struct SChozoGhost_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SChozoGhost_FuncPtrs, 0x4)
void SetSChozoGhost_FuncPtrs(SChozoGhost_FuncPtrs* callbacks);

struct SScriptDarkSamusBattleStage_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SScriptDarkSamusBattleStage_FuncPtrs, 0x4)
void SetSScriptDarkSamusBattleStage_FuncPtrs(SScriptDarkSamusBattleStage_FuncPtrs* callbacks);

struct SDarkTrooper_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SDarkTrooper_FuncPtrs, 0x4)
void SetSDarkTrooper_FuncPtrs(SDarkTrooper_FuncPtrs* callbacks);

struct SDestructibleBarrier_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SDestructibleBarrier_FuncPtrs, 0x4)
void SetSDestructibleBarrier_FuncPtrs(SDestructibleBarrier_FuncPtrs* callbacks);

struct SElitePirate_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SElitePirate_FuncPtrs, 0x4)
void SetSElitePirate_FuncPtrs(SElitePirate_FuncPtrs* callbacks);

struct SEmperorIngStage2Tentacle_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SEmperorIngStage2Tentacle_FuncPtrs, 0x4)
void SetSEmperorIngStage2Tentacle_FuncPtrs(SEmperorIngStage2Tentacle_FuncPtrs* callbacks);

struct SEmperorIngStage3_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SEmperorIngStage3_FuncPtrs, 0x4)
void SetSEmperorIngStage3_FuncPtrs(SEmperorIngStage3_FuncPtrs* callbacks);

struct SEyeBall_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SEyeBall_FuncPtrs, 0x4)
void SetSEyeBall_FuncPtrs(SEyeBall_FuncPtrs* callbacks);

struct SFlyerSwarm_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SFlyerSwarm_FuncPtrs, 0x4)
void SetSFlyerSwarm_FuncPtrs(SFlyerSwarm_FuncPtrs* callbacks);

struct SFlyingPirate_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SFlyingPirate_FuncPtrs, 0x4)
void SetSFlyingPirate_FuncPtrs(SFlyingPirate_FuncPtrs* callbacks);

struct SScriptFogOverlay_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SScriptFogOverlay_FuncPtrs, 0x4)
void SetSScriptFogOverlay_FuncPtrs(SScriptFogOverlay_FuncPtrs* callbacks);

struct SGlowbug_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SGlowbug_FuncPtrs, 0x4)
void SetSGlowbug_FuncPtrs(SGlowbug_FuncPtrs* callbacks);

struct SGrenchler_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SGrenchler_FuncPtrs, 0x4)
void SetSGrenchler_FuncPtrs(SGrenchler_FuncPtrs* callbacks);

struct SGunTurretBase_FuncPtrs {
  // Guessed member names.
  FScriptLoader mLoadBase;
  FScriptLoader mLoadTop;
};
CHECK_SIZEOF(SGunTurretBase_FuncPtrs, 0x8)
void SetSGunTurretBase_FuncPtrs(SGunTurretBase_FuncPtrs* callbacks);

struct SIngBoostBallGuardian_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SIngBoostBallGuardian_FuncPtrs, 0x4)
void SetSIngBoostBallGuardian_FuncPtrs(SIngBoostBallGuardian_FuncPtrs* callbacks);

struct SIngBlobSwarm_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SIngBlobSwarm_FuncPtrs, 0x4)
void SetSIngBlobSwarm_FuncPtrs(SIngBlobSwarm_FuncPtrs* callbacks);

struct SIngPuddle_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SIngPuddle_FuncPtrs, 0x4)
void SetSIngPuddle_FuncPtrs(SIngPuddle_FuncPtrs* callbacks);

struct SIngSpiderballGuardian_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SIngSpiderballGuardian_FuncPtrs, 0x4)
void SetSIngSpiderballGuardian_FuncPtrs(SIngSpiderballGuardian_FuncPtrs* callbacks);

struct SLumite_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SLumite_FuncPtrs, 0x4)
void SetSLumite_FuncPtrs(SLumite_FuncPtrs* callbacks);

struct SMediumIng_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SMediumIng_FuncPtrs, 0x4)
void SetSMediumIng_FuncPtrs(SMediumIng_FuncPtrs* callbacks);

struct SMinorIng_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SMinorIng_FuncPtrs, 0x4)
void SetSMinorIng_FuncPtrs(SMinorIng_FuncPtrs* callbacks);

struct SMysteryFlyer_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SMysteryFlyer_FuncPtrs, 0x4)
void SetSMysteryFlyer_FuncPtrs(SMysteryFlyer_FuncPtrs* callbacks);

struct SPlantScarabSwarm_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SPlantScarabSwarm_FuncPtrs, 0x4)
void SetSPlantScarabSwarm_FuncPtrs(SPlantScarabSwarm_FuncPtrs* callbacks);

struct SPuddleSpore_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SPuddleSpore_FuncPtrs, 0x4)
void SetSPuddleSpore_FuncPtrs(SPuddleSpore_FuncPtrs* callbacks);

struct SRezbit_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SRezbit_FuncPtrs, 0x4)
void SetSRezbit_FuncPtrs(SRezbit_FuncPtrs* callbacks);

struct SSandBoss_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SSandBoss_FuncPtrs, 0x4)
void SetSSandBoss_FuncPtrs(SSandBoss_FuncPtrs* callbacks);

struct SScriptRsfAudio_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SScriptRsfAudio_FuncPtrs, 0x4)
void SetSScriptRsfAudio_FuncPtrs(SScriptRsfAudio_FuncPtrs* callbacks);

struct SStreamedMovie_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SStreamedMovie_FuncPtrs, 0x4)
void SetSStreamedMovie_FuncPtrs(SStreamedMovie_FuncPtrs* callbacks);

struct SShrieker_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SShrieker_FuncPtrs, 0x4)
void SetSShrieker_FuncPtrs(SShrieker_FuncPtrs* callbacks);

struct SScriptSkyRipple_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SScriptSkyRipple_FuncPtrs, 0x4)
void SetSScriptSkyRipple_FuncPtrs(SScriptSkyRipple_FuncPtrs* callbacks);

struct SSpankWeed_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SSpankWeed_FuncPtrs, 0x4)
void SetSSpankWeed_FuncPtrs(SSpankWeed_FuncPtrs* callbacks);

struct SSplinter_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SSplinter_FuncPtrs, 0x4)
void SetSSplinter_FuncPtrs(SSplinter_FuncPtrs* callbacks);

struct SSwampBossStage1_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SSwampBossStage1_FuncPtrs, 0x4)
void SetSSwampBossStage1_FuncPtrs(SSwampBossStage1_FuncPtrs* callbacks);

struct SSwampBossStage2_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SSwampBossStage2_FuncPtrs, 0x4)
void SetSSwampBossStage2_FuncPtrs(SSwampBossStage2_FuncPtrs* callbacks);

struct STryclops_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(STryclops_FuncPtrs, 0x4)
void SetSTryclops_FuncPtrs(STryclops_FuncPtrs* callbacks);

struct SWispTentacle_FuncPtrs {
  FScriptLoader mLoader; // Guessed member name.
};
CHECK_SIZEOF(SWispTentacle_FuncPtrs, 0x4)
void SetSWispTentacle_FuncPtrs(SWispTentacle_FuncPtrs* callbacks);

#endif // _SCRIPTLOADERREL
