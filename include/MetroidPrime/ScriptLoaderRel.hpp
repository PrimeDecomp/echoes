#ifndef _SCRIPTLOADERREL
#define _SCRIPTLOADERREL

#include "MetroidPrime/ScriptLoader.hpp"

#include "Kyoto/Math/CTransform4f.hpp"
#include "MetroidPrime/TGameTypes.hpp"

class CVector3f;
class CDamageInfo;
class CFinalInput;
class CSpacePirate;

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
  FScriptLoader guiWidget;
  FScriptLoader guiScreen;
  FScriptLoader guiSlider;
  FScriptLoader guiMenu;
  FScriptLoader guiPlayerJoinManager;
};
void SetSGuiWidget_FuncPtrs(SGuiWidget_FuncPtrs*);

struct SSafeZone_FuncPtrs {
  FScriptLoader safeZone;
  FScriptLoader safeZoneCrystal;
  void (CEntity::*method)(CStateManager& mgr);
};
void SetSSafeZone_FuncPtrs(SSafeZone_FuncPtrs*);

struct SFishCloud_FuncPtrs {
  FScriptLoader fishCloud;
  FScriptLoader fishCloudModifier;
};
void SetSFishCloud_FuncPtrs(SFishCloud_FuncPtrs*);

struct SSnakeWeedSwarm_FuncPtrs {
  FScriptLoader swarm;
  void (CEntity::*method)(const CVector3f&, const CDamageInfo&, CStateManager&);
};
void SetSSnakeWeedSwarm_FuncPtrs(SSnakeWeedSwarm_FuncPtrs*);

struct SPlayerActor_FuncPtrs {
  FScriptLoader loader;
  void (CEntity::*method)(CStateManager& mgr);
};
void SetSPlayerActor_FuncPtrs(SPlayerActor_FuncPtrs*);

struct SPlayerTurret_FuncPtrs {
  FScriptLoader loader;
  CTransform4f (CEntity::*GetTransform1)(CStateManager&);
  CTransform4f (CEntity::*GetTransform2)(CStateManager&);
  void (CEntity::*SendSomeMsg)(CStateManager&);
  void (CEntity::*CheckInput)(float, CFinalInput&, CStateManager&);
  TUniqueId (CEntity::*GetSomeId)();
};
void SetSPlayerTurret_FuncPtrs(SPlayerTurret_FuncPtrs*);
TUniqueId PlayerTurret_GetSomeId(CEntity& entity); // Guessed name; existing REL ID query.

struct SScriptForgottenObject_FuncPtrs {
  FScriptLoader loader;
};
void SetSScriptForgottenObject_FuncPtrs(SScriptForgottenObject_FuncPtrs*);

void SetLoader_CannonBall(FScriptLoader* loader);

struct STweaks_FuncPtrs {
  void (*Loader)(CInputStream&);
  void (*CreateGlobals)();
  void (*FreeTweaks)();
};
void SetTweaks_FuncPtrs(STweaks_FuncPtrs*);

// Guessed names; dispatch through the currently loaded Tweaks REL's function table.
void LoadTweaks(CInputStream&);
void CreateTweakGlobals();
void FreeTweaks();

#endif // _SCRIPTLOADERREL
