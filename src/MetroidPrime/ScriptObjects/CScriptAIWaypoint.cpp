#include "MetroidPrime/ScriptObjects/CScriptAIWaypoint.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAIWaypoint.hpp"

CScriptAIWaypoint::CScriptAIWaypoint(TUniqueId uid, const rstl::string& name,
                                     const CEntityInfo& info, const CTransform4f& xf, float speed,
                                     float pause, int flags, int locatorIndex, int unknown)
: CScriptWaypoint(uid, name, info, xf)
, mSpeed(speed)
, mPause(pause)
, mLocatorIndex(locatorIndex)
, x164_(static_cast< ushort >(flags))
, x168_(unknown) {}

CScriptAIWaypoint::~CScriptAIWaypoint() {}

CEntity* LoadAIWaypoint(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrAIWaypoint sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrAIWaypoint.inc"

  return rs_new CScriptAIWaypoint(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                                  LdrToEntityInfo(info, sldrThis.editorProperties),
                                  LdrToTransform4f(sldrThis.editorProperties), sldrThis.speed,
                                  sldrThis.pause, sldrThis.unknown_0xc6705a00,
                                  sldrThis.locatorIndex, sldrThis.unknown_0x166979d4);
}
