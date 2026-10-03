#ifndef _CHUDBOSSENERGYINTERFACE
#define _CHUDBOSSENERGYINTERFACE

#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/pair.hpp"
#include "rstl/string.hpp"
#include "types.h"

class CGuiFrame;
class CGuiWidget;
class CGuiTextPane;
class CAuiEnergyBarT01;
class CHudBossEnergyInterface {
public:
  CHudBossEnergyInterface(CGuiFrame& frame, int hudState);
  ~CHudBossEnergyInterface();
  void Update(float dt);
  void SetAlpha(float alpha);
  void SetBossParams(bool visible, const rstl::wstring& name, float energy, float maxEnergy);
  static rstl::pair< CVector3f, CVector3f > BallBossEnergyCoordFunc(float t);
  static rstl::pair< CVector3f, CVector3f > BossEnergyCoordFunc(float t);

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
