#include "MetroidPrime/ScriptObjects/CScriptPlayerProxy.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/HUD/CHUDMemoParms.hpp"
#include "MetroidPrime/HUD/CSamusHud.hpp"
#include "MetroidPrime/Player/CGMCoin.hpp"
#include "MetroidPrime/Player/CGMMultiplayer.hpp"
#include "MetroidPrime/Player/CGameMode.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlayerController.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "REL/REL_Setup.h"
#include "rstl/StringExtras.hpp"

#include <math.h>

static EMaterialTypes sDamageMaterial = kMT_Solid; // Guessed name.

CScriptPlayerProxy::~CScriptPlayerProxy() {}

void CScriptPlayerProxy::CListener::OnGameEvent(CStateManager& mgr, uint sourceIndex,
                                                uint targetIndex, uint event, const void* value) {
  mOwner->OnGameEvent(mgr, sourceIndex, targetIndex, event, value);
}

void CScriptPlayerProxy::ClearDamageOverTime(CStateManager& mgr) {
  mDamageOverTime.clear();
  mDamageActive = false;
}

void CScriptPlayerProxy::AddDamageOverTime(uint sourceIndex, TUniqueId attacker,
                                           CStateManager& mgr) {
  float damage = mgr.Random()->Range(mCurrentMinDamage, mCurrentMaxDamage);
  if (sourceIndex < mgr.GetNumPlayers() && mDamageOverTime.size() != 8) {
    mDamageOverTime.push_back(
        SDamageOverTime(mCurrentDuration, damage / mCurrentDuration, attacker));
  }
}

void CScriptPlayerProxy::UpdateDamageOverTime(float dt, CStateManager& mgr) {
  uint firstPlayer = mListener.GetFirstPlayer(mgr);
  if (firstPlayer > mgr.GetNumPlayers()) {
    return;
  }

  CPlayer* player = mgr.GetPlayer(firstPlayer);
  SDamageOverTime* it = mDamageOverTime.begin();
  while (it != mDamageOverTime.end()) {
    if (it->mTimeRemaining > 0.f) {
      it->mTimeRemaining -= dt;
      CDamageInfo info(CWeaponMode(EWeaponType(mCurrentWeaponType)), it->mDamagePerSecond * dt, 0.f,
                       0.f, true);
      info.SetDamageLoopSfxId(0x458);
      mgr.ApplyDamage(
          GetUniqueId(), player->GetUniqueId(), it->mAttacker, info,
          CMaterialFilter::MakeIncludeExclude(CMaterialList(sDamageMaterial), CMaterialList()),
          CVector3f::Zero());
      if (mDamageOverTime.empty()) {
        break;
      }
      if (it->mTimeRemaining <= 0.f) {
        it = mDamageOverTime.erase(it);
      } else {
        ++it;
      }
    } else {
      ++it;
    }
  }

  if (mDamageActive && mDamageOverTime.empty()) {
    SendScriptMsgs(kSS_Exited, mgr, player->GetUniqueId());
    mDamageActive = false;
  } else if (!mDamageActive && !mDamageOverTime.empty()) {
    SendScriptMsgs(kSS_Entered, mgr, player->GetUniqueId());
    mDamageActive = true;
  }
}

bool CScriptPlayerProxy::NotifyProxies(CStateManager& mgr, CPlayer& player,
                                       EScriptObjectState state) {
  bool sent = false;
  const CObjectList& list = mgr.GetObjectListById(kOL_All);
  for (int idx = list.GetFirstObjectIndex(); idx != -1; idx = list.GetNextObjectIndex(idx)) {
    const CScriptPlayerProxy* proxy = TCastToConstPtr< CScriptPlayerProxy >(list[idx]);
    if (proxy != nullptr && proxy->GetActive() && proxy->mProxyType == kPT_PlayerMessageRelay &&
        (proxy->GetPlayerMask() & (1 << player.GetPlayerIndex())) != 0) {
      const rstl::vector< SConnection >& connections = proxy->GetConnectionList();
      for (rstl::vector< SConnection >::const_iterator conn = connections.begin();
           conn != connections.end(); ++conn) {
        if (conn->state == state) {
          TUniqueId target = mgr.GetIdForScript(conn->objId);
          mgr.SendScriptMsg(
              CScriptMsg(player.GetUniqueId(), target, conn->msg, player.GetUniqueId()));
          sent = true;
        }
      }
      break;
    }
  }
  return sent;
}

uint CScriptPlayerProxy::GetPlayerMask() const { return mListener.GetPlayerMask(); }

void CScriptPlayerProxy::AddToRenderer(const CStateManager& mgr) const {
  uint firstPlayer = mListener.GetFirstPlayer(mgr);
  if ((mProxyType == kPT_PlayerFollower && mFollowing) ||
      (mProxyType == kPT_ArchenemyMarker && GetArchenemyIndex(mgr) != kInvalidPlayerIndex &&
       firstPlayer == mgr.GetCurrentRenderPlayerIndex())) {
    if (mgr.GetPlayerState(firstPlayer)->GetItemAmount(CPlayerState::kIT_Invisibility) == 0) {
      CActor::AddToRenderer(mgr);
    }
  }
}

void CScriptPlayerProxy::SendStateToPlayer(EScriptObjectState state, CStateManager& mgr,
                                           uint playerIndex) {
  UpdateTransform(mgr, playerIndex);
  SendScriptMsgs(state, mgr, GetUniqueId());
}

void CScriptPlayerProxy::UpdateTransform(CStateManager& mgr, uint playerIndex) {
  if (playerIndex == kInvalidPlayerIndex) {
    return;
  }

  CPlayer* player = mgr.GetPlayer(playerIndex);
  CTransform4f xf = CTransform4f::Identity();
  if (mProxyType == kPT_ArchenemyMarker) {
    CVector3f rotated = player->GetTransform().Rotate(mPlayerOffset);
    xf.SetTranslation(rotated + player->GetTranslation());
    xf.RotateLocalZ(CRelAngle::FromDegrees(360.f * (float(fmod(mTime, 5.0)) / 5.f)));
  } else if (mProxyType == kPT_PlayerFollower) {
    if (mVectorParameter1.IsNonZero()) {
      xf.SetTranslation(mPlayerOffset + player->GetTranslation());
      xf.RotateLocalZ(CRelAngle::FromDegrees(mTime * mVectorParameter1.GetZ()));
      xf.RotateLocalY(CRelAngle::FromDegrees(mTime * mVectorParameter1.GetY()));
      xf.RotateLocalX(CRelAngle::FromDegrees(mTime * mVectorParameter1.GetX()));
    } else {
      CVector3f rotated = player->GetTransform().Rotate(mPlayerOffset);
      xf.SetTranslation(rotated + player->GetTranslation());
    }
  } else {
    xf.SetTranslation(mPlayerOffset + player->GetTranslation());
  }
  SetTransform(xf);
}

void CScriptPlayerProxy::SetArchenemyIndex(CStateManager& mgr, uint playerIndex) {
  uint firstPlayer = mListener.GetFirstPlayer(mgr);
  if (firstPlayer != kInvalidPlayerIndex) {
    mgr.PlayerState(firstPlayer)
        ->SetItemAmount(CPlayerState::kIT_Multiplayer_Archenemy, playerIndex + 1);
  }
}

uint CScriptPlayerProxy::GetArchenemyIndex(const CStateManager& mgr) const {
  uint firstPlayer = mListener.GetFirstPlayer(mgr);
  uint archenemy = kInvalidPlayerIndex;
  if (firstPlayer != archenemy) {
    archenemy =
        mgr.GetPlayerState(firstPlayer)->GetItemAmount(CPlayerState::kIT_Multiplayer_Archenemy) - 1;
  }
  return archenemy;
}

void CScriptPlayerProxy::CheckVisors(CStateManager& mgr) {
  uint playerMask = mListener.GetPlayerMask();
  for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
    if ((1 << i & playerMask) != 0) {
      CPlayerState::EPlayerVisor visor = mgr.GetPlayerState(i)->GetActiveVisor(mgr);
      uint bit = 1u << i;
      bool inVisor = visor == mIntParameter2;
      bool wasInVisor = (mIntParameter1 & bit) != 0;
      if (inVisor != wasInVisor) {
        SendStateToPlayer(inVisor ? kSS_Entered : kSS_Exited, mgr, i);
        if (inVisor) {
          mIntParameter1 |= bit;
        } else {
          mIntParameter1 &= ~bit;
        }
      }
    }
  }
}

void CScriptPlayerProxy::InitVisors(CStateManager& mgr) {
  uint playerMask = mListener.GetPlayerMask();
  for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
    if ((1 << i & playerMask) != 0) {
      CPlayerState::EPlayerVisor visor = mgr.GetPlayerState(i)->GetActiveVisor(mgr);
      uint bit = 1u << i;
      if (visor != mIntParameter2) {
        mIntParameter1 |= bit;
      } else {
        mIntParameter1 &= ~bit;
      }
    }
  }
  CheckVisors(mgr);
}

void CScriptPlayerProxy::DropCoins(CStateManager& mgr, int amount, uint playerIndex, bool amplified,
                                   int denominationMode) {
  CPlayerState* state = mgr.PlayerState(playerIndex);
  int available = state->GetItemAmount(CPlayerState::kIT_CoinCounter);
  if (available < amount) {
    amount = available;
  }

  int remaining = amount;
  state->DecrPickUp(CPlayerState::kIT_CoinCounter, amount);
  if (amplified) {
    remaining = (amount + 1) / 2;
  }

  int count100 = 0;
  int count50 = 0;
  int count10 = 0;
  int count5 = 0;
  if (denominationMode == 1) {
    count100 = remaining / 100;
    remaining -= count100 * 100;
    count50 = remaining / 50;
    remaining -= count50 * 50;
    count10 = remaining / 10;
    remaining -= count10 * 10;
    count5 = remaining / 5;
    remaining -= count5 * 5;
  } else if (denominationMode == 2) {
    count100 = remaining / 200;
    remaining -= count100 * 100;
    count50 = remaining / 100;
    remaining -= count50 * 50;
    count10 = remaining / 20;
    remaining -= count10 * 10;
    count5 = remaining / 10;
    remaining -= count5 * 5;
  }

  UpdateTransform(mgr, playerIndex);
  for (int i = 0; i < count100; ++i) {
    SendScriptMsgs(kSS_BIDG, mgr, GetUniqueId());
  }
  for (int i = 0; i < count50; ++i) {
    SendScriptMsgs(kSS_BXDG, mgr, GetUniqueId());
  }
  for (int i = 0; i < count10; ++i) {
    SendScriptMsgs(kSS_IceXDamage, mgr, GetUniqueId());
  }
  for (int i = 0; i < count5; ++i) {
    SendScriptMsgs(kSS_XDamage, mgr, GetUniqueId());
  }
  for (int i = 0; i < remaining; ++i) {
    SendScriptMsgs(kSS_Damage, mgr, GetUniqueId());
  }
}

void CScriptPlayerProxy::SetVisorFromProxyType() {
  switch (mProxyType) {
  case kPT_CombatVisorWatcher:
    mIntParameter2 = CPlayerState::kPV_Combat;
    break;
  case kPT_ScanVisorWatcher:
    mIntParameter2 = CPlayerState::kPV_Scan;
    break;
  case kPT_EchoVisorWatcher:
    mIntParameter2 = CPlayerState::kPV_Echo;
    break;
  case kPT_DarkVisorWatcher:
    mIntParameter2 = CPlayerState::kPV_Dark;
    break;
  default:
    mIntParameter2 = CPlayerState::kPV_Invalid;
    break;
  }
}

void CScriptPlayerProxy::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  mTime += dt;
  if (mTime > 900.f) {
    mTime -= 900.f;
  }

  switch (mProxyType) {
  case kPT_PlayerFollower:
    if (mFollowing) {
      UpdateTransform(mgr, mListener.GetFirstPlayer(mgr));
    }
    break;
  case kPT_ArchenemyMarker: {
    uint archenemy = GetArchenemyIndex(mgr);
    if (archenemy != kInvalidPlayerIndex) {
      UpdateTransform(mgr, archenemy);
    }
    break;
  }
  case kPT_CoinCollector: {
    uint firstPlayer = mListener.GetFirstPlayer(mgr);
    if (mCoinsToDrop != 0) {
      if (mCoinDropCooldown > 0) {
        --mCoinDropCooldown;
        return;
      }

      if (mCoinsToDrop > 1000) {
        mCoinsToDrop = 1000;
      }

      int chunk = 10;
      if (mCoinsToDrop > 100) {
        chunk = 100;
      } else if (mCoinsToDrop > 50) {
        chunk = 50;
      }
      int amount = chunk;
      if (mCoinsToDrop < chunk) {
        amount = mCoinsToDrop;
      }

      bool amplified = false;
      for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
        if (mgr.PlayerState(i)->GetItemAmount(CPlayerState::kIT_CoinAmplifier) != 0) {
          amplified = true;
          break;
        }
      }

      DropCoins(mgr, amount, firstPlayer, amplified, 1);
      mCoinsToDrop -= amount;
      mCoinDropCooldown = 6;
    }

    if (mPendingCoins != 0) {
      CPlayerState* state = mgr.PlayerState(firstPlayer);
      if (state->GetHealthInfo().GetHP() > 0.f) {
        state->IncrPickUp(CPlayerState::kIT_CoinCounter, mPendingCoins);
        mPendingCoins = 0;
      }
    }

    const CGameState& gameState = *gpGameState;
    CGameMode& mode = gameState.GetGameMode();
    if (mode.GetGameModeType() == 'COIN') {
      firstPlayer = mListener.GetFirstPlayer(mgr);
      if (firstPlayer != kInvalidPlayerIndex) {
        static_cast< CGMCoin& >(mode).SetCanRespawn(firstPlayer, mCoinsToDrop == 0);
      }
    }
    break;
  }
  case kPT_CombatVisorWatcher:
  case kPT_ScanVisorWatcher:
  case kPT_DarkVisorWatcher:
  case kPT_EchoVisorWatcher:
    CheckVisors(mgr);
    break;
  case kPT_DamageOverTime:
    UpdateDamageOverTime(dt, mgr);
    break;
  }
}

void CScriptPlayerProxy::HandleDamageOverTimeMessage(EScriptObjectMessage msg, TUniqueId sender,
                                                     CStateManager& mgr, uint playerIndex) {
  if (msg == kSM_AreaLoaded) {
    mCurrentDuration = mFloatParameter1;
    mCurrentMinDamage = mFloatParameter2;
    mCurrentMaxDamage = mFloatParameter3;
    mCurrentWeaponType = mIntParameter1;
  }
}

void CScriptPlayerProxy::HandleVisorMessage(EScriptObjectMessage msg, TUniqueId sender,
                                            CStateManager& mgr, uint playerIndex) {
  if (msg == kSM_AreaLoaded) {
    InitVisors(mgr);
  }
}

void CScriptPlayerProxy::HandleMessage(EScriptObjectMessage msg, TUniqueId sender,
                                       CStateManager& mgr, uint playerIndex) {
  const CGameState& gameState = *gpGameState;
  CGameMode& mode = gameState.GetGameMode();
  switch (msg) {
  case kSM_Increment:
    mode.GiveScore(mgr, playerIndex, 1);
    break;
  case kSM_Decrement:
    mode.GiveScore(mgr, playerIndex, -1);
    break;
  case kSM_SetToZero:
    mgr.ApplyDamage(kInvalidUniqueId, mgr.GetPlayer(playerIndex)->GetUniqueId(), kInvalidUniqueId,
                    CDamageInfo(CWeaponMode(), 1000000.f, 0.f, 0.f, false, false),
                    CMaterialFilter::GetPassEverything(), CVector3f::Zero());
    break;
  case kSM_SetToMax:
    mode.EndGame(playerIndex, mgr);
    break;
  }
}

void CScriptPlayerProxy::HandleRespawnMessage(EScriptObjectMessage msg, TUniqueId sender,
                                              CStateManager& mgr, uint playerIndex) {
  const CGameState& gameState = *gpGameState;
  CGameMode& mode = gameState.GetGameMode();
  if (msg == kSM_SetToZero) {
    mode.RespawnPlayer(mgr, playerIndex);
    CPlayerState* state = mgr.PlayerState(playerIndex);
    for (int item = CPlayerState::kIT_PersistentCounter1;
         item < CPlayerState::kIT_PersistentCounter5; ++item) {
      state->SetItemAmount(CPlayerState::EItemType(item), 0);
    }
  } else {
    HandleMessage(msg, sender, mgr, playerIndex);
  }
}

void CScriptPlayerProxy::HandleFollowerMessage(EScriptObjectMessage msg, TUniqueId sender,
                                               CStateManager& mgr, uint playerIndex) {
  switch (msg) {
  case kSM_Increment:
    mFollowing = true;
    break;
  case kSM_Decrement:
    mFollowing = false;
    break;
  }
}

void CScriptPlayerProxy::HandleArchenemyMessage(EScriptObjectMessage msg, TUniqueId sender,
                                                CStateManager& mgr, uint playerIndex) {
  if (msg == kSM_Action) {
    if (GetArchenemyIndex(mgr) == kInvalidPlayerIndex) {
      uint pick = mgr.Random()->Range(0, int(mgr.GetNumPlayers()) - 1);
      if (pick == mListener.GetFirstPlayer(mgr)) {
        pick = (pick + 1) % mgr.GetNumPlayers();
      }
      SetArchenemyIndex(mgr, pick);
      CSamusHud::DisplayHudMemo(
          CStringExtras::ConvertToUNICODE(rstl::string(
              CBasics::Stringize("ARCHENEMY!\nDefeat player %d for\nBONUS SCORE!\n", pick + 1))),
          CHUDMemoParms(5.f, true, false, false, 1 << mListener.GetFirstPlayer(mgr), true));
    }
  } else {
    HandleMessage(msg, sender, mgr, playerIndex);
  }
}

void CScriptPlayerProxy::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  TUniqueId sender = msg.GetSenderId();
  EScriptObjectMessage type = msg.GetMessage();
  const CGameState& gameState = *gpGameState;
  CGameMode& mode = gameState.GetGameMode();
  switch (type) {
  case kSM_Create:
    switch (mProxyType) {
    case kPT_ArchenemyMarker:
      if (!mode.v21()) {
        mgr.DeleteObjectRequest(GetUniqueId());
      }
      break;
    case kPT_CombatVisorWatcher:
    case kPT_ScanVisorWatcher:
    case kPT_DarkVisorWatcher:
    case kPT_EchoVisorWatcher:
      SetVisorFromProxyType();
      break;
    }
    break;
  case kSM_Delete:
    mListener.KillListener(mgr);
    break;
  case kSM_AreaLoaded:
    mActorCount = mgr.ObjectListById(kOL_All).size();
    // fallthrough
  default:
    if (GetActive() && mProxyType >= 0 && mProxyType <= 10) {
      for (int i = mListener.GetFirstPlayer(mgr); i != -1; i = mListener.GetNextPlayer(mgr, i)) {
        switch (mProxyType) {
        case kPT_PersistentCounterReset:
          HandleRespawnMessage(type, sender, mgr, i);
          break;
        case kPT_PlayerFollower:
          HandleFollowerMessage(type, sender, mgr, i);
          break;
        case kPT_ArchenemyMarker:
          HandleArchenemyMessage(type, sender, mgr, i);
          break;
        case kPT_CombatVisorWatcher:
        case kPT_ScanVisorWatcher:
        case kPT_DarkVisorWatcher:
        case kPT_EchoVisorWatcher:
          HandleVisorMessage(type, sender, mgr, i);
          break;
        case kPT_DamageOverTime:
          HandleDamageOverTimeMessage(type, sender, mgr, i);
          break;
        default:
          HandleMessage(type, sender, mgr, i);
          break;
        }
      }
    }
    break;
  }
  CActor::AcceptScriptMsg(mgr, msg);
}

void CScriptPlayerProxy::OnGameEvent(CStateManager& mgr, uint sourceIndex, uint targetIndex,
                                     uint event, const void* value) {
  if (!GetActive()) {
    return;
  }

  const CGameState& gameState = *gpGameState;
  CGameMode& mode = gameState.GetGameMode();
  CPlayer* player = mgr.GetPlayer(targetIndex);
  switch (mProxyType) {
  case kPT_EventRelay:
  case kPT_PersistentCounterReset: {
    CPlayerState* state = mgr.PlayerState(targetIndex);
    int newState;
    switch (event) {
    case CGMMultiplayer::kGE_Damage:
      newState = kSS_Damage;
      break;
    case CGMMultiplayer::kGE_Kill:
      newState = kSS_Dead;
      if (mFloatParameter2 > 0.f && -1.f * state->GetHealthInfo().GetHP() > mFloatParameter2) {
        SendStateToPlayer(kSS_XDamage, mgr, targetIndex);
      }
      break;
    case CGMMultiplayer::kGE_Score:
      newState = kSS_MaxReached;
      break;
    case CGMMultiplayer::kGE_Spawn:
      newState = kSS_Entered;
      break;
    default:
      newState = -1;
      break;
    }

    if (mProxyType == kPT_PersistentCounterReset) {
      int count = 0;
      for (int item = CPlayerState::kIT_PersistentCounter1;
           item < CPlayerState::kIT_PersistentCounter5; ++item) {
        if (state->GetItemAmount(CPlayerState::EItemType(item)) != 0) {
          ++count;
        }
      }
      if (count > 0) {
        int pick = mgr.Random()->Next() % count;
        int seen = 0;
        for (int item = CPlayerState::kIT_PersistentCounter1;
             item < CPlayerState::kIT_PersistentCounter5; ++item) {
          if (state->GetItemAmount(CPlayerState::EItemType(item)) != 0) {
            ++seen;
            if (seen == pick) {
              state->SetItemAmount(CPlayerState::EItemType(item), 0);
              break;
            }
          }
        }
      }
    }

    if (newState != -1) {
      SendStateToPlayer(EScriptObjectState(newState), mgr, targetIndex);
    }
    break;
  }
  case kPT_CoinCollector:
    if (event == CGMMultiplayer::kGE_Kill || event == CGMMultiplayer::kGE_Damage) {
      CPlayerState* state = mgr.PlayerState(targetIndex);
      CGMCoin& coin = static_cast< CGMCoin& >(mode);
      if (event == CGMMultiplayer::kGE_Damage) {
        mAccumulatedDamage += *static_cast< const float* >(value);
      }

      int owned = state->GetItemAmount(CPlayerState::kIT_CoinCounter) - mCoinsToDrop;
      int available = owned > 0 ? owned : 0;
      int limit = coin.GetCoinLimit() > 0 ? coin.GetCoinLimit() * 100 / 80 : 0xFFFFF;
      float multiplier;
      if (available >= limit) {
        multiplier = 1.5f;
      } else {
        multiplier = 1.f;
      }
      if (coin.IsNearTimeLimit()) {
        multiplier = 2.f;
      }

      if (event == CGMMultiplayer::kGE_Kill) {
        bool overkill = false;
        if (mFloatParameter2 > 0.f && -1.f * state->GetHealthInfo().GetHP() > mFloatParameter2) {
          overkill = true;
        }
        if (overkill) {
          multiplier *= 2.f;
        }
        int amount = int(float(mIntParameter2) * multiplier);
        if (available < amount) {
          amount = available;
        }
        mCoinsToDrop += amount;
        mAccumulatedDamage = 0.f;
        mPendingCoins += mIntParameter1;
      } else {
        int amount = int(multiplier * float(int(mAccumulatedDamage / mFloatParameter1)));
        if (amount < available) {
          available = amount;
        }
        if (available != 0) {
          mAccumulatedDamage = 0.f;
          mCoinsToDrop += available;
        }
      }
    }
    break;
  case kPT_ArchenemyMarker:
    if (event == CGMMultiplayer::kGE_Score) {
      if (targetIndex == mListener.GetFirstPlayer(mgr) && sourceIndex == GetArchenemyIndex(mgr)) {
        CSamusHud::DisplayHudMemo(rstl::wstring_l(L"BONUS!\nYou defeated your archenemy!\n"),
                                  CHUDMemoParms(5.f, true, false, false, 1 << targetIndex, true));
        mode.GiveScore(mgr, targetIndex, 1);
        SetArchenemyIndex(mgr, kInvalidPlayerIndex);
        SendScriptMsgs(kSS_Arrived, mgr);
      }
    }
    break;
  case kPT_PlayerMessageRelay:
    switch (event) {
    case CGMMultiplayer::kGE_Spawn:
      NotifyProxies(mgr, *player, kSS_AboutToMassivelyDie);
      break;
    case CGMMultiplayer::kGE_Generic:
      NotifyProxies(mgr, *player, EScriptObjectState(*static_cast< const uint* >(value)));
      break;
    }
    break;
  case kPT_DamageOverTime:
    switch (event) {
    case CGMMultiplayer::kGE_Scan: {
      CPlayer* source = TCastToPtr< CPlayer >(*mgr.GetPlayer(sourceIndex));
      if (source != nullptr) {
        AddDamageOverTime(sourceIndex, source->GetUniqueId(), mgr);
      }
      break;
    }
    case CGMMultiplayer::kGE_Kill:
    case CGMMultiplayer::kGE_Stop:
      if (TCastToPtr< CPlayer >(*player) != nullptr) {
        ClearDamageOverTime(mgr);
      }
      break;
    }
    break;
  }
}

static uint BuildPlayerMask(const CStateManager& mgr, uint mask) {
  uint playerMask = mask & 0xF;
  for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
    if ((mask & 1 << (mgr.GetPlayerState(i)->GetTeamIndex() + 4)) != 0) {
      playerMask |= 1 << i;
    }
  }
  return playerMask;
}

CScriptPlayerProxy::CScriptPlayerProxy(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CModelData& model,
    CStateManager& mgr, uint playerMask, int proxyType, const CVector3f& playerOffset,
    int intParameter1, int intParameter2, float floatParameter1, float floatParameter2,
    float floatParameter3, const CVector3f& vectorParameter1, const rstl::string& stringParameter1)
: CActor(uid, name, info, 0, CTransform4f::Identity(), model, CMaterialList(),
         CActorParameters::None(), kInvalidUniqueId)
, mListener(mgr, BuildPlayerMask(mgr, playerMask), this)
, mProxyType(proxyType)
, mPlayerOffset(playerOffset)
, mIntParameter1(intParameter1)
, mIntParameter2(intParameter2)
, mFloatParameter1(floatParameter1)
, mFloatParameter2(floatParameter2)
, mFloatParameter3(floatParameter3)
, mVectorParameter1(vectorParameter1)
, mStringParameter1(stringParameter1)
, mAccumulatedDamage(0.f)
, mActorCount(0)
, mCoinsToDrop(0)
, mPendingCoins(0)
, mCoinDropCooldown(0)
, mTime(0.f)
, mCurrentDuration(0.f)
, mCurrentMinDamage(0.f)
, mCurrentMaxDamage(0.f)
, mCurrentWeaponType(-1)
, mDamageActive(false)
, mFollowing(false) {}

CEntity* LoadPlayerController(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrPlayerController sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrPlayerController.inc"

  rstl::optional_object< CModelData > model(
      LdrToModelData(sldrThis.editorProperties.transform.scale, sldrThis.model,
                     sldrThis.animationInformation, true));
  if (!model) {
    model = CModelData::None();
  }

  if (sldrThis.proxyType < 0 || sldrThis.proxyType > 10) {
    return nullptr;
  }

  return rs_new CScriptPlayerProxy(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), *model, mgr, sldrThis.unknown_0xe71de331,
      sldrThis.proxyType, sldrThis.playerOffset, sldrThis.intParameter1, sldrThis.intParameter2,
      sldrThis.floatParameter1, sldrThis.floatParameter2, sldrThis.floatParameter3,
      sldrThis.vectorParameter1, sldrThis.stringParameter1);
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SPlayerController_FuncPtrs funcPtrs;
  funcPtrs.mLoadPlayerController = &LoadPlayerController;
  SetSPlayerController_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSPlayerController_FuncPtrs(nullptr); }
#endif
