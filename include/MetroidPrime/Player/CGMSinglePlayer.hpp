#ifndef _CGMSINGLEPLAYER
#define _CGMSINGLEPLAYER

#include "types.h"

#include "MetroidPrime/Player/CGameMode.hpp"

// Name inferred: its game mode type is 'SNGL'.
class CGMSinglePlayer : public CGameMode {
public:
  CGMSinglePlayer();

  // CGameMode
  ~CGMSinglePlayer() override;
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

private:
  bool x4_;
  bool x5_;
  int x8_;
};
CHECK_SIZEOF(CGMSinglePlayer, 0xc)

#endif // _CGMSINGLEPLAYER
