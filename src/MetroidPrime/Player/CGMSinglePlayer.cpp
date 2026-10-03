#include "MetroidPrime/Player/CGMSinglePlayer.hpp"

#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

CGMSinglePlayer::CGMSinglePlayer()
: mPlayerSpawned(false), mGameOver(false), mResultIndex(kRI_Unset) {}

void CGMSinglePlayer::PutTo(COutputStream&) const {}

void CGMSinglePlayer::Update(float, CStateManager& mgr) {
  if (mgr.GetPlayer(0)->IsPlayerDeadEnough(mgr)) {
    mGameOver = true;
  }
}

void CGMSinglePlayer::OnPlayerKilled(CStateManager&, TUniqueId, TUniqueId) {}

void CGMSinglePlayer::NotifyGenericEvent(CStateManager&, TUniqueId, uint) {}

void CGMSinglePlayer::RespawnPlayer(CStateManager&, uint) {}

void CGMSinglePlayer::OnPlayerSpawned(CStateManager&, uint) { mPlayerSpawned = true; }

bool CGMSinglePlayer::IsGameOver() { return mGameOver; }

void CGMSinglePlayer::OnPlayerDamaged(CStateManager&, TUniqueId, TUniqueId, float) {}

void CGMSinglePlayer::NotifyStop(CStateManager&, TUniqueId) {}

CGMSinglePlayer::EResultIndex CGMSinglePlayer::CalculateResult(const CStateManager& mgr) {
  const int itemPercentage = mgr.GetPlayerState(0)->GetItemPercentageRatio();
  if (itemPercentage < 75) {
    return kRI_Under75Percent;
  }
  if (itemPercentage < 100) {
    return kRI_Under100Percent;
  }
  return kRI_AtLeast100Percent;
}

void CGMSinglePlayer::EndGame(int reason, CStateManager& mgr) {
  mResultIndex = CalculateResult(mgr);
  if (reason > 0) {
    gpMain->SetRestartMode(CMain::kRM_EndAutoSave);
  } else {
    switch (mResultIndex) {
    case kRI_Under75Percent:
      gpMain->SetRestartMode(CMain::kRM_Credits1);
      break;
    case kRI_Under100Percent:
      gpMain->SetRestartMode(CMain::kRM_Credits2);
      break;
    case kRI_AtLeast100Percent:
      gpMain->SetRestartMode(CMain::kRM_EndMovie1);
      break;
    }
  }
  mGameOver = true;
  mgr.QuitGame();
}

int CGMSinglePlayer::GetResultIndex() const { return mResultIndex; }

void CGMSinglePlayer::GiveScore(CStateManager&, uint, uint) {}

void CGMSinglePlayer::AddListener(CPlayerListener&, uint) {}

void CGMSinglePlayer::RemoveListener(CPlayerListener&, uint) {}

bool CGMSinglePlayer::v21() const { return false; }

void CGMSinglePlayer::SetSpawnPoint(uint, TUniqueId) {}

int CGMSinglePlayer::GetItemAmount(const CStateManager&, uint) const { return 0; }

bool CGMSinglePlayer::IsNearScoreLimit(const CStateManager&, uint) const { return false; }

bool CGMSinglePlayer::IsMultiplayer() const { return false; }

float CGMSinglePlayer::GetMatchTimeLimit() const { return -1.f; }

float CGMSinglePlayer::GetElapsedTime() const { return -1.f; }

int CGMSinglePlayer::GetGameModeType() { return 'SNGL'; }

uint CGMSinglePlayer::GetNumPlayers() const { return 1; }

CGMSinglePlayer::~CGMSinglePlayer() {}
