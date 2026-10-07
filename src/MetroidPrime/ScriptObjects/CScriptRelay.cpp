#include "MetroidPrime/ScriptObjects/CScriptRelay.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrRelay.hpp"

CScriptRelay::CScriptRelay(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const bool oneShot)
: CEntity(uid, info, name, 0), mOriginator(kInvalidUniqueId), mOneShot(oneShot) {}

void CScriptRelay::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CEntity::AcceptScriptMsg(mgr, msg);

  if (GetActive()) {
    switch (msg.GetMessage()) {
    case kSM_SetToZero: {
      SendScriptMsgs(kSS_Zero, mgr,
                     mOriginator != kInvalidUniqueId ? TUniqueId(mOriginator.value)
                                                     : msg.GetOriginator(),
                     kSM_None);
      if (mOneShot) {
        CEntity::AcceptScriptMsg(mgr, CScriptMsg(GetUniqueId(), GetUniqueId(), kSM_Deactivate));
      }
      break;
    }
    case kSM_SetOriginator:
      mOriginator = msg.GetOriginator();
      break;
    case kSM_ClearOriginator:
      mOriginator = kInvalidUniqueId;
      break;
    default:
      break;
    }
  }
}

CEntity* LoadRelay(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrRelay sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrRelay.inc"

  return rs_new CScriptRelay(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                             LdrToEntityInfo(info, sldrThis.editorProperties), sldrThis.oneShot);
}

CScriptRelay::~CScriptRelay() {}
