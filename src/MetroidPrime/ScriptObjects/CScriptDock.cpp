#include "MetroidPrime/ScriptObjects/CScriptDock.hpp"

#include "Kyoto/Math/CPlane.hpp"
#include "MetroidPrime/CPortalTransition.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Enemies/CMetroidAlpha.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDoor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPortalTransition.hpp"
#include "MetroidPrime/TCastTo.hpp"

CScriptDock::CScriptDock(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         const CVector3f& position, const CVector3f& extent, int dock, TAreaId area,
                         int dockReferenceCount, bool loadConnected, bool isVirtual,
                         bool showSoftTransition)
: CPhysicsActor(uid, name, info, 0, CTransform4f::Translate(position), CModelData(),
                CMaterialList(kMT_Trigger, kMT_Immovable, kMT_AIBlock),
                CAABox(-(0.5f * extent), 0.5f * extent), SMoverData(1.f), CActorParameters(),
                StepData(0.3f, 0.3f, 0))
, mDockReferenceCount(dockReferenceCount)
, mDock(dock)
, mArea(area)
, mDockState(kDS_InNextRoom)
, mDockReferenced(false)
, mLoadConnected(loadConnected && !isVirtual)
, mAreaPostConstructed(false)
, mIsVirtual(isVirtual)
, mShowSoftTransition(showSoftTransition) {}

CScriptDock::~CScriptDock() {}

void CScriptDock::Touch(CActor& actor, CStateManager& mgr) {
  if (mDockState == kDS_InNextRoom) {
    return;
  }

  if (TCastToPtr< CPlayer >(actor)) {
    mDockState = kDS_PlayerTouched;
  }
  if (CMetroidAlpha* metroid = TCastToPtr< CMetroidAlpha >(actor)) {
    metroid->OnDockTouch(mgr);
  }
}

rstl::optional_object< CAABox > CScriptDock::GetTouchBounds() const {
  if (mDockState == kDS_InNextRoom || mIsVirtual) {
    return rstl::optional_object_null();
  }
  return GetBoundingBox();
}

void CScriptDock::SetLoadConnected(CStateManager& mgr, bool loadConnected, bool pauseValidation) {
  CGameArea* area = mgr.World()->Area(mArea);
  const IGameArea::Dock& dock = area->GetDock(mDock);
  const TAreaId connectedArea = dock.GetConnectedAreaId(dock.GetReferenceCount());
  if (connectedArea != kInvalidAreaId) {
    mgr.World()->Area(connectedArea)->SetValidationPaused(pauseValidation);
  }

  if (loadConnected != dock.GetShouldLoadOther(dock.GetReferenceCount())) {
    area->DockNC(mDock).SetShouldLoadOther(dock.GetReferenceCount(), loadConnected);
  }
}

void CScriptDock::InitializeConnectedArea(CStateManager& mgr) {
  SetLoadConnected(mgr, mLoadConnected, false);
}

void CScriptDock::AreaUnloaded(CStateManager&) {}

void CWorld::PropogateAreaChain(CGameArea::EOcclusionState state, CGameArea* area, CWorld* world) {
  if (!area->IsLoaded() || state == area->GetOcclusionState()) {
    return;
  }

  if (state == CGameArea::kOS_Visible) {
    area->SetOcclusionState(CGameArea::kOS_Visible);
  }
  for (CGameArea::CChainIterator it = world->ChainHead(kC_Alive); it != skGlobalNonConstEnd; ++it) {
    if (&*it != area && it->GetOcclusionState() == CGameArea::kOS_Visible) {
      it->OtherAreaOcclusionChanged();
    }
  }
  for (CGameArea::CChainIterator it = world->ChainHead(kC_Alive); it != skGlobalNonConstEnd; ++it) {
    if (&*it != area && it->GetOcclusionState() == CGameArea::kOS_Occluded) {
      it->OtherAreaOcclusionChanged();
    }
  }
  if (state == CGameArea::kOS_Occluded) {
    area->SetOcclusionState(CGameArea::kOS_Occluded);
  }
}

void CGameArea::AddDock(TUniqueId uid) { mPostConstructed->mDockIds.push_back(uid); }

void CScriptDock::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_XCRT: {
    CGameArea* area = mgr.World()->Area(mArea);
    if (mDock >= area->GetDockCount()) {
      return;
    }
    IGameArea::Dock& dock = area->DockNC(mDock);
    if (!dock.IsReferenced()) {
      dock.SetReferenceCount(mDockReferenceCount);
    }
    break;
  }
  case kSM_XDelete:
    AreaUnloaded(mgr);
    break;
  case kSM_XALD:
    mgr.World()->Area(GetCurrentAreaId())->AddDock(GetUniqueId());
    break;
  case kSM_XWLD: {
    UpdateAreaActivateFlags(mgr);
    CMaterialList include = GetMaterialFilter().GetIncludeList();
    include.Add(kMT_AIBlock);
    SetMaterialFilter(
        CMaterialFilter::MakeIncludeExclude(include, GetMaterialFilter().GetExcludeList()));
    break;
  }
  case kSM_Unload:
  case kSM_SetToZero: {
    if (mgr.GetNextAreaId() != mArea) {
      return;
    }
    SetLoadConnected(mgr, false, false);

    const IGameArea::Dock& dock = mgr.GetWorld()->GetAreaAlways(mArea).GetDock(mDock);
    const TAreaId connectedArea = dock.GetConnectedAreaId(dock.GetReferenceCount());
    const rstl::list< CEntity* >& doors = mgr.GetDoorList();
    for (rstl::list< CEntity* >::const_iterator it = doors.begin(); it != doors.end(); ++it) {
      CScriptDoor* door = static_cast< CScriptDoor* >(*it);
      if (door != nullptr && door->IsConnectedToArea(mgr, connectedArea)) {
        door->ForceClosed(mgr);
      }
    }
    break;
  }
  case kSM_InternalMessage00: {
    IGameArea::Dock& dock = mgr.World()->Area(mArea)->DockNC(mDock);
    dock.SetLoadOtherBlocked(dock.GetReferenceCount(), true);
    break;
  }
  case kSM_Load:
    SetLoadConnected(mgr, true, true);
    break;
  case kSM_SetToMax: {
    if (mgr.GetNextAreaId() != mArea) {
      return;
    }
    if (!mIsVirtual) {
      SetLoadConnected(mgr, true, false);
      break;
    }

    for (int i = 0; i < mgr.GetWorld()->GetNumAreas(); ++i) {
      CGameArea* area = mgr.World()->Area(TAreaId(i));
      for (int j = 0; j < area->GetDockCount(); ++j) {
        IGameArea::Dock& dock = area->DockNC(j);
        dock.SetShouldLoadOther(dock.GetReferenceCount(), false);
      }
    }
    SetLoadConnected(mgr, true, false);

    const IGameArea::Dock& dock = mgr.GetWorld()->GetAreaAlways(mArea).GetDock(mDock);
    mgr.SetPendingDockTransition(GetCurrentConnectedAreaId(mgr),
                                 dock.GetOtherDockNumber(dock.GetReferenceCount()),
                                 mShowSoftTransition);
    const TUniqueId transitionId = FindConnectedObject(mgr, kSS_Play, kSM_None);
    if (const CScriptPortalTransition* portal =
            TCastToConstPtr< CScriptPortalTransition >(mgr.GetObjectById(transitionId))) {
      rstl::single_ptr< CPortalTransition > transition = portal->CreateTransition(mgr);
      mgr.SetPortalTransition(transition);
    }
    break;
  }
  case kSM_Increment:
    SetLoadConnected(mgr, true, false);
    // Fall through.
  case kSM_Decrement: {
    TAreaId areaId = mArea;
    if (mgr.GetNextAreaId() == mArea) {
      const IGameArea::Dock& dock =
          mgr.GetWorld()->GetAreaAlways(mgr.GetNextAreaId()).GetDock(mDock);
      areaId = dock.GetConnectedAreaId(dock.GetReferenceCount());
    }
    if (mgr.GetWorld()->DoesAreaExist(areaId) && mgr.GetWorld()->IsAreaValid(areaId)) {
      CWorld::PropogateAreaChain(msg.GetMessage() == kSM_Increment ? CGameArea::kOS_Visible
                                                                   : CGameArea::kOS_Occluded,
                                 mgr.World()->Area(areaId), mgr.World());
    }
    break;
  }
  default:
    CActor::AcceptScriptMsg(mgr, msg);
    break;
  }
}

void CScriptDock::Think(float dt, CStateManager& mgr) {
  if (mDockReferenced) {
    UpdateAreaActivateFlags(mgr);
    mDockReferenced = false;
  }

  const IGameArea::Dock& gameDock = mgr.GetWorld()->GetAreaAlways(mArea).GetDock(mDock);
  const TAreaId connectedArea = gameDock.GetConnectedAreaId(gameDock.GetReferenceCount());
  if (connectedArea != kInvalidAreaId) {
    const CGameArea& area = mgr.GetWorld()->GetAreaAlways(connectedArea);
    if (mAreaPostConstructed != area.IsLoaded()) {
      mAreaPostConstructed = area.IsLoaded();
      SendScriptMsgs(mAreaPostConstructed ? kSS_MaxReached : kSS_Zero, mgr, kInvalidUniqueId,
                     kSM_None);
    }
  }

  if (mgr.GetNextAreaId() != mArea) {
    mDockState = kDS_InNextRoom;
  } else if (mDockState == kDS_InNextRoom) {
    mDockState = kDS_InSourceRoom;
  } else if (mDockState == kDS_PlayerTouched) {
    mDockState = kDS_EnterNextArea;
  } else if (mDockState == kDS_EnterNextArea) {
    if (HasPointCrossedDock(mgr, mgr.GetPlayer(0)->GetTranslation())) {
      const IGameArea::Dock& dock =
          mgr.GetWorld()->GetAreaAlways(mgr.GetNextAreaId()).GetDock(mDock);
      const TAreaId nextArea = dock.GetConnectedAreaId(dock.GetReferenceCount());
      if (nextArea != kInvalidAreaId && mgr.GetWorld()->GetAreaAlways(nextArea).IsLoaded()) {
        mgr.SetCurrentAreaId(nextArea);
        if (CScriptDock* nextDock =
                TCastToPtr< CScriptDock >(mgr.ObjectById(GetConnectedScriptDockId(mgr)))) {
          nextDock->SetLoadConnected(mgr, true, false);
        }
      }
    }
    mDockState = kDS_InSourceRoom;
  }
}

bool CScriptDock::HasPointCrossedDock(const CStateManager& mgr, const CVector3f& point) const {
  const IGameArea::Dock& dock = mgr.GetWorld()->GetAreaAlways(mgr.GetNextAreaId()).GetDock(mDock);
  const rstl::reserved_vector< CVector3f, 4 >& vertices = dock.GetPlaneVertices();
  return CPlane(vertices[0], vertices[1], vertices[2]).IsFacing(point);
}

CPlane CScriptDock::GetPlane(const CStateManager& mgr) const {
  const IGameArea::Dock& dock = mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetDock(mDock);
  const rstl::reserved_vector< CVector3f, 4 >& vertices = dock.GetPlaneVertices();
  return CPlane(vertices[0], vertices[1], vertices[2]);
}

int CScriptDock::GetDockReference(const CStateManager& mgr) const {
  return mgr.GetWorld()->GetAreaAlways(mArea).GetDock(mDock).GetReferenceCount();
}

void CScriptDock::UpdateAreaActivateFlags(CStateManager& mgr) {
  if (mArea.Value() >= mgr.GetWorld()->GetNumAreas()) {
    return;
  }
  const CGameArea& area = mgr.GetWorld()->GetAreaAlways(mArea);
  if (mDock >= area.GetDockCount()) {
    return;
  }

  const IGameArea::Dock& dock = area.GetDock(mDock);
  for (int i = 0; i < dock.GetDockRefs().size(); ++i) {
    const TAreaId connectedArea = dock.GetConnectedAreaId(i);
    if (connectedArea != kInvalidAreaId) {
      mgr.World()->Area(connectedArea)->SetActive(dock.GetReferenceCount() == i);
    }
  }
  mgr.SetCurrentAreaId(mgr.GetNextAreaId());
}

TAreaId CScriptDock::GetCurrentConnectedAreaId(const CStateManager& mgr) const {
  if (mgr.GetWorld()->GetNumAreas() > mArea.Value()) {
    const CGameArea& area = mgr.GetWorld()->GetAreaAlways(mArea);
    if (area.GetDockCount() > mDock) {
      const IGameArea::Dock& dock = area.GetDock(mDock);
      return dock.GetConnectedAreaId(dock.GetReferenceCount());
    }
  }
  return kInvalidAreaId;
}

TUniqueId CScriptDock::GetConnectedScriptDockId(const CStateManager& mgr) const {
  const IGameArea::Dock& dock = mgr.GetWorld()->GetAreaAlways(mArea).GetDock(mDock);
  const int otherDock = dock.GetOtherDockNumber(dock.GetReferenceCount());
  const TAreaId connectedArea = dock.GetConnectedAreaId(dock.GetReferenceCount());
  const CObjectList& objects = *mgr.GetWorld()->GetAreaAlways(connectedArea).ObjectList();
  for (int i = objects.GetFirstObjectIndex(); i != -1; i = objects.GetNextObjectIndex(i)) {
    if (const CScriptDock* nextDock = TCastToConstPtr< CScriptDock >(objects[i])) {
      if (nextDock->GetDockId() == otherDock) {
        return nextDock->GetUniqueId();
      }
    }
  }
  return kInvalidUniqueId;
}
