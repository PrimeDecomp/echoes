#ifndef _CGMCOIN
#define _CGMCOIN

#include "MetroidPrime/Player/CGMMultiplayer.hpp"
#include "rstl/vector.hpp"

// Guessed name
class CGMCoin : public CGMMultiplayer {
public:
  // Guessed name
  struct SPlayerState {
    SPlayerState()
    : mDead(false), mCanRespawn(true), mRespawnTimer(0.f), mScore(0), mPlayerSelection(-1) {}
    bool mDead : 1;
    bool mCanRespawn : 1;
    float mRespawnTimer;
    int mScore;
    int mPlayerSelection;
  };

  CGMCoin(int playerCount, int coinLimit, float timeLimit, bool flag);

  // CGameMode
  ~CGMCoin() override;
  void Update(float dt, CStateManager& mgr) override;
  void RespawnPlayer(CStateManager& mgr, uint playerIndex) override;
  void OnPlayerSpawned(CStateManager& mgr, uint playerIndex) override;
  uint GetNumPlayers() const override;
  bool IsGameOver() override;
  void EndGame(int resultIndex, CStateManager& mgr) override;
  int GetGameModeType() override;
  void GiveScore(CStateManager& mgr, uint playerIndex, uint amount) override;
  int GetItemAmount(const CStateManager& mgr, uint playerIndex) const override;
  bool IsNearScoreLimit(const CStateManager& mgr, uint playerIndex) const override;

  bool IsNearTimeLimit() const;
  int GetCoinLimit() const;

private:
  bool x38_;
  int mPlayerCount;
  int mCoinLimit;
  rstl::vector< SPlayerState > mPlayers;
  bool mCoinLimitReached;
};

CHECK_SIZEOF(CGMCoin, 0x58)
NESTED_CHECK_SIZEOF(CGMCoin, SPlayerState, 0x10)

#endif // _CGMCOIN
