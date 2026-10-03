#include "MetroidPrime/Player/CPlayerListener.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CGameMode.hpp"
#include "MetroidPrime/Player/CGameState.hpp"

CPlayerListener::CPlayerListener(CStateManager& mgr, uint playerMask) : mPlayerMask(0) {
  CGameMode& mode = gpGameState->GetGameMode();

  for (uint i = 0; i < static_cast< uint >(mgr.GetNumPlayers()); ++i) {
    if (playerMask & (1u << i)) {
      mode.AddListener(*this, i);
      mPlayerMask |= 1u << i;
    }
  }
}

CPlayerListener::~CPlayerListener() {}

void CPlayerListener::KillListener(CStateManager& mgr) {
  CGameMode& mode = gpGameState->GetGameMode();

  if (mPlayerMask != 0) {
    for (uint i = 0; i < static_cast< uint >(mgr.GetNumPlayers()); ++i) {
      if (mPlayerMask & (1u << i)) {
        mode.RemoveListener(*this, i);
      }
    }

    mPlayerMask = 0;
  }
}

uint CPlayerListener::GetFirstPlayer(const CStateManager& mgr) const {
  for (uint i = 0; i < static_cast< uint >(mgr.GetNumPlayers()); ++i) {
    if (mPlayerMask & (1u << i)) {
      return i;
    }
  }

  return kInvalidPlayer;
}

uint CPlayerListener::GetNextPlayer(const CStateManager& mgr, uint playerIndex) const {
  if (playerIndex == kInvalidPlayer) {
    return kInvalidPlayer;
  }

  for (uint i = playerIndex + 1; i < static_cast< uint >(mgr.GetNumPlayers()); ++i) {
    if (mPlayerMask & (1u << i)) {
      return i;
    }
  }

  return kInvalidPlayer;
}
