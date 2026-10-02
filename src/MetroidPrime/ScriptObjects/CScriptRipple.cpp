#include "MetroidPrime/ScriptObjects/CScriptRipple.hpp"

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
