#include "MetroidPrime/CEntity.hpp"

#include "MetroidPrime/CStateManager.hpp"

namespace rstl {
template <>
vector< TUniqueId >::vector(const vector< TUniqueId >& other)
: x0_allocator(other.x0_allocator), x4_count(other.x4_count), x8_capacity(other.x8_capacity) {
  if (other.x4_count == 0 && other.x8_capacity == 0) {
    xc_items = nullptr;
  } else {
    x0_allocator.allocate(xc_items, x8_capacity);
    TUniqueId* dst = xc_items;
    const TUniqueId* src = other.xc_items;
    int count = x4_count;
    while (count--) {
      *dst++ = *src++;
    }
  }
}

template <>
vector< SConnection >::vector(const vector< SConnection >& other)
: x0_allocator(other.x0_allocator), x4_count(other.x4_count), x8_capacity(other.x8_capacity) {
  if (other.x4_count == 0 && other.x8_capacity == 0) {
    xc_items = nullptr;
  } else {
    x0_allocator.allocate(xc_items, x8_capacity);
    SConnection* dst = xc_items;
    const SConnection* src = other.xc_items;
    int count = x4_count;
    while (count--) {
      *dst++ = *src++;
    }
  }
}
} // namespace rstl

rstl::vector< SConnection > CEntity::NullConnectionList;

CEntityInfo CEntity::NullEntityInfo = CEntityInfo(kInvalidAreaId, NullConnectionList, true, kInvalidEditorId);

CEntityInfo::CEntityInfo(TAreaId aid, const rstl::vector< SConnection >& connections,
                         bool isActive, TEditorId eid)
: areaId(aid)
, conns(connections)
, editorId(eid)
, active(isActive)
, scriptingBlocked(true)
, unk(true) {}

CEntity::CEntity(TUniqueId id, const CEntityInfo& info, const rstl::string& name, uint castFlags)
: m_areaId(info.GetAreaId())
, m_uid(id)
, m_editorId(info.GetEditorId())
, m_conns(info.GetConnectionList())
, m_active(info.GetActive())
, m_notInArea(m_areaId == kInvalidAreaId)
, m_castFlags(castFlags)
, m_scriptingBlocked(info.GetScriptingBlocked())
, m_entityUnknown(info.GetUnk()) {}

CEntity::~CEntity() {}

void CEntity::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
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
    EScriptObjectMessage next = m_active ? kSM_Deactivate : kSM_Activate;
    CScriptMsg newMsg(msg.GetUnk(), msg.GetOriginator(), msg.GetId(),
                      next, msg.GetState());
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

void CEntity::SendActive(CStateManager& mgr, bool active) {
  if (active != GetActive()) {
    mgr.SendScriptMsg(this, GetUniqueId(), active ? kSM_Activate : kSM_Deactivate,
                      kInvalidUniqueId);
  }
}

TAreaId CEntity::GetAreaIdForPersistence() const { return m_notInArea ? kInvalidAreaId : m_areaId; }

TUniqueId CEntity::FindConnectedObject(const CStateManager& mgr, EScriptObjectState state,
                                       EScriptObjectMessage msg) const {
  for (rstl::vector< SConnection >::const_iterator it = m_conns.begin(); it != m_conns.end(); ++it) {
    if ((state == kSS_InvalidState || state == it->state) &&
        (msg == kSM_None || msg == it->msg)) {
      CStateManager::TIdListResult ids = mgr.GetIdListForScript(it->objId);
      if (!(ids.first == ids.second)) {
        return ids.first->second;
      }
    }
  }
  return kInvalidUniqueId;
}

TUniqueId CEntity::FindConnectedObject_if(const CStateManager& mgr, EScriptObjectState state,
                                          EScriptObjectMessage msg,
                                          const CValidEntityPredicate& predicate) const {
  for (rstl::vector< SConnection >::const_iterator it = m_conns.begin(); it != m_conns.end(); ++it) {
    if ((state == kSS_InvalidState || state == it->state) &&
        (msg == kSM_None || msg == it->msg)) {
      CStateManager::TIdListResult ids = mgr.GetIdListForScript(it->objId);
      if (!(ids.first == ids.second)) {
        if (predicate.IsValid(mgr, ids.first->second)) {
          return ids.first->second;
        }
      }
    }
  }
  return kInvalidUniqueId;
}

rstl::vector< TUniqueId > CEntity::FindConnectedObjects(const CStateManager& mgr,
                                                         EScriptObjectState state,
                                                         EScriptObjectMessage msg) const {
  rstl::vector< TUniqueId > result;
  for (rstl::vector< SConnection >::const_iterator it = m_conns.begin(); it != m_conns.end(); ++it) {
    if ((state == kSS_InvalidState || state == it->state) &&
        (msg == kSM_None || msg == it->msg)) {
      CStateManager::TIdListResult ids = mgr.GetIdListForScript(it->objId);
      if (!(ids.first == ids.second)) {
        result.reserve(result.size() + rstl::distance(ids.first, ids.second));
        for (CStateManager::TIdList::const_iterator current = ids.first; current != ids.second;
             ++current) {
          result.data()[result.x4_count++] = current->second;
        }
      }
    }
  }
  return result;
}

rstl::vector< TUniqueId > CEntity::FindConnectedObjects_if(
    const CStateManager& mgr, EScriptObjectState state, EScriptObjectMessage msg,
    const CValidEntityPredicate& predicate) const {
  rstl::vector< TUniqueId > result;
  for (rstl::vector< SConnection >::const_iterator it = m_conns.begin(); it != m_conns.end(); ++it) {
    if ((state == kSS_InvalidState || state == it->state) &&
        (msg == kSM_None || msg == it->msg)) {
      CStateManager::TIdListResult ids = mgr.GetIdListForScript(it->objId);
      if (!(ids.first == ids.second)) {
        result.reserve(result.size() + rstl::distance(ids.first, ids.second));
        for (CStateManager::TIdList::const_iterator current = ids.first; current != ids.second;
             ++current) {
          if (predicate.IsValid(mgr, current->second)) {
            result.data()[result.x4_count++] = current->second;
          }
        }
      }
    }
  }
  return result;
}

TUniqueId CEntity::CheckConnectedObject(const CStateManager& mgr, EScriptObjectState state,
                                        EScriptObjectMessage msg) const {
  for (rstl::vector< SConnection >::const_iterator it = m_conns.begin(); it != m_conns.end(); ++it) {
    if ((state == kSS_InvalidState || state == it->state) &&
        (msg == kSM_None || msg == it->msg)) {
      CStateManager::TIdListResult ids = mgr.GetIdListForScript(it->objId);
      if (!(ids.first == ids.second)) {
        return ids.first->second;
      }
    }
  }
  return kInvalidUniqueId;
}

TUniqueId CEntity::CheckConnectedObject_if(const CStateManager& mgr, EScriptObjectState state,
                                           EScriptObjectMessage msg,
                                           const CValidEntityPredicate& predicate) const {
  for (rstl::vector< SConnection >::const_iterator it = m_conns.begin(); it != m_conns.end(); ++it) {
    if ((state == kSS_InvalidState || state == it->state) &&
        (msg == kSM_None || msg == it->msg)) {
      CStateManager::TIdListResult ids = mgr.GetIdListForScript(it->objId);
      if (!(ids.first == ids.second)) {
        if (predicate.IsValid(mgr, ids.first->second)) {
          return ids.first->second;
        }
      }
    }
  }
  return kInvalidUniqueId;
}

CValidEntityPredicate::~CValidEntityPredicate() {}

bool CValidEntityPredicate::IsValid(const CStateManager&, TUniqueId) const { return true; }
