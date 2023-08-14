#ifndef _CSTATEMANAGER
#define _CSTATEMANAGER

#include "TGameTypes.hpp"

class CStateManager {
public:
  TUniqueId AllocateUniqueId();
  const CEntity* GetObjectById(TUniqueId id) const;
  uint MaskUIdNumPlayers(TUniqueId id) const;
  void ShowPausedHUDMemo(CAssetId strg, float time);
};

#endif // _CSTATEMANAGER
