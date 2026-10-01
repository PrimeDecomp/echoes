#include "MetroidPrime/Player/CGMDeathMatch.hpp"

#include "Kyoto/Streams/COutputStream.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"

CGMDeathMatch::CGMDeathMatch(int playerCount, int fragLimit, float timeLimit, bool awardFrags,
                             bool flag)
: CGMMultiplayer(timeLimit, flag)
, mPlayerCount(playerCount)
, mFragLimit(fragLimit)
, x40_24_(false)
, mHasFragLimit(fragLimit > 0)
, mHasTimeLimit(timeLimit > 0.f)
, mAwardFrags(awardFrags)
, mFragLimitReached(false)
, mPlayers(4, SPlayerState()) {}

void CGMDeathMatch::PutTo(COutputStream& out) const {
  CGMMultiplayer::PutTo(out);
  out.WriteInt32(mPlayerCount);
  out.WriteInt32(mFragLimit);
  out.WriteBool(mAwardFrags);
}

void CGMDeathMatch::Update(float dt, CStateManager& mgr) {
  UpdateTimer(dt, mgr);
  if (mgr.GetIsDarkWorld()) {
    mgr.SetIsDarkWorld(false);
  }
  if (mHasTimeLimit && GetMatchTimeLimit() > 0.f && GetElapsedTime() > GetMatchTimeLimit()) {
    EndGame(GetResultIndex(), mgr);
  }

  for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
    CPlayerState& state = *mgr.PlayerState(i);
    SPlayerState& player = mPlayers[i];
    CPlayer* playerObj = mgr.GetPlayer(i);
    if (!state.IsPlayerAlive()) {
      if (player.mDead) {
        player.mRespawnTimer -= dt;
        if (player.mRespawnTimer < 0.f && playerObj->fn_80019e20(mgr)) {
          RespawnPlayer(mgr, i);
          player.mDead = false;
        }
      } else {
        player.mDead = true;
        state.AddPowerUp(CPlayerState::kIT_DiedCount, 1);
        state.IncrPickUp(CPlayerState::kIT_DiedCount, 1);
        player.mRespawnTimer = 1.f;
        player.mDeaths = state.GetItemAmount(CPlayerState::kIT_DiedCount);
      }
    }
    if (mHasFragLimit && state.GetItemAmount(CPlayerState::kIT_FragCount) >= mFragLimit) {
      mFragLimitReached = true;
      EndGame(GetResultIndex(), mgr);
    }
  }
}

void CGMDeathMatch::OnPlayerKilled(CStateManager& mgr, TUniqueId victim, TUniqueId killer) {
  const CPlayer* victimPlayer = TCastToConstPtr< CPlayer >(mgr.GetObjectById(victim));
  const CPlayer* killerPlayer = TCastToConstPtr< CPlayer >(mgr.GetObjectById(killer));
  mgr.MaskUIdNumPlayers(victim);
  if (victimPlayer != nullptr && killerPlayer != nullptr) {
    CPlayerState& state = *mgr.PlayerState(mgr.MaskUIdNumPlayers(killer));
    if (mAwardFrags) {
      state.AddPowerUp(CPlayerState::kIT_FragCount, 1);
      state.IncrPickUp(CPlayerState::kIT_FragCount, 1);
    }
  }
  CGMMultiplayer::OnPlayerKilled(mgr, victim, killer);
}

bool CGMDeathMatch::IsGameOver() { return x40_24_ || CGMMultiplayer::IsGameOver(); }

void CGMDeathMatch::EndGame(int resultIndex, CStateManager& mgr) {
  CGMMultiplayer::EndGame(resultIndex, mgr);
  for (int i = 0; i < mPlayerCount; ++i) {
    const CPlayerState& state = *mgr.GetPlayerState(i);
    SPlayerState& player = mPlayers[i];
    player.mScore = state.GetItemAmount(CPlayerState::kIT_FragCount);
    player.mDeaths = state.GetItemAmount(CPlayerState::kIT_DiedCount);
    player.mPlayerSelection = state.GetPlayerSelection();
  }
}

void CGMDeathMatch::GiveScore(CStateManager& mgr, uint playerIndex, uint amount) {
  CPlayerState& state = *mgr.PlayerState(playerIndex);
  state.AddPowerUp(CPlayerState::kIT_FragCount, amount);
  state.IncrPickUp(CPlayerState::kIT_FragCount, amount);
}

int CGMDeathMatch::GetItemAmount(const CStateManager& mgr, uint playerIndex) const {
  return mgr.GetPlayerState(playerIndex)->GetItemAmount(CPlayerState::kIT_FragCount);
}

bool CGMDeathMatch::IsNearScoreLimit(const CStateManager& mgr, uint playerIndex) const {
  return mFragLimit - GetItemAmount(mgr, playerIndex) <= 1 && mFragLimit > 1;
}

int CGMDeathMatch::GetGameModeType() { return 'DTHM'; }

uint CGMDeathMatch::GetNumPlayers() const { return mPlayerCount; }
