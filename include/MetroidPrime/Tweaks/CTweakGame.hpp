#ifndef _CTWEAKGAME
#define _CTWEAKGAME

#include "rstl/string.hpp"

class CTweakGame {
public:
  const rstl::string& GetPakFile();
  bool GetSplashScreensDisabled();
};

extern CTweakGame* gpTweakGame;

#endif // _CTWEAKGAME
