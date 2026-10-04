#ifndef _CGUNDRAWBLOCKSET
#define _CGUNDRAWBLOCKSET

#include "types.h"

class CStateManager;

// Guessed class and method names; native embedded storage and gun-block behavior are established.
class CGunDrawBlockSet {
public:
  CGunDrawBlockSet();
  ~CGunDrawBlockSet();

  void AddPlayer(CStateManager& mgr, int playerIndex, bool holsterGun);
  void RemovePlayer(CStateManager& mgr, int playerIndex);
  void Clear(CStateManager& mgr);

private:
  uint mBlockedPlayers;
};
CHECK_SIZEOF(CGunDrawBlockSet, 4)

#endif // _CGUNDRAWBLOCKSET
