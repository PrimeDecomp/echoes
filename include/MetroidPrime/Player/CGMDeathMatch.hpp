#ifndef _CGMDEATHMATCH
#define _CGMDEATHMATCH

#include "MetroidPrime/Player/CGMMultiplayer.hpp"
#include "rstl/vector.hpp"

// Guessed name
class CGMDeathMatch : public CGMMultiplayer {
public:
  // Guessed name
  struct SPlayerState {
    SPlayerState()
    : mRespawnTimer(0.f), mDead(false), mScore(0), mDeaths(0), mPlayerSelection(-1) {}
    float mRespawnTimer;
    bool mDead;
    int mScore;
    int mDeaths;
    int mPlayerSelection;
  };

  CGMDeathMatch(int playerCount, int fragLimit, float timeLimit, bool awardFrags, bool flag);

  // CGameMode
  void PutTo(COutputStream& out) const override;
  void Update(float dt, CStateManager& mgr) override;
  void OnPlayerKilled(CStateManager& mgr, TUniqueId victim, TUniqueId killer) override;
  uint GetNumPlayers() const override;
  bool IsGameOver() override;
  void EndGame(int resultIndex, CStateManager& mgr) override;
  int GetGameModeType() override;
  void GiveScore(CStateManager& mgr, uint playerIndex, uint amount) override;
  int GetItemAmount(const CStateManager& mgr, uint playerIndex) const override;
  bool IsNearScoreLimit(const CStateManager& mgr, uint playerIndex) const override;

private:
  int mPlayerCount;
  int mFragLimit;
  bool x40_24_ : 1;
  bool mHasFragLimit : 1;
  bool mHasTimeLimit : 1;
  bool mAwardFrags : 1;
  bool mFragLimitReached : 1;
  rstl::vector< SPlayerState > mPlayers;
};

CHECK_SIZEOF(CGMDeathMatch, 0x54)
NESTED_CHECK_SIZEOF(CGMDeathMatch, SPlayerState, 0x14)

#endif // _CGMDEATHMATCH
