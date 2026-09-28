#ifndef _CTWEAKGAME
#define _CTWEAKGAME

#include "rstl/string.hpp"
#include "rstl/single_ptr.hpp"

struct SLdrTweakGame;

class CTweakGame {
public:
  explicit CTweakGame(const SLdrTweakGame& data) : mData(&data) {}

  const rstl::string& GetPakFile();
  bool GetSplashScreensDisabled();
  int GetTotalPercentage();
  float GetHardModeDamageMultiplier() const;
  float GetHardModeWeaponMultiplier() const;

private:
  const SLdrTweakGame* mData;
};
CHECK_SIZEOF(CTweakGame, 0x4)

extern rstl::single_ptr< CTweakGame > gpTweakGame;

#endif // _CTWEAKGAME
