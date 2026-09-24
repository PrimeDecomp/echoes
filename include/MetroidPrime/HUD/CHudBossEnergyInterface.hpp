#ifndef _CHUDBOSSENERGYINTERFACE
#define _CHUDBOSSENERGYINTERFACE
#include "types.h"
class CGuiFrame;
class CGuiWidget;
class CGuiTextPane;
class CAuiEnergyBarT01;
class CHudBossEnergyInterface {
public:
  CHudBossEnergyInterface(CGuiFrame& frame, int hudState);
  ~CHudBossEnergyInterface();

private:
  float mAlpha;
  float mFade;
  float mCurrentEnergy;
  float mMaximumEnergy;
  bool mVisible : 1;
  CGuiWidget* mRoot;
  CAuiEnergyBarT01* mEnergyBar;
  CGuiTextPane* mName;
};
CHECK_SIZEOF(CHudBossEnergyInterface, 0x20)
#endif // _CHUDBOSSENERGYINTERFACE
