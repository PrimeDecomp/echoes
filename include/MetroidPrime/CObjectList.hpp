#ifndef _COBJECTLIST
#define _COBJECTLIST

#include "TGameTypes.hpp"

class CEntity;


enum EGameObjectList {
  kOL_Invalid = -1,
  kOL_All,
  kOL_Actor,
  kOL_PhysicsActor,
  kOL_GameCamera,
  kOL_GameLight,
  kOL_ListeningAi,
  kOL_AiWaypoint,
  kOL_PlatformAndDoor,
  kOL_Unk,
};

class CObjectList {
public:
  CEntity* fn_8000B538(TUniqueId id) const;
};

#endif // _COBJECTLIST
