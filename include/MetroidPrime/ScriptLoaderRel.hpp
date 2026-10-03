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
