#include "MetroidPrime/ScriptObjects/CScriptDock.hpp"

#include "Kyoto/Math/CPlane.hpp"
#include "MetroidPrime/CPortalTransition.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Enemies/CMetroid.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDock.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDoor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPortalTransition.hpp"
#include "MetroidPrime/TCastTo.hpp"

CEntity* LoadDock(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrDock sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrDock.inc"

  return rs_new CScriptDock(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                            LdrToEntityInfo(info, sldrThis.editorProperties),
                            sldrThis.editorProperties.transform.position,
                            sldrThis.editorProperties.transform.scale, sldrThis.dockNumber,
                            sldrThis.areaNumber, 0, sldrThis.loadConnectedImmediate,
                            sldrThis.isVirtual, sldrThis.showSoftTransition);
}

CScriptDock::CScriptDock(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         const CVector3f& position, const CVector3f& extent, int dock, TAreaId area,
                         int dockReferenceCount, bool loadConnected, bool isVirtual,
                         bool showSoftTransition)
: CPhysicsActor(
      uid, name, info, 0, CTransform4f::Translate(position), CModelData::CModelDataNull(),
      CMaterialList(kMT_Trigger, kMT_Immovable, kMT_AIBlock),
      CAABox(CVector3f(-(0.5f * extent.GetX()), -(0.5f * extent.GetY()), -(0.5f * extent.GetZ())),
             CVector3f(0.5f * extent.GetX(), 0.5f * extent.GetY(), 0.5f * extent.GetZ())),
      SMoverData(1.f), CActorParameters::None(), skDefaultStepData)
, mDockReferenceCount(dockReferenceCount)
, mDock(dock)
, mArea(area)
, mDockState(kDS_InNextRoom)
, mDockReferenced(false)
, mLoadConnected(isVirtual ? false : loadConnected)
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
  if (CMetroid* metroid = TCastToPtr< CMetroid >(actor)) {
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
  if (dock.GetConnectedAreaId(dock.GetReferenceCount()) != kInvalidAreaId) {
    mgr.World()
        ->Area(dock.GetConnectedAreaId(dock.GetReferenceCount()))
        ->SetValidationPaused(pauseValidation);
  }

  const bool shouldLoad = dock.GetShouldLoadOther(dock.GetReferenceCount());
  if (loadConnected != shouldLoad) {
    area->DockNC(mDock).SetShouldLoadOther(dock.GetReferenceCount(), loadConnected);
  }
}

void CScriptDock::InitializeConnectedArea(CStateManager& mgr) {
  SetLoadConnected(mgr, mLoadConnected, false);
}

void CScriptDock::AreaUnloaded(CStateManager&) {}

void CWorld::PropogateAreaChain(CGameArea::EOcclusionState state, CGameArea* area, CWorld* world) {
  if (!area->IsLoaded()) {
    return;
  }
  if (state == area->GetOcclusionState()) {
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
  const EScriptObjectMessage message = msg.GetMessage();
  switch (message) {
  case kSM_Create: {
    CGameArea* area = mgr.World()->Area(GetAreaId());
    if (area->GetDockCount() <= mDock) {
      return;
    }
    IGameArea::Dock& dock = area->DockNC(mDock);
    if (!dock.IsReferenced()) {
      dock.SetReferenceCount(mDockReferenceCount);
    }
    break;
  }
  case kSM_Delete:
    AreaUnloaded(mgr);
    break;
  case kSM_AreaLoaded:
    mgr.World()->Area(GetCurrentAreaId())->AddDock(GetUniqueId());
    break;
  case kSM_WorldLoaded: {
    UpdateAreaActivateFlags(mgr);
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
        GetMaterialFilter().GetIncludeList().Union(CMaterialList(kMT_AIBlock)),
        GetMaterialFilter().GetExcludeList()));
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
    if (mIsVirtual) {
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
      if (const CScriptPortalTransition* portal = TCastToConstPtr< CScriptPortalTransition >(
              mgr.GetObjectById(FindConnectedObject(mgr, kSS_Play, kSM_None)))) {
        rstl::single_ptr< CPortalTransition > transition(portal->CreateTransition(mgr));
        mgr.SetPortalTransition(transition);
      }
    } else {
      SetLoadConnected(mgr, true, false);
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
    if (areaId.Value() >= 0 && mgr.GetWorld()->GetNumAreas() > areaId.Value() &&
        mgr.GetWorld()->IsAreaValid(areaId)) {
      CWorld::PropogateAreaChain(message == kSM_Increment ? CGameArea::kOS_Visible
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
      if (mAreaPostConstructed) {
        SendScriptMsgs(kSS_MaxReached, mgr);
      } else {
        SendScriptMsgs(kSS_Zero, mgr);
      }
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
  const CVector3f* vertices = dock.GetPlaneVertices().data();
  const CPlane plane(vertices[0], vertices[1], vertices[2]);
  return plane.IsFacing(point);
}

CPlane CScriptDock::GetPlane(const CStateManager& mgr) const {
  const IGameArea::Dock& dock = mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetDock(mDock);
  const CVector3f* vertices = dock.GetPlaneVertices().data();
  const CPlane plane(vertices[0], vertices[1], vertices[2]);
  return plane;
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
  const int count = dock.GetDockRefs().size();
  for (int i = 0; i < count; ++i) {
    const bool active = dock.GetReferenceCount() == i;
    const TAreaId connectedArea = dock.GetConnectedAreaId(i);
    if (connectedArea != kInvalidAreaId) {
      mgr.World()->Area(connectedArea)->SetActive(active);
    }
  }
  mgr.SetCurrentAreaId(mgr.GetNextAreaId());
}

TAreaId CScriptDock::GetCurrentConnectedAreaId(const CStateManager& mgr) const {
  if (mgr.GetWorld()->GetNumAreas() > mArea.Value()) {
    const CGameArea& area = mgr.GetWorld()->GetAreaAlways(TAreaId(mArea));
    if (area.GetDockCount() > mDock) {
      const IGameArea::Dock& dock = area.GetDock(mDock);
      return dock.GetConnectedAreaId(dock.GetReferenceCount());
    }
  }
  return kInvalidAreaId;
}

TUniqueId CScriptDock::GetConnectedScriptDockId(const CStateManager& mgr) const {
  const TAreaId area = mArea;
  const IGameArea::Dock& dock = mgr.GetWorld()->GetAreaAlways(area).GetDock(mDock);
  const int otherDock = dock.GetOtherDockNumber(dock.GetReferenceCount());
  const TAreaId connectedArea = dock.GetConnectedAreaId(dock.GetReferenceCount());
  CObjectList& objects =
      *const_cast< CGameArea& >(mgr.GetWorld()->GetAreaAlways(connectedArea)).ObjectList();
  for (int i = objects.GetFirstObjectIndex(); i != -1; i = objects.GetNextObjectIndex(i)) {
    if (const CScriptDock* nextDock = TCastToConstPtr< CScriptDock >(objects[i])) {
      if (nextDock->GetDockId() == otherDock) {
        return nextDock->GetUniqueId();
      }
    }
  }
  return kInvalidUniqueId;
}
