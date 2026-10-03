#ifndef _CTURRETHUD
#define _CTURRETHUD

#include "rstl/single_ptr.hpp"

class CAuiEnergyBarT01;
class CGuiFrame;
class CGuiFrameLoader;
class CStateManager;

// Guessed names; binds energybart01_hullenergy in the native turret HUD.
class CTurretHud {
public:
  CTurretHud(const CStateManager& mgr, int playerIndex);

private:
  int mPlayerIndex;
  rstl::single_ptr< CGuiFrameLoader > mFrameLoader;
  rstl::single_ptr< CGuiFrame > mFrame;
  CAuiEnergyBarT01* mHullEnergy;
};
CHECK_SIZEOF(CTurretHud, 0x10)

#endif // _CTURRETHUD
