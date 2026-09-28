#ifndef _CGMDEATHMATCH
#define _CGMDEATHMATCH

#include "MetroidPrime/Player/CGMMultiplayer.hpp"
#include "rstl/vector.hpp"

// Guessed name. Partial interface required by CGameState's game-start routine.
class CGMDeathMatch : public CGMMultiplayer {
public:
  // Guessed name
  struct SPlayerState {
    float x0_;
    bool x4_;
    int x8_;
    int xc_;
    int x10_;
  };

  CGMDeathMatch(int playerCount, int fragLimit, float timeLimit, bool flag1, bool flag2);

  // CGameMode
  ~CGMDeathMatch() override;
  // TODO: recover the remaining virtual signatures before emitting this class's vtable.

private:
  int mPlayerCount;
  int mFragLimit;
  bool x40_24_ : 1;
  bool mHasFragLimit : 1;
  bool mHasTimeLimit : 1;
  bool x40_27_ : 1;
  bool x40_28_ : 1;
  rstl::vector< SPlayerState > mPlayers;
};

CHECK_SIZEOF(CGMDeathMatch, 0x54)
NESTED_CHECK_SIZEOF(CGMDeathMatch, SPlayerState, 0x14)

#endif // _CGMDEATHMATCH
