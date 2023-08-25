#ifndef _SCRIPTLOADERREL
#define _SCRIPTLOADERREL

#include "MetroidPrime/ScriptLoader.hpp"

#include "Kyoto/Math/CTransform4f.hpp"
#include "MetroidPrime/TGameTypes.hpp"

class CVector3f;
class CDamageInfo;
class CFinalInput;

struct GUILoaders {
  FScriptLoader guiWidget;
  FScriptLoader guiScreen;
  FScriptLoader guiSlider;
  FScriptLoader guiMenu;
  FScriptLoader guiPlayerJoinManager;
};

struct SafeCrystalLoaders {
  FScriptLoader safeZone;
  FScriptLoader safeZoneCrystal;
  void (CEntity::*method)(CStateManager& mgr);
};

struct FishCloudLoaders {
  FScriptLoader fishCloud;
  FScriptLoader fishCloudModifier;
};

struct SnakeWeedLoaders {
  FScriptLoader swarm;
  void (CEntity::*method)(const CVector3f&, const CDamageInfo&, CStateManager&);
};

struct PlayerActorFunctions {
  FScriptLoader loader;
  void (CEntity::*method)(CStateManager& mgr);
};

struct PlayerTurretFunctions {
  FScriptLoader loader;
  CTransform4f (CEntity::*GetTransform1)(CStateManager&);
  CTransform4f (CEntity::*GetTransform2)(CStateManager&);
  void (CEntity::*SendSomeMsg)(CStateManager&);
  void (CEntity::*CheckInput)(float, CFinalInput&, CStateManager&);
  TUniqueId (CEntity::*GetSomeId)();
};

#endif // _SCRIPTLOADERREL
