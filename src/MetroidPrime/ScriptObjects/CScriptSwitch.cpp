#include "MetroidPrime/ScriptObjects/CScriptSwitch.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSwitch.hpp"

CScriptSwitch::CScriptSwitch(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                             const bool opened, const bool closeOnOpened)
: CEntity(uid, info, name, 0), mOpened(opened), mCloseOnOpened(closeOnOpened) {}

void CScriptSwitch::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();

  if (GetActive()) {
    switch (message) {
    case kSM_Open:
      mOpened = true;
      break;
    case kSM_Close:
      mOpened = false;
      break;
    case kSM_SetToZero:
      if (mOpened) {
        SendScriptMsgs(kSS_Open, mgr, msg.GetOriginator(), kSM_None);
        if (mCloseOnOpened) {
          mOpened = false;
        }
      } else {
        SendScriptMsgs(kSS_Closed, mgr, msg.GetOriginator(), kSM_None);
      }
      break;
    default:
      break;
    }
  }

  CEntity::AcceptScriptMsg(mgr, msg);
}

CEntity* LoadSwitch(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSwitch sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSwitch.inc"

  return rs_new CScriptSwitch(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                              LdrToEntityInfo(info, sldrThis.editorProperties), sldrThis.isOpen,
                              sldrThis.isAutoClose);
}

CScriptSwitch::~CScriptSwitch() {}
