#ifndef _CTURRETHUD
#define _CTURRETHUD

#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/pair.hpp"
#include "rstl/single_ptr.hpp"

class CAuiEnergyBarT01;
class CGuiFrame;
class CGuiFrameLoader;
class CStateManager;

// Guessed names; binds energybart01_hullenergy in the native turret HUD.
class CTurretHud {
public:
  CTurretHud(const CStateManager& mgr, int playerIndex);
  void Draw() const;
  void Update(float dt, const CStateManager& mgr);

private:
  void UpdateEnergy(const CStateManager& mgr);
  void BindWidgets();
  static rstl::pair< CVector3f, CVector3f > GetEnergyBarCoords(float t);

  int mPlayerIndex;
  rstl::single_ptr< CGuiFrameLoader > mFrameLoader;
  rstl::single_ptr< CGuiFrame > mFrame;
  CAuiEnergyBarT01* mHullEnergy;
};
CHECK_SIZEOF(CTurretHud, 0x10)

#endif // _CTURRETHUD
