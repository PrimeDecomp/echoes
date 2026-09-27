#ifndef _CGMSINGLEPLAYER
#define _CGMSINGLEPLAYER

#include "types.h"

#include "MetroidPrime/Player/CGameMode.hpp"

// Name inferred: its game mode type is 'SNGL'.
class CGMSinglePlayer : public CGameMode {
public:
  CGMSinglePlayer();

private:
  bool x4_;
  bool x5_;
  int x8_;
};
CHECK_SIZEOF(CGMSinglePlayer, 0xc)

#endif // _CGMSINGLEPLAYER
