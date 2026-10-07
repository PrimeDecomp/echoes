#include "MetroidPrime/Player/CGMCoin.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "rstl/math.hpp"

// Guessed name. Initial coin count decreases with the player's death count.
static int sRespawnCoins[] = {100, 80, 60, 50, 40, 30, 20};

CGMCoin::CGMCoin(int playerCount, int coinLimit, float timeLimit, bool flag)
: CGMMultiplayer(timeLimit, flag)
, x38_(false)
, mPlayerCount(playerCount)
, mCoinLimit(coinLimit)
, mPlayers(playerCount, SPlayerState())
, mCoinLimitReached(false) {}

void CGMCoin::Update(float dt, CStateManager& mgr) {
  UpdateTimer(dt, mgr);
  if (GetMatchTimeLimit() > 0.f && GetElapsedTime() > GetMatchTimeLimit()) {
    EndGame(GetResultIndex(), mgr);
  }

  for (uint i = 0; i < uint(mPlayerCount); ++i) {
    CPlayerState& state = *mgr.PlayerState(i);
    SPlayerState& player = mPlayers[i];
    state.ReInitializePowerUp(CPlayerState::kIT_CoinCounter, 0x8000);
    if (player.mDead) {
      player.mRespawnTimer -= dt;
      if (player.mRespawnTimer < 0.f && player.mCanRespawn && mgr.GetPlayer(i)->fn_80019e20(mgr)) {
        RespawnPlayer(mgr, i);
        player.mDead = false;
      }
    } else if (!state.IsPlayerAlive()) {
      player.mDead = true;
      state.AddPowerUp(CPlayerState::kIT_DiedCount, 1);
      state.IncrPickUp(CPlayerState::kIT_DiedCount, 1);
      player.mRespawnTimer = 1.f;
    }
    if (state.GetItemAmount(CPlayerState::kIT_CoinCounter) >= mCoinLimit && mCoinLimit != -1) {
      EndGame(GetResultIndex(), mgr);
      mCoinLimitReached = true;
    }
  }
}

void CGMCoin::RespawnPlayer(CStateManager& mgr, uint playerIndex) {
  CGMMultiplayer::RespawnPlayer(mgr, playerIndex);
  CPlayerState& state = *mgr.PlayerState(playerIndex);
  int deaths = state.GetPowerUp(CPlayerState::kIT_DiedCount).mAmount;
  if (deaths >= 7) {
    deaths = 6;
  }
  state.PowerUp(CPlayerState::kIT_CoinCounter).mAmount = sRespawnCoins[deaths];
}

void CGMCoin::OnPlayerSpawned(CStateManager& mgr, uint playerIndex) {
  CGMMultiplayer::OnPlayerSpawned(mgr, playerIndex);
  mgr.PlayerState(playerIndex)->PowerUp(CPlayerState::kIT_CoinCounter).mAmount = sRespawnCoins[0];
}

uint CGMCoin::GetNumPlayers() const { return mPlayerCount; }

bool CGMCoin::IsGameOver() { return x38_ || CGMMultiplayer::IsGameOver(); }

void CGMCoin::EndGame(int resultIndex, CStateManager& mgr) {
  CGMMultiplayer::EndGame(resultIndex, mgr);
  for (int i = 0; i < mPlayerCount; ++i) {
    mPlayers[i].mScore = GetItemAmount(mgr, i);
    mPlayers[i].mPlayerSelection = mgr.GetPlayerState(i)->GetPlayerSelection();
  }
}

int CGMCoin::GetGameModeType() { return 'COIN'; }

void CGMCoin::GiveScore(CStateManager&, uint, uint) {}

int CGMCoin::GetItemAmount(const CStateManager& mgr, uint playerIndex) const {
  return mgr.GetPlayerState(playerIndex)->GetItemAmount(CPlayerState::kIT_CoinCounter);
}

bool CGMCoin::IsNearScoreLimit(const CStateManager& mgr, uint playerIndex) const {
  return mCoinLimit - GetItemAmount(mgr, playerIndex) <= 1 && mCoinLimit > 1;
}

bool CGMCoin::IsNearTimeLimit() const {
  const float remaining = GetMatchTimeLimit() - GetElapsedTime();
  if (GetMatchTimeLimit() > 0.f) {
    if (GetMatchTimeLimit() > 60.f) {
      return remaining < 60.f;
    }
    return GetElapsedTime() > 0.5f * GetMatchTimeLimit();
  }
  return false;
}

int CGMCoin::GetCoinLimit() const { return mCoinLimit; }

CGMCoin::~CGMCoin() {}
