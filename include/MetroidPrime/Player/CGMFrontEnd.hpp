#ifndef _CGMFRONTEND
#define _CGMFRONTEND

#include "types.h"

#include "MetroidPrime/Player/CGameMode.hpp"
#include "rstl/reserved_vector.hpp"

// Guessed name. Partial interface used when starting a game from the front end.
class CGMFrontEnd : public CGameMode {
public:
  // Guessed names
  enum ESelectedGameMode {
    kSGM_SinglePlayer = 'SNGL',
    kSGM_DeathMatch = 'DTHM',
    kSGM_Coin = 'COIN',
    kSGM_FrontEnd = 'FRND'
  };

  // Guessed name
  struct SPlayerConfig {
    uint mPlayerSelection;
    bool mRumbleEnabled;
    bool x5_; // Second controller option; meaning unresolved.
  };

  CGMFrontEnd();
  CGMFrontEnd(const CGMFrontEnd& other);

  // CGameMode
  ~CGMFrontEnd() override;
  void PutTo(COutputStream& out) const override;
  void Update(float dt, CStateManager& mgr) override;
  void OnPlayerKilled(CStateManager& mgr, TUniqueId victim, TUniqueId killer) override;
  void NotifyGenericEvent(CStateManager& mgr, TUniqueId player, uint value) override;
  void RespawnPlayer(CStateManager& mgr, uint playerIndex) override;
  bool IsMultiplayer() const override;
  void OnPlayerSpawned(CStateManager& mgr, uint playerIndex) override;
  void SetSpawnPoint(uint playerIndex, TUniqueId spawnPoint) override;
  void OnPlayerDamaged(CStateManager& mgr, TUniqueId victim, TUniqueId attacker,
                       float damage) override;
  void NotifyStop(CStateManager& mgr, TUniqueId player) override;
  uint GetNumPlayers() const override;
  bool IsGameOver() override;
  void EndGame(int resultIndex, CStateManager& mgr) override;
  int GetResultIndex() const override;
  int GetGameModeType() override;
  void GiveScore(CStateManager& mgr, uint playerIndex, uint amount) override;
  int GetItemAmount(const CStateManager& mgr, uint playerIndex) const override;
  bool IsNearScoreLimit(const CStateManager& mgr, uint playerIndex) const override;
  void AddListener(CGameModeListener& listener, uint playerIndex) override;
  void RemoveListener(CGameModeListener& listener, uint playerIndex) override;
  bool v21() const override;
  float GetElapsedTime() const override;
  float GetMatchTimeLimit() const override;

  const SPlayerConfig& GetPlayer(int player) const;
  int GetPlayerCount() const;
  float GetTimeLimit() const { return mTimeLimit; }
  ESelectedGameMode GetSelectedGameMode() const { return mSelectedGameMode; }
  int GetFragLimit() const { return mFragLimit; }
  int GetCoinLimit() const { return mCoinLimit; }
  int GetMusicIndex() const { return mMusicIndex; }

private:
  bool x4_;
  int x8_;
  ESelectedGameMode mSelectedGameMode;
  int mFragLimit;
  int mCoinLimit;
  float mTimeLimit;
  int mMusicIndex;
  rstl::reserved_vector< SPlayerConfig, 4 > mPlayers;
};

CHECK_SIZEOF(CGMFrontEnd, 0x44)
NESTED_CHECK_SIZEOF(CGMFrontEnd, SPlayerConfig, 8)

#endif // _CGMFRONTEND
