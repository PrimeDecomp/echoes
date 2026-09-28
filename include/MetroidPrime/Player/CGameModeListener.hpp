#ifndef _CGAMEMODELISTENER
#define _CGAMEMODELISTENER

#include "types.h"

class CStateManager;

// Guessed name. Shared listener base embedded in script player proxies.
class CGameModeListener {
public:
  CGameModeListener(CStateManager& mgr, uint playerMask);
  virtual ~CGameModeListener() = 0;
  virtual void OnGameEvent(CStateManager& mgr, uint sourceIndex, uint targetIndex, uint event,
                           const void* value) = 0;
  void Unregister(CStateManager& mgr);

private:
  uint mPlayerMask;
};

CHECK_SIZEOF(CGameModeListener, 8)

#endif // _CGAMEMODELISTENER
