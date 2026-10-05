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
  // Reconstructed accessors for the native pause-state gate.
  bool GetDisabled() const { return mRumbleGenerator.GetDisabled(); }
  void SetDisabled(bool disabled) { mRumbleGenerator.SetDisabled(disabled); }
  void Update(float dt);

private:
  int mPlayerIndex;
  EIOPort mPort;
  CRumbleGenerator mRumbleGenerator;
};
CHECK_SIZEOF(CRumbleManager, 0x4c)

#endif // _CRUMBLEMANAGER
