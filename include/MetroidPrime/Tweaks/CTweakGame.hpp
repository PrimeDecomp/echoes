#ifndef _CTWEAKGAME
#define _CTWEAKGAME

#include "rstl/string.hpp"

class CTweakGame {
public:
  const rstl::string& GetPakFile();
};

extern CTweakGame* gpTweakGame;

#endif // _CTWEAKGAME
