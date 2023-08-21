#include "MetroidPrime/CEntity.hpp"

#include "MetroidPrime/CStateManager.hpp"

rstl::vector< SConnection > CEntity::NullConnectionList;

CEntityInfo CEntity::NullEntityInfo(
  kInvalidAreaId, NullConnectionList, true, kInvalidEditorId
);

// CEntityInfo::CEntityInfo(TAreaId aid, const rstl::vector< SConnection >& conns, TEditorId eid)
// : x0_areaId(aid), x4_conns(conns), x14_editorId(eid) {}

CEntity::CEntity(TUniqueId id, const CEntityInfo& info, const rstl::string& name, int graveyard)
: m_areaId(info.GetAreaId())
, m_uid(id)
, m_editorId(info.GetEditorId())
, m_conns(info.GetConnectionList())
, m_active(info.GetActive())
, m_notInArea(m_areaId == kInvalidAreaId)
, m_inGraveyard(graveyard)
, m_scriptingBlocked(info.GetActive())
, m_entityUnknown(info.GetActive()) {}

CEntity::~CEntity() {}

void CEntity::AcceptScriptMsg(CStateManager& mgr, CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Activate:
    if (!m_active) {
      SetActive(true);
      SendScriptMsgs(kSS_Active, mgr, kInvalidUniqueId, kSM_None);
    }
    break;
  case kSM_Deactivate:
    if (m_active) {
      SetActive(false);
      SendScriptMsgs(kSS_Inactive, mgr, kInvalidUniqueId, kSM_None);
    }
    break;
  case kSM_ToggleActive: {
    CScriptMsg newMsg(msg);
    if (!m_active) {
      newMsg.SetMessage(kSM_Deactivate);
    }
    AcceptScriptMsg(mgr, newMsg);
    break;
  }
  }
}

void CEntity::SendScriptMsgs(EScriptObjectState state, CStateManager& mgr, TUniqueId id,
                             EScriptObjectMessage skipMsg) {
  rstl::vector< SConnection >::const_iterator it = m_conns.begin();
  for (; it != m_conns.end(); ++it) {
    if (it->state == state && it->msg != skipMsg) {
      CStateManager::TIdListResult search = mgr.GetIdListForScript(it->objId);
      CStateManager::TIdList::const_iterator current = search.first;
      CStateManager::TIdList::const_iterator end = search.second;
      while (current != end) {
        mgr.SendScriptMsg_fn_80037100(
            CScriptMsg(GetUniqueId(), id, current->second, it->msg, it->state));
        ++current;
      }
    }
  }
}

void CEntity::PreThink(float dt, CStateManager& mgr) {}

void CEntity::Think(float dt, CStateManager& mgr) {}

void CEntity::SetActive(const bool active) { m_active = active; }

TAreaId CEntity::GetAreaId() const { return m_notInArea ? kInvalidAreaId : m_areaId; }

TUniqueId CEntity::SearchForSomething(CStateManager&, EScriptObjectState, EScriptObjectMessage) {}
