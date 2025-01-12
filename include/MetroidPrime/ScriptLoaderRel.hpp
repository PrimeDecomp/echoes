#ifndef _SCRIPTLOADERREL
#define _SCRIPTLOADERREL

#include "MetroidPrime/ScriptLoader.hpp"

#include "Kyoto/Math/CTransform4f.hpp"
#include "MetroidPrime/TGameTypes.hpp"

class CVector3f;
class CDamageInfo;
class CFinalInput;

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

struct SScriptForgottenObject_FuncPtrs {
  FScriptLoader loader;
};
void SetSScriptForgottenObject_FuncPtrs(SScriptForgottenObject_FuncPtrs*);

void SetLoader_CannonBall(FScriptLoader* loader);

#endif // _SCRIPTLOADERREL
