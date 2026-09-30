#include "MetroidPrime/CEntity.hpp"

#include "MetroidPrime/CStateManager.hpp"

rstl::vector< SConnection > CEntity::mNullConnectionList;

CEntityInfo CEntity::mNullEntityInfo =
    CEntityInfo(kInvalidAreaId, mNullConnectionList, true, kInvalidEditorId);

CEntityInfo::CEntityInfo(TAreaId aid, const rstl::vector< SConnection >& connections, bool isActive,
                         TEditorId eid)
: mAreaId(aid)
, mConnections(connections)
, mEditorId(eid)
, mActive(isActive)
, mUpdateWhileOccluded(true)
, mUpdateDuringCinematicSkip(true) {}

CEntity::CEntity(TUniqueId id, const CEntityInfo& info, const rstl::string& name, uint castFlags)
: mAreaId(info.GetAreaId())
, mUniqueId(id)
, mEditorId(info.GetEditorId())
, mConnections(info.GetConnectionList())
, mActive(info.GetActive())
, mNotInArea(mAreaId == kInvalidAreaId)
, mCastFlags(castFlags)
, mUpdateWhileOccluded(info.GetUpdateWhileOccluded())
, mUpdateDuringCinematicSkip(info.GetUpdateDuringCinematicSkip()) {}

CEntity::~CEntity() {}

void CEntity::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Activate:
    if (!mActive) {
      SetActive(true);
      SendScriptMsgs(kSS_Active, mgr, kInvalidUniqueId, kSM_None);
    }
    break;
  case kSM_Deactivate:
    if (mActive) {
      SetActive(false);
      SendScriptMsgs(kSS_Inactive, mgr, kInvalidUniqueId, kSM_None);
    }
    break;
  case kSM_ToggleActive: {
    EScriptObjectMessage next = mActive ? kSM_Deactivate : kSM_Activate;
    CScriptMsg newMsg(msg.GetUnk(), msg.GetOriginator(), msg.GetId(), next, msg.GetState());
    AcceptScriptMsg(mgr, newMsg);
    break;
  }
  }
}

void CEntity::SendScriptMsgs(EScriptObjectState state, CStateManager& mgr, TUniqueId id,
                             EScriptObjectMessage skipMsg) {
  rstl::vector< SConnection >::const_iterator it = mConnections.begin();
  for (; it != mConnections.end(); ++it) {
    if (it->state == state && it->msg != skipMsg) {
      CStateManager::TIdListResult search = mgr.GetIdListForScript(it->objId);
      CStateManager::TIdList::const_iterator current = search.first;
      CStateManager::TIdList::const_iterator end = search.second;
      while (current != end) {
        mgr.SendScriptMsg(CScriptMsg(GetUniqueId(), id, current->second, it->msg, it->state));
        ++current;
      }
    }
  }
}

void CEntity::PreThink(float dt, CStateManager& mgr) {}

void CEntity::Think(float dt, CStateManager& mgr) {}

void CEntity::SetActive(const bool active) { mActive = active; }

void CEntity::SendActive(CStateManager& mgr, bool active) {
  if (active != GetActive()) {
    mgr.SendScriptMsg(this, GetUniqueId(), active ? kSM_Activate : kSM_Deactivate,
                      kInvalidUniqueId);
  }
}

TAreaId CEntity::GetAreaIdForPersistence() const { return mNotInArea ? kInvalidAreaId : mAreaId; }

TUniqueId CEntity::FindConnectedObject(const CStateManager& mgr, EScriptObjectState state,
                                       EScriptObjectMessage msg) const {
  for (rstl::vector< SConnection >::const_iterator it = mConnections.begin();
       it != mConnections.end(); ++it) {
    if ((state == kSS_InvalidState || state == it->state) && (msg == kSM_None || msg == it->msg)) {
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
  for (rstl::vector< SConnection >::const_iterator it = mConnections.begin();
       it != mConnections.end(); ++it) {
    if ((state == kSS_InvalidState || state == it->state) && (msg == kSM_None || msg == it->msg)) {
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
  for (rstl::vector< SConnection >::const_iterator it = mConnections.begin();
       it != mConnections.end(); ++it) {
    if ((state == kSS_InvalidState || state == it->state) && (msg == kSM_None || msg == it->msg)) {
      CStateManager::TIdListResult ids = mgr.GetIdListForScript(it->objId);
      if (!(ids.first == ids.second)) {
        result.reserve(result.size() + rstl::distance(ids.first, ids.second));
        for (CStateManager::TIdList::const_iterator current = ids.first; current != ids.second;
             ++current) {
          result.data()[result.mCount++] = current->second;
        }
      }
    }
  }
  return result;
}

rstl::vector< TUniqueId >
CEntity::FindConnectedObjects_if(const CStateManager& mgr, EScriptObjectState state,
                                 EScriptObjectMessage msg,
                                 const CValidEntityPredicate& predicate) const {
  rstl::vector< TUniqueId > result;
  for (rstl::vector< SConnection >::const_iterator it = mConnections.begin();
       it != mConnections.end(); ++it) {
    if ((state == kSS_InvalidState || state == it->state) && (msg == kSM_None || msg == it->msg)) {
      CStateManager::TIdListResult ids = mgr.GetIdListForScript(it->objId);
      if (!(ids.first == ids.second)) {
        result.reserve(result.size() + rstl::distance(ids.first, ids.second));
        for (CStateManager::TIdList::const_iterator current = ids.first; current != ids.second;
             ++current) {
          if (predicate.IsValid(mgr, current->second)) {
            result.data()[result.mCount++] = current->second;
          }
        }
      }
    }
  }
  return result;
}

TUniqueId CEntity::CheckConnectedObject(const CStateManager& mgr, EScriptObjectState state,
                                        EScriptObjectMessage msg) const {
  for (rstl::vector< SConnection >::const_iterator it = mConnections.begin();
       it != mConnections.end(); ++it) {
    if ((state == kSS_InvalidState || state == it->state) && (msg == kSM_None || msg == it->msg)) {
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
  for (rstl::vector< SConnection >::const_iterator it = mConnections.begin();
       it != mConnections.end(); ++it) {
    if ((state == kSS_InvalidState || state == it->state) && (msg == kSM_None || msg == it->msg)) {
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
