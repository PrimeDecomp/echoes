#include "MetroidPrime/ScriptObjects/CScriptConditionalRelay.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrConditionalRelay.hpp"
#include "MetroidPrime/TCastTo.hpp"

CConditionalRelayQuery::CConditionalRelayQuery(EBoolean boolean, CPlayerState::EItemType item,
                                               EField field, EComparison comparison, int value)
: mBoolean(boolean), mItem(item), mField(field), mComparison(comparison), mValue(value) {}

bool CConditionalRelayQuery::IsConditionSatisfied(CStateManager& mgr, uint playerIndex) const {
  const CPlayerState& player = *mgr.GetPlayerState(playerIndex);
  const int amount =
      mField == kF_Amount ? player.GetItemAmount(mItem, true) : player.GetItemCapacity(mItem);
  switch (mComparison) {
  case kC_Equal:
    return amount == mValue;
  case kC_NotEqual:
    return amount != mValue;
  case kC_Greater:
    return amount > mValue;
  case kC_Less:
    return amount < mValue;
  case kC_GreaterOrEqual:
    return amount >= mValue;
  case kC_LessOrEqual:
    return amount <= mValue;
  case kC_GreaterThanAllPlayers:
    for (int i = 0; i < uint(mgr.GetNumPlayers()); ++i) {
      if (i == playerIndex) {
        continue;
      }
      const CPlayerState& other = *mgr.GetPlayerState(i);
      const int otherAmount =
          mField == kF_Amount ? other.GetItemAmount(mItem, true) : other.GetItemCapacity(mItem);
      if (otherAmount + mValue >= amount) {
        return false;
      }
    }
    return true;
  case kC_LessThanAllPlayers:
    for (int i = 0; i < uint(mgr.GetNumPlayers()); ++i) {
      if (i == playerIndex) {
        continue;
      }
      const CPlayerState& other = *mgr.GetPlayerState(i);
      const int otherAmount =
          mField == kF_Amount ? other.GetItemAmount(mItem, true) : other.GetItemCapacity(mItem);
      if (otherAmount - mValue <= amount) {
        return false;
      }
    }
    return true;
  default:
    return false;
  }
}

CScriptConditionalRelay::CScriptConditionalRelay(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, uint playerMask,
    const rstl::reserved_vector< CConditionalRelayQuery, 4 >& conditions,
    bool setToZeroOnAreaLoaded)
: CEntity(uid, info, name, 0)
, mPlayerMask(playerMask)
, mConditions(conditions)
, mSetToZeroOnAreaLoaded(setToZeroOnAreaLoaded) {}

void CScriptConditionalRelay::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const TUniqueId originator = msg.GetOriginator();
  const EScriptObjectMessage message = msg.GetMessage();
  CEntity::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_SetToZero:
    OnSetToZero(mgr, originator);
    break;
  default:
    break;
  }
}

void CScriptConditionalRelay::Think(float dt, CStateManager& mgr) {
  if (mSetToZeroOnAreaLoaded) {
    OnSetToZero(mgr, kInvalidUniqueId);
    mSetToZeroOnAreaLoaded = false;
  }
}

void CScriptConditionalRelay::OnSetToZero(CStateManager& mgr, TUniqueId originator) {
  if (GetActive()) {
    if (VerifyConditions(mgr, originator)) {
      SendScriptMsgs(kSS_Opened, mgr, originator, kSM_None);
    } else {
      SendScriptMsgs(kSS_Closed, mgr, originator, kSM_None);
    }
  }
}

bool CScriptConditionalRelay::VerifyConditions(CStateManager& mgr, TUniqueId originator) const {
  if (!(mPlayerMask & (1u << (mgr.GetNumPlayers() + 8)))) {
    return false;
  }
  const bool requireAllPlayers = (mPlayerMask & 0x100) != 0;
  uint playerMask = mPlayerMask;
  if (!(playerMask & 0xff)) {
    const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(originator));
    if (player != nullptr) {
      playerMask |= 1u << mgr.MaskUIdNumPlayers(originator);
    }
  }
  for (int i = 0; i < uint(mgr.GetNumPlayers()); ++i) {
    const CPlayerState& player = *mgr.GetPlayerState(i);
    if (mgr.IsMultiplayer() && !(playerMask & (1u << i)) &&
        !(playerMask & (1u << (player.GetTeamIndex() + 4)))) {
      continue;
    }
    bool first = true;
    bool satisfied = true;
    for (int j = 0; j < mConditions.size(); ++j) {
      const CConditionalRelayQuery& query = mConditions[j];
      if (query.GetBoolean() == CConditionalRelayQuery::kB_Disabled) {
        continue;
      }
      bool result = query.IsConditionSatisfied(mgr, i);
      if (first) {
        satisfied = result;
        first = false;
      } else {
        switch (query.GetBoolean()) {
        case CConditionalRelayQuery::kB_Disabled:
          break;
        case CConditionalRelayQuery::kB_And:
          satisfied = satisfied && result;
          break;
        case CConditionalRelayQuery::kB_Or:
          satisfied = satisfied || result;
          break;
        }
      }
    }
    if (requireAllPlayers) {
      if (!satisfied) {
        return false;
      }
    } else if (satisfied) {
      return true;
    }
  }
  return requireAllPlayers;
}

CEntity* LoadConditionalRelay(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrConditionalRelay sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrConditionalRelay.inc"

  rstl::reserved_vector< CConditionalRelayQuery, 4 > conditions;
  conditions.push_back(CConditionalRelayQuery(
      static_cast< CConditionalRelayQuery::EBoolean >(sldrThis.conditional1.boolean),
      static_cast< CPlayerState::EItemType >(sldrThis.conditional1.playerItem.value),
      static_cast< CConditionalRelayQuery::EField >(sldrThis.conditional1.amountOrCapacity),
      static_cast< CConditionalRelayQuery::EComparison >(sldrThis.conditional1.condition),
      sldrThis.conditional1.value));
  conditions.push_back(CConditionalRelayQuery(
      static_cast< CConditionalRelayQuery::EBoolean >(sldrThis.conditional2.boolean),
      static_cast< CPlayerState::EItemType >(sldrThis.conditional2.playerItem.value),
      static_cast< CConditionalRelayQuery::EField >(sldrThis.conditional2.amountOrCapacity),
      static_cast< CConditionalRelayQuery::EComparison >(sldrThis.conditional2.condition),
      sldrThis.conditional2.value));
  conditions.push_back(CConditionalRelayQuery(
      static_cast< CConditionalRelayQuery::EBoolean >(sldrThis.conditional3.boolean),
      static_cast< CPlayerState::EItemType >(sldrThis.conditional3.playerItem.value),
      static_cast< CConditionalRelayQuery::EField >(sldrThis.conditional3.amountOrCapacity),
      static_cast< CConditionalRelayQuery::EComparison >(sldrThis.conditional3.condition),
      sldrThis.conditional3.value));
  conditions.push_back(CConditionalRelayQuery(
      static_cast< CConditionalRelayQuery::EBoolean >(sldrThis.conditional4.boolean),
      static_cast< CPlayerState::EItemType >(sldrThis.conditional4.playerItem.value),
      static_cast< CConditionalRelayQuery::EField >(sldrThis.conditional4.amountOrCapacity),
      static_cast< CConditionalRelayQuery::EComparison >(sldrThis.conditional4.condition),
      sldrThis.conditional4.value));
  return rs_new CScriptConditionalRelay(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                                        LdrToEntityInfo(info, sldrThis.editorProperties),
                                        sldrThis.multiplayerMaskandNegate, conditions,
                                        sldrThis.setToZeroOnAreaLoaded);
}

CScriptConditionalRelay::~CScriptConditionalRelay() {}
