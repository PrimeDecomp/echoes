#ifndef _CGAMEMODE
#define _CGAMEMODE

#include "MetroidPrime/TGameTypes.hpp"

class CGameModeListener;
class COutputStream;
class CStateManager;

// Method names are guessed from the GameCube virtual interface.
class CGameMode {
public:
  virtual ~CGameMode() {}
  virtual void PutTo(COutputStream& out) const = 0;
  virtual void Update(float dt, CStateManager& mgr) = 0;
  virtual void OnPlayerKilled(CStateManager& mgr, TUniqueId victim, TUniqueId killer) = 0;
  virtual void NotifyGenericEvent(CStateManager& mgr, TUniqueId player, uint value) = 0;
  virtual void RespawnPlayer(CStateManager& mgr, uint playerIndex) = 0;
  virtual bool IsMultiplayer() const = 0;
  virtual void OnPlayerSpawned(CStateManager& mgr, uint playerIndex) = 0;
  virtual void SetSpawnPoint(uint playerIndex, TUniqueId spawnPoint) = 0;
  virtual void OnPlayerDamaged(CStateManager& mgr, TUniqueId victim, TUniqueId attacker,
                               float damage) = 0;
  virtual void NotifyStop(CStateManager& mgr, TUniqueId player) = 0;
  virtual uint GetNumPlayers() const = 0;
  virtual bool IsGameOver() = 0;
  virtual void EndGame(int resultIndex, CStateManager& mgr) = 0;
  virtual int GetResultIndex() const = 0;
  virtual int GetGameModeType() = 0;
  virtual void GiveScore(CStateManager& mgr, uint playerIndex, uint amount) = 0;
  virtual int GetItemAmount(const CStateManager& mgr, uint playerIndex) const = 0;
  virtual bool IsNearScoreLimit(const CStateManager& mgr, uint playerIndex) const = 0;
  virtual void AddListener(CGameModeListener& listener, uint playerIndex) = 0;
  virtual void RemoveListener(CGameModeListener& listener, uint playerIndex) = 0;
  virtual bool v21() const = 0;
  virtual float GetElapsedTime() const = 0;
  virtual float GetMatchTimeLimit() const = 0;
};

CHECK_SIZEOF(CGameMode, 4)

#endif // _CGAMEMODE
