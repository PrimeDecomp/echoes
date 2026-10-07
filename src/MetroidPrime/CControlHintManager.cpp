#include "MetroidPrime/CControlHintManager.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/TCastTo.hpp"

CControlHintManager::CControlHintManager(int playerIndex, const rstl::string& name)
: CHintManager(playerIndex, name) {}

CControlHintManager::~CControlHintManager() {}

void CControlHintManager::Reset(CStateManager& mgr) {
  CHintManager::Reset(mgr);
  ClearHint(mgr, false);
}

bool CControlHintManager::SetHint(CHintState* hint, CStateManager& mgr, bool areaChanged,
                                  bool force) {
  if (hint) {
    if (!CHintManager::SetHint(hint, mgr, areaChanged, force)) {
      return false;
    }
    ApplyHint(*hint, mgr);
  } else {
    return false;
  }
  return true;
}

void CControlHintManager::ClearHint(CStateManager& mgr, bool areaChanged) {
  SetCurrentHint(kInvalidUniqueId, 10000);
  mgr.GetPlayer(GetPlayerIndex())->GetControlMapper().ResetCommandFilters();
}

bool CControlHintManager::SelectHintFromStack(CHintState* hint, CStateManager& mgr,
                                              bool areaChanged) {
  const bool select = hint->GetPriority() < GetCurrentPriority();
  if (select) {
    SetHint(hint, mgr, areaChanged, false);
  }
  return select;
}

bool CControlHintManager::ApplyHint(const CHintState& hint, CStateManager& mgr) {
  CPlayer* player = mgr.GetPlayer(GetPlayerIndex());
  CScriptControlHint::TCommandEnabled enabled(true);
  for (rstl::vector< SHint >::const_iterator it = GetHints().begin(); it != GetHints().end();
       ++it) {
    const CScriptControlHint* actor =
        TCastToPtr< CScriptControlHint >(mgr.ObjectById(it->mState.GetHintId()));
    if (!actor) {
      continue;
    }
    const uint flags = actor->GetDisableFlags();
    if (flags & (CScriptControlHint::kDF_All | CScriptControlHint::kDF_ResetGun)) {
      player->GetPlayerGun()->Reset(mgr);
    }
    if (flags & CScriptControlHint::kDF_All) {
      for (int i = 0; i < enabled.size(); ++i) {
        enabled[i] = false;
      }
    } else {
      const CScriptControlHint::TCommandEnabled commands = actor->GetCommandEnabled();
      for (int i = 0; i < commands.size(); ++i) {
        if (!commands[i]) {
          enabled[i] = false;
        }
      }
    }
  }
  player->GetControlMapper().SetCommandFilters(enabled);
  return true;
}

void CControlHintManager::ProcessInput(const CFinalInput& input, const CControlMapper& mapper,
                                       CStateManager& mgr) {
  for (rstl::vector< SHint >::iterator it = GetHints().begin(); it != GetHints().end(); ++it) {
    CHintState& hint = it->mState;
    if (hint.ProcessInput(input, mapper, mgr)) {
      ForceRemoveHint(hint.GetHintId(), mgr, hint.GetFirstSender());
    }
  }
}

TUniqueId CControlHintManager::CreateHint(CStateManager& mgr, const rstl::string& name,
                                          int priority, float timer, uint disableFlags,
                                          const CScriptControlHint::TCommandStates& commandStates,
                                          TUniqueId sender, CGameHint::EBreakHintType breakType,
                                          uint requiredPresses, float unknown16c,
                                          CGameHint::SCallback onExpire,
                                          CGameHint::SCallback onBreak, float breakDelay,
                                          int acrossAreas) {
  const rstl::vector< SConnection > connections;
  CEntity* ent = rs_new CScriptControlHint(
      mgr.AllocateUniqueId(), name,
      CEntityInfo(kInvalidAreaId, connections, true, kInvalidEditorId), CTransform4f::Identity(),
      priority, timer, disableFlags, commandStates, breakType, 1, requiredPresses, unknown16c,
      onExpire, onBreak, breakDelay, acrossAreas);
  if (ent) {
    mgr.AddObject(ent);
    AddHint(ent->GetUniqueId(), sender, mgr);
    return ent->GetUniqueId();
  }
  return kInvalidUniqueId;
}

const bool CControlHintManager::HasDisableFlags(uint flags, const CStateManager& mgr) const {
  bool ret = false;
  for (rstl::vector< SHint >::const_iterator it = GetHints().begin(); it != GetHints().end();
       ++it) {
    const CScriptControlHint* hint =
        TCastToConstPtr< CScriptControlHint >(mgr.GetObjectById(it->mState.GetHintId()));
    if (hint && (hint->GetDisableFlags() & flags)) {
      ret = true;
      break;
    }
  }
  return ret;
}
