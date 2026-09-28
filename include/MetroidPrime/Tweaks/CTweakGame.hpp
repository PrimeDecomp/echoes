#ifndef _CTWEAKGAME
#define _CTWEAKGAME

#include "rstl/string.hpp"

class CTweakGame {
public:
  const rstl::string& GetPakFile();
  bool GetSplashScreensDisabled();
  int GetTotalPercentage();
  float GetHardModeDamageMultiplier() const;
  float GetHardModeWeaponMultiplier() const;
};

extern CTweakGame* gpTweakGame;

#endif // _CTWEAKGAME
