#include "MetroidPrime/ScriptObjects/CScriptRipple.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrRipple.hpp"

CScriptRipple::CScriptRipple(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                             const CVector3f& center, float energy)
: CEntity(uid, info, name, 0), mEnergy(energy), mCenter(center) {}

CScriptRipple::~CScriptRipple() {}

void CScriptRipple::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  // Ripple playback is disabled in Echoes; other messages retain entity behavior.
  switch (msg.GetMessage()) {
  case kSM_Play:
    break;
  default:
    CEntity::AcceptScriptMsg(mgr, msg);
    break;
  }
}

void CScriptRipple::Think(float, CStateManager&) {}

CEntity* LoadRipple(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrRipple sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrRipple.inc"

  return rs_new CScriptRipple(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                              LdrToEntityInfo(info, sldrThis.editorProperties),
                              sldrThis.editorProperties.transform.position, sldrThis.energy);
}
