#include "MetroidPrime/ScriptObjects/CScriptMemoryRelay.hpp"

#include "MetroidPrime/CScriptMailbox.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrMemoryRelay.hpp"

CScriptMemoryRelay::CScriptMemoryRelay(TUniqueId uid, const rstl::string& name,
                                       const CEntityInfo& info, bool defaultActive,
                                       bool skipSendActive)
: CEntity(uid, info, name, 0), mDefaultActive(defaultActive), mSkipSendActive(skipSendActive) {}

CScriptMemoryRelay::~CScriptMemoryRelay() {}

void CScriptMemoryRelay::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Activate:
    mgr.Mailbox()->AddMsg(GetEditorId());
    if (!mSkipSendActive) {
      SendScriptMsgs(kSS_Active, mgr, kSM_None);
    }
    break;
  case kSM_Deactivate:
    mgr.Mailbox()->RemoveMsg(GetEditorId());
    break;
  default:
    CEntity::AcceptScriptMsg(mgr, msg);
    break;
  }
}

CEntity* LoadMemoryRelay(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrMemoryRelay sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrMemoryRelay.inc"

  LdrToEntityInfo(info, sldrThis.editorProperties);
  info.SetActive(true);
  return rs_new CScriptMemoryRelay(mgr.AllocateUniqueId(), sldrThis.editorProperties.name, info,
                                   sldrThis.oneShot, sldrThis.delayedAction);
}
