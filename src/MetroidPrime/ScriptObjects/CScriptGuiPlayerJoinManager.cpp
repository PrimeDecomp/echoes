#include "MetroidPrime/ScriptObjects/CScriptGuiPlayerJoinManager.hpp"

#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Input/IController.hpp"
#include "MetroidPrime/CGameGlobalObjects.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/ScriptLoader/SLdrGuiPlayerJoinManager.hpp"

// Guessed names. Per-controller states sent on join-state transitions.
static const EScriptObjectState skJoinedStates[4] = {kSS_InternalState00, kSS_InternalState01,
                                                     kSS_InternalState02, kSS_InternalState03};
static const EScriptObjectState skJoinedExtraStates[4] = {kSS_BombDamage, kSS_PowerBombDamage,
                                                          kSS_MissileDamage, kSS_BoostBallDamage};
static const EScriptObjectState skDisconnectedStates[4] = {
    kSS_InternalState04, kSS_InternalState05, kSS_InternalState06, kSS_InternalState07};
static const EScriptObjectState skReadyStates[4] = {kSS_InternalState08, kSS_InternalState09,
                                                    kSS_InternalState10, kSS_InternalState11};
static const EScriptObjectState skLeftStates[4] = {kSS_InternalState12, kSS_InternalState13,
                                                   kSS_InternalState14, kSS_InternalState15};
static const EScriptObjectState skLeftExtraStates[4] = {kSS_CannonBallDamage, kSS_ScrewAttackDamage,
                                                        kSS_PhazonDamage, kSS_AIDamage};
static const EScriptObjectState skConnectedStates[4] = {kSS_PowerDamage, kSS_DarkDamage,
                                                        kSS_LightDamage, kSS_AnnihilatorDamage};
static const EScriptObjectState skPlayerCountStates[4] = {kSS_InternalState16, kSS_InternalState17,
                                                          kSS_InternalState18, kSS_InternalState19};

static CScriptGuiPlayerJoinManager::EJoinState sInitialJoinState[1] = {
    CScriptGuiPlayerJoinManager::kJS_Disconnected};

CScriptGuiPlayerJoinManager::CScriptGuiPlayerJoinManager(TUniqueId uid, const rstl::string& name,
                                                         const CEntityInfo& info)
: CEntity(uid, info, name, 0), mJoinStates(sInitialJoinState[0]), mPlayerCountState(0) {}

void CScriptGuiPlayerJoinManager::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CEntity::AcceptScriptMsg(mgr, msg);

  switch (msg.GetMessage()) {
  case kSM_Create:
    RestorePreviousPlayers(mgr);
    break;
  case kSM_Reset:
    for (rstl::reserved_vector< EJoinState, 4 >::iterator it = mJoinStates.begin();
         it != mJoinStates.end(); ++it) {
      *it = kJS_Disconnected;
    }
    mPlayerCountState = 0;
    break;
  default:
    break;
  }

  if (GetActive()) {
    switch (msg.GetMessage()) {
    case kSM_InternalMessage00:
    case kSM_InternalMessage01:
    case kSM_InternalMessage02:
    case kSM_InternalMessage03: {
      int idx;
      if (msg.GetMessage() == kSM_InternalMessage00) {
        idx = 0;
      } else if (msg.GetMessage() == kSM_InternalMessage01) {
        idx = 1;
      } else if (msg.GetMessage() == kSM_InternalMessage02) {
        idx = 2;
      } else {
        idx = 3;
      }
      mJoinStates[idx] = kJS_ForcedJoin;
      break;
    }
    case kSM_InternalMessage04:
      SendControllerStates(mgr);
      mPlayerCountState = 0;
      UpdatePlayerCount(mgr);
      break;
    default:
      break;
    }
  }
}

void CScriptGuiPlayerJoinManager::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  bool backPressed = false;
  bool startPressed = false;
  for (int i = 0; i < 4; ++i) {
    EJoinState& state = mJoinStates[i];
    if (state == kJS_Ready) {
      continue;
    }
    if (state == kJS_ForcedJoin) {
      state = kJS_Joined;
    }

    const CFinalInput& input = mgr.mFinalInputs[i];
    if (input.PA()) {
      if (state != kJS_Joined) {
        state = kJS_Joined;
        SendScriptMsgs(skJoinedStates[i], mgr);
        SendScriptMsgs(skJoinedExtraStates[i], mgr);
      }
    } else if (input.PY()) {
      if (state == kJS_Joined) {
        state = kJS_Ready;
        SendScriptMsgs(skReadyStates[i], mgr);
      }
    } else if (input.PB()) {
      if (state == kJS_Joined) {
        state = kJS_Connected;
        SendScriptMsgs(skLeftStates[i], mgr);
        SendScriptMsgs(skLeftExtraStates[i], mgr);
      } else {
        backPressed = true;
      }
    } else if (input.PStart()) {
      if (state == kJS_Joined) {
        startPressed = true;
      }
    } else {
      bool present = gpController->GetGamepadData(i).DeviceIsPresent();
      if (present && state == kJS_Disconnected) {
        state = kJS_Connected;
        SendScriptMsgs(skConnectedStates[i], mgr);
      } else if (!present && state != kJS_Disconnected) {
        state = kJS_Disconnected;
        SendScriptMsgs(skDisconnectedStates[i], mgr);
      }
    }
  }

  if (backPressed && NoPlayersJoined()) {
    SendScriptMsgs(kSS_PressB, mgr);
  } else if (startPressed && HasEnoughPlayers()) {
    SendScriptMsgs(kSS_PressStart, mgr);
  }

  UpdatePlayerCount(mgr);
}

void CScriptGuiPlayerJoinManager::RestorePreviousPlayers(CStateManager& mgr) {
  const CGameState::SPreviousGameResults& results = gpGameState->PreviousGameResults();
  if (results.mGameMode == 'COIN' || results.mGameMode == 'DTHM') {
    for (int i = 0; i < results.mPlayerCount; ++i) {
      mJoinStates[results.mPlayers[i].mPlayerSelection] = kJS_Joined;
    }
  }
}

void CScriptGuiPlayerJoinManager::SendControllerStates(CStateManager& mgr) {
  for (int i = 0; i < 4; ++i) {
    if (gpController->GetGamepadData(i).DeviceIsPresent()) {
      if (mJoinStates[i] == kJS_Joined) {
        SendScriptMsgs(skJoinedStates[i], mgr);
      } else if (mJoinStates[i] == kJS_Connected) {
        SendScriptMsgs(skLeftStates[i], mgr);
      }
    } else {
      mJoinStates[i] = kJS_Disconnected;
      SendScriptMsgs(skDisconnectedStates[i], mgr);
    }
  }
}

void CScriptGuiPlayerJoinManager::UpdatePlayerCount(CStateManager& mgr) {
  int joined = 0;
  rstl::reserved_vector< EJoinState, 4 >::iterator end = mJoinStates.end();
  for (rstl::reserved_vector< EJoinState, 4 >::iterator it = mJoinStates.begin(); it != end; ++it) {
    const EJoinState state = *it;
    if (state == kJS_Ready || state == kJS_ForcedJoin) {
      joined = 0;
      break;
    }
    if (state == kJS_Joined) {
      ++joined;
    }
  }

  const int countState = joined > 3 ? 3 : joined > 2 ? 2 : joined > 1 ? 1 : 0;
  if (countState != mPlayerCountState) {
    mPlayerCountState = countState;
    SendScriptMsgs(skPlayerCountStates[countState], mgr);
  }
}

bool CScriptGuiPlayerJoinManager::NoPlayersJoined() const {
  rstl::reserved_vector< EJoinState, 4 >::const_iterator end = mJoinStates.end();
  for (rstl::reserved_vector< EJoinState, 4 >::const_iterator it = mJoinStates.begin(); it != end;
       ++it) {
    const EJoinState state = *it;
    if (state == kJS_Ready) {
      return false;
    }
    if (state == kJS_ForcedJoin) {
      return false;
    }
    if (state == kJS_Joined) {
      return false;
    }
  }
  return true;
}

bool CScriptGuiPlayerJoinManager::HasEnoughPlayers() const { return mPlayerCountState != 0; }

CEntity* LoadGuiPlayerJoinManager(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrGuiPlayerJoinManager sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrGuiPlayerJoinManager.inc"

  return rs_new CScriptGuiPlayerJoinManager(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                                            LdrToEntityInfo(info, sldrThis.editorProperties));
}

CScriptGuiPlayerJoinManager::~CScriptGuiPlayerJoinManager() {}
