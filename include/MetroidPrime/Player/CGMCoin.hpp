#ifndef _CGMCOIN
#define _CGMCOIN

#include "MetroidPrime/Player/CGMMultiplayer.hpp"
#include "rstl/vector.hpp"

// Guessed name. Partial interface required by CGameState's game-start routine.
class CGMCoin : public CGMMultiplayer {
public:
  // Guessed name
  struct SPlayerState {
    bool x0_24_ : 1;
    bool x0_25_ : 1;
    float x4_;
    int x8_;
    int xc_;
  };

  CGMCoin(int playerCount, int coinLimit, float timeLimit, bool flag);

  // CGameMode
  ~CGMCoin() override;
  // TODO: recover the remaining virtual signatures before emitting this class's vtable.

private:
  bool x38_;
  int mPlayerCount;
  int mCoinLimit;
  rstl::vector< SPlayerState > mPlayers;
  int x54_;
};

CHECK_SIZEOF(CGMCoin, 0x58)
NESTED_CHECK_SIZEOF(CGMCoin, SPlayerState, 0x10)

#endif // _CGMCOIN
