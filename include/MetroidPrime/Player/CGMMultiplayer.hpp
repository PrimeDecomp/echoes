#ifndef _CGMMULTIPLAYER
#define _CGMMULTIPLAYER

#include "MetroidPrime/Player/CGameMode.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/set.hpp"

// Guessed name. Shared base of the deathmatch and coin game modes.
class CGMMultiplayer : public CGameMode {
public:
  CGMMultiplayer(float timeLimit, bool flag);

  // CGameMode
  ~CGMMultiplayer() override {}
  void PutTo(COutputStream& out) const override;
  void OnPlayerKilled(CStateManager& mgr, TUniqueId victim, TUniqueId killer) override;
  void NotifyGenericEvent(CStateManager& mgr, TUniqueId player, uint value) override;
  void RespawnPlayer(CStateManager& mgr, uint playerIndex) override;
  bool IsMultiplayer() const override;
  void OnPlayerSpawned(CStateManager& mgr, uint playerIndex) override;
  void SetSpawnPoint(uint playerIndex, TUniqueId spawnPoint) override;
  void OnPlayerDamaged(CStateManager& mgr, TUniqueId victim, TUniqueId attacker,
                       float damage) override;
  void NotifyStop(CStateManager& mgr, TUniqueId player) override;
  bool IsGameOver() override;
  void EndGame(int resultIndex, CStateManager& mgr) override;
  int GetResultIndex() const override;
  void AddListener(CPlayerListener& listener, uint playerIndex) override;
  void RemoveListener(CPlayerListener& listener, uint playerIndex) override;
  bool v21() const override;
  float GetElapsedTime() const override { return mElapsedTime; }
  float GetMatchTimeLimit() const override { return mTimeLimit; }

  virtual void OnPlayerScanned(CStateManager& mgr, TUniqueId target, TUniqueId scanner);
  virtual TUniqueId ChooseSpawnPoint(CStateManager& mgr, uint playerIndex, TUniqueId requested);

  void SetMusicIndex(int musicIndex);
  int GetMusicIndex() const { return mMusicIndex; } // Guessed name; multiplayer pause music choice.
  void UpdateTimer(float dt, CStateManager& mgr);

protected:
  // Guessed names. FourCCs carried by the native listener callback.
  enum EGameEvent {
    kGE_Score = 'SCOR',
    kGE_Kill = 'KILL',
    kGE_Generic = 'GNRT',
    kGE_Scan = 'SCAN',
    kGE_Damage = 'DAMG',
    kGE_Stop = 'STOP',
    kGE_Spawn = 'SPWN'
  };

  void NotifyListeners(CStateManager& mgr, uint sourceIndex, uint targetIndex, EGameEvent event,
                       const void* value);

private:
  typedef rstl::pair< uint, CPlayerListener* > TListener;
  float mTimeLimit;
  float mElapsedTime;
  int mMusicIndex;
  rstl::set< TListener > mListeners;
  rstl::reserved_vector< TUniqueId, 4 > mSpawnPoints;
  int mResultIndex;
  bool x34_24_ : 1; // Serialized option, queried by v21; meaning unresolved.
  bool mGameOver : 1;
};

CHECK_SIZEOF(CGMMultiplayer, 0x38)

#endif // _CGMMULTIPLAYER
