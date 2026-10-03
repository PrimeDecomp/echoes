#include "MetroidPrime/ScriptObjects/CScriptTimeKeyframe.hpp"

#include "MetroidPrime/ScriptLoader.hpp"

CScriptTimeKeyframe::CScriptTimeKeyframe(TUniqueId uid, const rstl::string& name,
                                       const CEntityInfo& info, float time)
: CEntity(uid, info, name, 0), mTime(time) {}

CScriptTimeKeyframe::~CScriptTimeKeyframe() {}

CEntity* CScriptTimeKeyframe::TypesMatch(int typeId) const {}

void CScriptTimeKeyframe::SetTime(float time, CStateManager& mgr) {}

void CScriptTimeKeyframe::ApplyTime(TUniqueId id, CStateManager& mgr) {}

void CScriptTimeKeyframe::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {}

CEntity* LoadTimeKeyframe(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {}
