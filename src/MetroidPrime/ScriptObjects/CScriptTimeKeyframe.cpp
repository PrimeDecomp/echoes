#include "MetroidPrime/ScriptObjects/CScriptTimeKeyframe.hpp"

#include "MetroidPrime/CEffectWaypointPredicate.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTimeKeyframe.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActorRotate.hpp"
#include "MetroidPrime/ScriptObjects/CScriptColorModulate.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSequenceTimer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"

CScriptTimeKeyframe::CScriptTimeKeyframe(TUniqueId uid, const rstl::string& name,
                                         const CEntityInfo& info, float time)
: CEntity(uid, info, name, 0), mTime(time) {}

CScriptTimeKeyframe::~CScriptTimeKeyframe() {}

CEntity* CScriptTimeKeyframe::TypesMatch(int typeId) const {
  if (typeId == 90) {
    return const_cast< CScriptTimeKeyframe* >(this);
  }
  if (typeId > 90) {
    return nullptr;
  }
  return CEntity::TypesMatch(typeId);
}

void CScriptTimeKeyframe::SetTime(float time, CStateManager& mgr) {
  mTime = time;
  ApplyTime(kInvalidUniqueId, mgr);
}

void CScriptTimeKeyframe::ApplyTime(TUniqueId id, CStateManager& mgr) {
  const rstl::vector< TUniqueId > targets = FindConnectedObjects(mgr, kSS_Connect, kSM_Attach);
  if (targets.empty()) {
    return;
  }

  for (int i = 0; i < targets.size(); ++i) {
    CEntity* target = mgr.ObjectById(targets[i]);
    if (CScriptPlatform* platform = TCastToPtr< CScriptPlatform >(target)) {
      const CScriptWaypoint* waypoint = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(
          CheckConnectedObject_if(mgr, kSS_Connect, kSM_Attach, CEffectWaypointPredicate())));
      if (waypoint) {
        platform->TeleportToWaypoint(waypoint->GetUniqueId(), mgr);
      } else {
        platform->SetMotionTime(mTime, mgr);
      }
    }

    if (CScriptSequenceTimer* timer = TCastToPtr< CScriptSequenceTimer >(target)) {
      timer->SetCurrentTime(mTime);
    }
    if (CScriptColorModulate* color = TCastToPtr< CScriptColorModulate >(target)) {
      color->SetExternalTime(mTime);
    }
    if (CScriptActorRotate* rotate = TCastToPtr< CScriptActorRotate >(target)) {
      rotate->SetCurrentTime(mTime);
    }
  }
}

void CScriptTimeKeyframe::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const TUniqueId sender = msg.GetSenderId();
  const EScriptObjectMessage message = msg.GetMessage();
  CEntity::AcceptScriptMsg(mgr, msg);

  if (GetActive()) {
    switch (message) {
    case kSM_Action:
      ApplyTime(sender, mgr);
      break;
    default:
      break;
    }
  }
}

CEntity* LoadTimeKeyframe(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrTimeKeyframe sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrTimeKeyframe.inc"

  return rs_new CScriptTimeKeyframe(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                                    LdrToEntityInfo(info, sldrThis.editorProperties),
                                    sldrThis.time);
}
