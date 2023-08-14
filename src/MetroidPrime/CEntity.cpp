#include "MetroidPrime/CEntity.hpp"

rstl::vector< SConnection > CEntity::NullConnectionList;

// CEntityInfo::CEntityInfo(TAreaId aid, const rstl::vector< SConnection >& conns, TEditorId eid)
// : x0_areaId(aid), x4_conns(conns), x14_editorId(eid) {}

CEntity::CEntity(TUniqueId id, const CEntityInfo& info, const rstl::string& name, bool active)
: areaId(info.GetAreaId())
, uid(id)
, editorId(info.GetEditorId())
, conns(info.GetConnectionList())
, active(active)
, inGraveyard(false)
, scriptingBlocked(false)
, notInArea(areaId == kInvalidAreaId)
{}

CEntity::~CEntity() {}

void CEntity::AcceptScriptMsg(CStateManager& mgr, CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Activate:
    if (!active) {
      SetActive(true);
      SendScriptMsgs(kSS_Active, mgr, kSM_None);
    }
    break;
  case kSM_Deactivate:
    if (active) {
      SetActive(false);
      SendScriptMsgs(kSS_Inactive, mgr, kSM_None);
    }
    break;
  case kSM_ToggleActive: {
    CScriptMsg newMsg(msg);
    if (!active) {
      newMsg.msg = kSM_Deactivate;
    }
    AcceptScriptMsg(mgr, newMsg);
    break;
  }
  }
}

void CEntity::SendScriptMsgs(EScriptObjectState state, CStateManager& mgr,
                             EScriptObjectMessage skipMsg) {
  rstl::vector< SConnection >::const_iterator it = conns.begin();
  for (; it != conns.end(); ++it) {
    if (it->state == state && it->msg != skipMsg) {
      //   mgr.SendScriptMsg(GetUniqueId(), it->x8_objId, it->x4_msg, state);
    }
  }
}

void CEntity::PreThink(float dt, CStateManager& mgr) {}

void CEntity::Think(float dt, CStateManager& mgr) {}

void CEntity::SetActive(const bool active) { this->active = active; }

TAreaId CEntity::GetAreaId() const { return notInArea ? kInvalidAreaId : areaId; }

TUniqueId CEntity::SearchForSomething(CStateManager&, EScriptObjectState, EScriptObjectMessage) {}
