#include "MetroidPrime/ScriptObjects/CScriptPlayerStateChange.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlayerStateChange.hpp"

CScriptPlayerStateChange::CScriptPlayerStateChange(TUniqueId uid, const rstl::string& name,
                                                   const CEntityInfo& info,
                                                   CPlayerState::EItemType itemType, int amount,
                                                   int capacityIncrease, int command,
                                                   int commandAction)
: CEntity(uid, info, name, 0)
, mItemType(itemType)
, mAmount(amount)
, mCapacityIncrease(capacityIncrease)
, mCommand(command)
, mCommandAction(commandAction) {}

void CScriptPlayerStateChange::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  if (GetActive() && message == kSM_SetToZero) {
    // Native code also discards a side-effect-free multiplayer query here.
    mgr.PlayerState(0)->AddPowerUp(mItemType, mCapacityIncrease);
    mgr.PlayerState(0)->IncrPickUp(mItemType, mAmount);
  }

  CEntity::AcceptScriptMsg(mgr, msg);
}

CEntity* LoadPlayerStateChange(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrPlayerStateChange sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrPlayerStateChange.inc"

  return rs_new CScriptPlayerStateChange(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                                         LdrToEntityInfo(info, sldrThis.editorProperties),
                                         CPlayerState::EItemType(sldrThis.itemToChange.value),
                                         sldrThis.amount, sldrThis.capacityIncrease,
                                         sldrThis.command, sldrThis.commandAction);
}

CScriptPlayerStateChange::~CScriptPlayerStateChange() {}
