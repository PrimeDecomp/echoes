#ifndef _CRUMBLEMANAGER
#define _CRUMBLEMANAGER

#include "types.h"

#include "Kyoto/Input/CRumbleGenerator.hpp"

class CStateManager;
class CVector3f;

class CRumbleManager {
public:
  CRumbleManager(int playerIndex, EIOPort port);
  ~CRumbleManager();

  short Rumble(CStateManager&, const CVector3f&, ERumbleFxId, float, ERumblePriority);
  short Rumble(CStateManager&, ERumbleFxId, float, ERumblePriority);

  void StopRumble(short id);
  void HardStopAll() { mRumbleGenerator.HardStopAll(); }
  void Update(float dt);

private:
  int mPlayerIndex;
  EIOPort mPort;
  CRumbleGenerator mRumbleGenerator;
};
CHECK_SIZEOF(CRumbleManager, 0x4c)

#endif // _CRUMBLEMANAGER
