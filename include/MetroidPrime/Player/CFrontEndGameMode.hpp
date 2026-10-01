#ifndef _CFRONTENDGAMEMODE
#define _CFRONTENDGAMEMODE

#include "types.h"

#include "MetroidPrime/Player/CGameMode.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"

// Class name from the Wii MP2 SEL (SetPlayerData__17CFrontEndGameMode...). Members are guessed.
class CFrontEndPlayerData {
public:
  uint mPlayerSelection;
  rstl::pair< bool, bool > mOptions; // Rumble enabled; second meaning unresolved.
};
CHECK_SIZEOF(CFrontEndPlayerData, 8)

// Class and SetNextGame*/SetPlayerData names from the Wii MP2 SEL; the rest is guessed.
class CFrontEndGameMode : public CGameMode {
public:
  // Guessed names
  enum ESelectedGameMode {
    kSGM_SinglePlayer = 'SNGL',
    kSGM_DeathMatch = 'DTHM',
    kSGM_Coin = 'COIN',
    kSGM_FrontEnd = 'FRND'
  };

  CFrontEndGameMode();

  // CGameMode
  ~CFrontEndGameMode() override {}
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
  uint GetNumPlayers() const override { return 1; }
  bool IsGameOver() override;
  void EndGame(int resultIndex, CStateManager& mgr) override;
  int GetResultIndex() const override;
  int GetGameModeType() override { return kSGM_FrontEnd; }
  void GiveScore(CStateManager& mgr, uint playerIndex, uint amount) override;
  int GetItemAmount(const CStateManager& mgr, uint playerIndex) const override;
  bool IsNearScoreLimit(const CStateManager& mgr, uint playerIndex) const override;
  void AddListener(CGameModeListener& listener, uint playerIndex) override;
  void RemoveListener(CGameModeListener& listener, uint playerIndex) override;
  bool v21() const override;
  float GetElapsedTime() const override { return -1.f; }
  float GetMatchTimeLimit() const override { return -1.f; }

  void SetNextGameType4CC(uint type);
  void SetNextGameFragLimit(int limit);
  void SetNextGameCoinLimit(int limit);
  void SetNextGameTimeLimit(float limit);
  void SetNextGameMusicIndex(int index);
  void SetPlayerData(const rstl::reserved_vector< CFrontEndPlayerData, 4 >& players);

  const CFrontEndPlayerData& GetPlayer(int player) const;
  int GetPlayerCount() const;
  float GetTimeLimit() const { return mTimeLimit; }
  ESelectedGameMode GetSelectedGameMode() const { return mSelectedGameMode; }
  int GetFragLimit() const { return mFragLimit; }
  int GetCoinLimit() const { return mCoinLimit; }
  int GetMusicIndex() const { return mMusicIndex; }

private:
  bool mGameOver;
  int mResultIndex;
  ESelectedGameMode mSelectedGameMode;
  int mFragLimit;
  int mCoinLimit;
  float mTimeLimit;
  int mMusicIndex;
  rstl::reserved_vector< CFrontEndPlayerData, 4 > mPlayers;
};

CHECK_SIZEOF(CFrontEndGameMode, 0x44)

#endif // _CFRONTENDGAMEMODE
