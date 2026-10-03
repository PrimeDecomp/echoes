#include "MetroidPrime/Player/CFrontEndGameMode.hpp"

CFrontEndGameMode::CFrontEndGameMode()
: mGameOver(false)
, mResultIndex(-1)
, mSelectedGameMode(kSGM_FrontEnd)
, mFragLimit(-1)
, mCoinLimit(-1)
, mTimeLimit(-1.f)
, mMusicIndex(0) {}

void CFrontEndGameMode::PutTo(COutputStream& out) const {}

void CFrontEndGameMode::Update(float dt, CStateManager& mgr) {}

void CFrontEndGameMode::OnPlayerKilled(CStateManager& mgr, TUniqueId victim, TUniqueId killer) {}

void CFrontEndGameMode::NotifyGenericEvent(CStateManager& mgr, TUniqueId player, uint value) {}

void CFrontEndGameMode::RespawnPlayer(CStateManager& mgr, uint playerIndex) {}

void CFrontEndGameMode::OnPlayerSpawned(CStateManager& mgr, uint playerIndex) {}

bool CFrontEndGameMode::IsGameOver() { return mGameOver; }

void CFrontEndGameMode::OnPlayerDamaged(CStateManager& mgr, TUniqueId victim, TUniqueId attacker,
                                        float damage) {}

void CFrontEndGameMode::NotifyStop(CStateManager& mgr, TUniqueId player) {}

void CFrontEndGameMode::EndGame(int resultIndex, CStateManager& mgr) {
  mGameOver = true;
  mResultIndex = resultIndex;
}

int CFrontEndGameMode::GetResultIndex() const { return mResultIndex; }

void CFrontEndGameMode::GiveScore(CStateManager& mgr, uint playerIndex, uint amount) {}

void CFrontEndGameMode::AddListener(CPlayerListener& listener, uint playerIndex) {}

void CFrontEndGameMode::RemoveListener(CPlayerListener& listener, uint playerIndex) {}

bool CFrontEndGameMode::v21() const { return false; }

void CFrontEndGameMode::SetSpawnPoint(uint playerIndex, TUniqueId spawnPoint) {}

int CFrontEndGameMode::GetItemAmount(const CStateManager& mgr, uint playerIndex) const {
  return 0;
}

bool CFrontEndGameMode::IsNearScoreLimit(const CStateManager& mgr, uint playerIndex) const {
  return false;
}

void CFrontEndGameMode::SetNextGameType4CC(uint type) {
  mSelectedGameMode = static_cast< ESelectedGameMode >(type);
}

int CFrontEndGameMode::GetPlayerCount() const { return mPlayers.size(); }

const CFrontEndPlayerData& CFrontEndGameMode::GetPlayer(int player) const {
  return mPlayers[player];
}

void CFrontEndGameMode::SetNextGameFragLimit(int limit) { mFragLimit = limit; }

void CFrontEndGameMode::SetNextGameCoinLimit(int limit) { mCoinLimit = limit; }

void CFrontEndGameMode::SetNextGameTimeLimit(float limit) { mTimeLimit = limit; }

void CFrontEndGameMode::SetNextGameMusicIndex(int index) { mMusicIndex = index; }

void CFrontEndGameMode::SetPlayerData(
    const rstl::reserved_vector< CFrontEndPlayerData, 4 >& players) {
  mPlayers = players;
}

bool CFrontEndGameMode::IsMultiplayer() const { return true; }
