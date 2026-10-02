#include "MetroidPrime/CPlayerHintManager.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlayerHint.hpp"
#include "MetroidPrime/TCastTo.hpp"

CPlayerHintManager::CPlayerHintManager(int playerIndex, const rstl::string& name)
: CHintManager(playerIndex, name) {}

CPlayerHintManager::~CPlayerHintManager() {}

bool CPlayerHintManager::SetHint(CHintState* hint, CStateManager& mgr, bool areaChanged,
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

void CPlayerHintManager::ClearHint(CStateManager& mgr, bool areaChanged) {
  ClearCurrentHint(10000);
  mgr.GetPlayer(GetPlayerIndex())->ResetPlayerHintState(mgr);
}

bool CPlayerHintManager::SelectHintFromStack(CHintState* hint, CStateManager& mgr,
                                             bool areaChanged) {
  const bool select = hint->GetPriority() < GetCurrentPriority();
  if (select) {
    SetHint(hint, mgr, areaChanged, false);
  }
  return select;
}

bool CPlayerHintManager::ApplyHint(const CHintState& hint, CStateManager& mgr) {
  const CScriptPlayerHint* actor =
      TCastToPtr< CScriptPlayerHint >(mgr.ObjectById(hint.GetHintId()));
  if (!actor) {
    return false;
  }
  if (mgr.GetPlayer(GetPlayerIndex())->SetAreaPlayerHint(*actor, mgr)) {
    ForceRemoveHint(hint.GetHintId(), mgr, kInvalidUniqueId);
  }
  return true;
}
