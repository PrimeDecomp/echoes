#include "MetroidPrime/Player/CGMMultiplayer.hpp"

#include "Kyoto/Streams/COutputStream.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayerListener.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpawnPoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "rstl/vector.hpp"

CGMMultiplayer::CGMMultiplayer(float timeLimit, bool flag)
: mTimeLimit(timeLimit)
, mElapsedTime(0.f)
, mMusicIndex(0)
, mSpawnPoints(4, kInvalidUniqueId)
, mResultIndex(-1)
, x34_24_(flag)
, mGameOver(false) {}

void CGMMultiplayer::PutTo(COutputStream& out) const {
  out.WriteBool(x34_24_);
  out.WriteReal32(mTimeLimit);
}

void CGMMultiplayer::OnPlayerDamaged(CStateManager& mgr, TUniqueId victim, TUniqueId attacker,
                                     float damage) {
  uint victimIndex = -1;
  if (TCastToConstPtr< CPlayer >(mgr.GetObjectById(victim)) != nullptr) {
    victimIndex = mgr.MaskUIdNumPlayers(victim);
  }
  uint attackerIndex = -1;
  if (TCastToConstPtr< CPlayer >(mgr.GetObjectById(attacker)) != nullptr) {
    attackerIndex = mgr.MaskUIdNumPlayers(attacker);
  }

  NotifyListeners(mgr, attackerIndex, victimIndex, kGE_Damage, &damage);
}

void CGMMultiplayer::NotifyStop(CStateManager& mgr, TUniqueId player) {
  uint playerIndex = -1;
  if (TCastToConstPtr< CPlayer >(mgr.GetObjectById(player)) != nullptr) {
    playerIndex = mgr.MaskUIdNumPlayers(player);
  }
  NotifyListeners(mgr, playerIndex, playerIndex, kGE_Stop, nullptr);
}

void CGMMultiplayer::OnPlayerKilled(CStateManager& mgr, TUniqueId victim, TUniqueId killer) {
  uint victimIndex = -1;
  if (TCastToConstPtr< CPlayer >(mgr.GetObjectById(victim)) != nullptr) {
    victimIndex = mgr.MaskUIdNumPlayers(victim);
  }
  uint killerIndex = -1;
  if (TCastToConstPtr< CPlayer >(mgr.GetObjectById(killer)) != nullptr) {
    killerIndex = mgr.MaskUIdNumPlayers(killer);
  }

  NotifyListeners(mgr, victimIndex, killerIndex, kGE_Score, nullptr);
  NotifyListeners(mgr, killerIndex, victimIndex, kGE_Kill, nullptr);
}

void CGMMultiplayer::OnPlayerScanned(CStateManager& mgr, TUniqueId target, TUniqueId scanner) {
  uint targetIndex = -1;
  if (TCastToConstPtr< CPlayer >(mgr.GetObjectById(target)) != nullptr) {
    targetIndex = mgr.MaskUIdNumPlayers(target);
  }
  uint scannerIndex = -1;
  if (TCastToConstPtr< CPlayer >(mgr.GetObjectById(scanner)) != nullptr) {
    scannerIndex = mgr.MaskUIdNumPlayers(scanner);
  }

  NotifyListeners(mgr, scannerIndex, targetIndex, kGE_Scan, nullptr);
}

void CGMMultiplayer::NotifyGenericEvent(CStateManager& mgr, TUniqueId player, uint value) {
  uint playerIndex = -1;
  if (TCastToConstPtr< CPlayer >(mgr.GetObjectById(player)) != nullptr) {
    playerIndex = mgr.MaskUIdNumPlayers(player);
  }
  NotifyListeners(mgr, playerIndex, playerIndex, kGE_Generic, &value);
}

void CGMMultiplayer::AddListener(CPlayerListener& listener, uint playerIndex) {
  mListeners.insert(TListener(playerIndex, &listener));
}

void CGMMultiplayer::RemoveListener(CPlayerListener& listener, uint playerIndex) {
  mListeners.erase(TListener(playerIndex, &listener));
}

void CGMMultiplayer::NotifyListeners(CStateManager& mgr, uint sourceIndex, uint targetIndex,
                                     EGameEvent event, const void* value) {
  if (targetIndex == uint(-1)) {
    return;
  }
  for (rstl::set< TListener >::iterator it = mListeners.begin(); it != mListeners.end(); ++it) {
    if (it->first == targetIndex) {
      it->second->OnGameEvent(mgr, sourceIndex, targetIndex, event, value);
    }
  }
}

TUniqueId CGMMultiplayer::ChooseSpawnPoint(CStateManager& mgr, uint playerIndex,
                                           TUniqueId requested) {
  rstl::vector< TUniqueId > candidates;
  candidates.reserve(16);
  CObjectList& objects = mgr.ObjectListById(kOL_All);
  for (int index = objects.GetFirstObjectIndex(); index != -1;
       index = objects.GetNextObjectIndex(index)) {
    const CScriptSpawnPoint* spawn = TCastToPtr< CScriptSpawnPoint >(objects[index]);
    if (spawn == nullptr || (requested != kInvalidUniqueId && spawn->GetUniqueId() != requested)) {
      continue;
    }

    const CVector3f position = spawn->GetTransform().GetTranslation();
    float nearestDistance = 1000000.f;
    for (uint player = 0; player < GetNumPlayers(); ++player) {
      if (player != playerIndex) {
        const float distance = (mgr.GetPlayer(player)->GetTranslation() - position).Magnitude();
        if (distance < nearestDistance) {
          nearestDistance = distance;
        }
      }
    }
    if (nearestDistance >= 7.f && spawn->GetActive() && spawn->IsFirstSpawn()) {
      candidates.push_back(spawn->GetUniqueId());
    }
  }

  if (candidates.empty()) {
    return kInvalidUniqueId;
  }
  return candidates[mgr.Random()->Range(0, candidates.size() - 1)];
}

void CGMMultiplayer::RespawnPlayer(CStateManager& mgr, uint playerIndex) {
  const TUniqueId spawnId = ChooseSpawnPoint(mgr, playerIndex, mSpawnPoints[playerIndex]);
  if (spawnId == kInvalidUniqueId) {
    return;
  }

  CScriptSpawnPoint& spawn = static_cast< CScriptSpawnPoint& >(*mgr.ObjectById(spawnId));
  CPlayerState& state = *mgr.PlayerState(playerIndex);
  CPlayer& player = *mgr.Player(playerIndex);
  const CPlayerState::SPersistentState persistent = state.GetPersistentState();
  state = CPlayerState(playerIndex, nullptr);
  for (int i = 0; i < CPlayerState::kIT_Max; ++i) {
    const CPlayerState::EItemType item = CPlayerState::EItemType(i);
    if (state.GetItemCapacity2(item) != spawn.GetItemCapacity(item)) {
      state.AddPowerUp(item, spawn.GetItemCapacity(item) - state.GetItemCapacity2(item));
    }
    if (state.GetItemAmount(item) != spawn.GetItemAmount(item)) {
      state.IncrPickUp(item, spawn.GetItemAmount(item) - state.GetItemAmount(item));
    }
  }
  state.SetPersistentState(persistent);

  const CVector3f position = spawn.GetTransform().GetTranslation();
  CVector3f forward = spawn.GetTransform().GetForward();
  forward.SetZ(0.f);
  if (forward.CanBeNormalized()) {
    player.Teleport(CTransform4f::LookAt(position, position + forward, CVector3f::Up()), mgr, true);
  }
  player.AsyncLoadSuit(mgr);
  player.SetSpawnedMorphBallState(spawn.IsMorphed() ? CPlayer::kMS_Morphed : CPlayer::kMS_Unmorphed,
                                  mgr);
  player.ResetPlayerState(mgr, 1);
  spawn.SendSpawnMessage(mgr, player);
  NotifyListeners(mgr, playerIndex, playerIndex, kGE_Spawn, nullptr);
}

void CGMMultiplayer::OnPlayerSpawned(CStateManager& mgr, uint playerIndex) {
  NotifyListeners(mgr, playerIndex, playerIndex, kGE_Spawn, nullptr);
}

void CGMMultiplayer::SetSpawnPoint(uint playerIndex, TUniqueId spawnPoint) {
  mSpawnPoints[playerIndex] = spawnPoint;
}

bool CGMMultiplayer::v21() const { return x34_24_; }

void CGMMultiplayer::EndGame(int resultIndex, CStateManager&) {
  mGameOver = true;
  mResultIndex = resultIndex;
}

int CGMMultiplayer::GetResultIndex() const { return mResultIndex; }

bool CGMMultiplayer::IsGameOver() { return mGameOver; }

void CGMMultiplayer::UpdateTimer(float dt, CStateManager& mgr) {
  if (!mgr.GetCameraManager(0)->IsInFullScreenCinematic()) {
    mElapsedTime += dt;
  }
}

bool CGMMultiplayer::IsMultiplayer() const { return true; }

void CGMMultiplayer::SetMusicIndex(int musicIndex) { mMusicIndex = musicIndex; }
