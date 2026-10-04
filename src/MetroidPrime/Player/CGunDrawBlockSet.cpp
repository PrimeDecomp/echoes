#include "MetroidPrime/Player/CGunDrawBlockSet.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"

CGunDrawBlockSet::CGunDrawBlockSet() : mBlockedPlayers(0) {}

CGunDrawBlockSet::~CGunDrawBlockSet() {}

void CGunDrawBlockSet::AddPlayer(CStateManager& mgr, int playerIndex, bool holsterGun) {
  const uint playerBit = 1u << playerIndex;
  if ((mBlockedPlayers & playerBit) == 0) {
    CPlayerGun* gun = mgr.GetPlayer(playerIndex)->GetPlayerGun();
    if (holsterGun) {
      gun->HolsterGun(mgr);
    }
    gun->AddGunDrawBlock();
    mBlockedPlayers |= playerBit;
  }
}

void CGunDrawBlockSet::RemovePlayer(CStateManager& mgr, int playerIndex) {
  const uint playerBit = 1u << playerIndex;
  if ((mBlockedPlayers & playerBit) != 0) {
    mgr.GetPlayer(playerIndex)->GetPlayerGun()->RemoveGunDrawBlock();
    mBlockedPlayers &= ~playerBit;
  }
}

void CGunDrawBlockSet::Clear(CStateManager& mgr) {
  for (uint i = 0; mBlockedPlayers != 0 && i < 4; ++i) {
    RemovePlayer(mgr, i);
  }
}
