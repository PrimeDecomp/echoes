#include "MetroidPrime/CRumbleManager.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/RumbleFxTable.hpp"

CRumbleManager::CRumbleManager(int playerIndex, EIOPort port)
: mPlayerIndex(playerIndex), mPort(port), mRumbleGenerator(port) {}

CRumbleManager::~CRumbleManager() { mRumbleGenerator.HardStopAll(); }

short CRumbleManager::Rumble(CStateManager& mgr, ERumbleFxId fx, float gain,
                             ERumblePriority priority) {
  CGameOptions& options = gpGameState->GameOptions();
  if (mgr.IsMultiplayer()) {
    if (options.GetIsPlayerRumbleEnabled(mPlayerIndex)) {
      return mRumbleGenerator.Rumble(skRumbleFxTable[fx], gain, priority);
    }
    return -1;
  }
  if (options.GetIsRumbleEnabled()) {
    return mRumbleGenerator.Rumble(skRumbleFxTable[fx], gain, priority);
  }
  return -1;
}

short CRumbleManager::Rumble(CStateManager& mgr, const CVector3f& pos, ERumbleFxId fx, float dist,
                             ERumblePriority priority) {
  if (!close_enough(dist, 0.f)) {
    CVector3f delta = mgr.GetPlayer(mPlayerIndex)->GetTranslation() - pos;
    if (delta.MagSquared() < dist * dist) {
      return Rumble(mgr, fx, 1.f - delta.Magnitude() / dist, priority);
    }
  }
  return -1;
}

void CRumbleManager::StopRumble(short id) {
  if (id == -1) {
    return;
  }
  mRumbleGenerator.Stop(id);
}

void CRumbleManager::Update(float dt) { mRumbleGenerator.Update(dt); }
