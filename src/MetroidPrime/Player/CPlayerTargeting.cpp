#include "MetroidPrime/Player/CPlayerTargeting.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CInGameGuiManager.hpp"
#include "MetroidPrime/CInGameGuiManagerSet.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CStaticGeometryMap.hpp"
#include "MetroidPrime/CValidEntityPredicate.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Factories/CScannableObjectInfo.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPointOfInterest.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"

static const CColor skScanPulseStart(1.f, 1.f, 0.f, 1.f);
static const CColor skScanPulseEnd(0.3f, 0.f, 0.f, 1.f);

// Guessed names; native lower-bound calls dispatch through these two comparison overloads.
struct SScanObjectLess {
  virtual bool operator()(const CPlayerTargeting::SScanObject& object, TUniqueId id) const {
    return object.mId < id;
  }

  virtual bool operator()(TUniqueId id, const CPlayerTargeting::SScanObject& object) const;
};

// Guessed name; SCNS links accept only active point-of-interest objects.
class CActiveScanPointPredicate : public CValidEntityPredicate {
public:
  // CValidEntityPredicate
  ~CActiveScanPointPredicate() override {}

  bool IsValid(const CStateManager& mgr, TUniqueId id) const override {
    const CScriptPointOfInterest* point =
        TCastToConstPtr< CScriptPointOfInterest >(mgr.GetObjectById(id));
    return point && point->GetActive();
  }
};

CPlayerTargeting::CPlayerTargeting(TUniqueId playerId)
: mPlayerId(playerId)
, mTargetId(kInvalidUniqueId)
, mResolvedTargetId(kInvalidUniqueId)
, mReserved(-1)
, mTargetTime(0.f)
, mScanTime(0.f)
, mRefreshTimer(0.f) {
  mScanObjects.reserve(62);
  CBasics::ZeroMemory(mScanObjectMembership, sizeof(mScanObjectMembership));
}

CFrustumPlanes CPlayerTargeting::GetScanFrustum(const CStateManager& mgr) const {
  const uint playerIndex = mgr.MaskUIdNumPlayers(mPlayerId);
  const CGameCamera* camera = mgr.GetCameraManager(playerIndex)->GetCurrentCamera(mgr, true);
  const float farClip = 1.75f * mgr.GetPlayer(playerIndex)->GetTweakPlayer()->GetScanningRange();
  return CFrustumPlanes(camera->GetTransform(), CMath::Deg2Rad(camera->GetFov()),
                        camera->GetAspectRatio(), camera->GetNearClipDistance(), true, farClip);
}

bool CPlayerTargeting::IsInVisibleArea(const CStateManager& mgr, const CEntity* entity) const {
  if (!entity) {
    return false;
  }

  const TAreaId areaId = entity->GetCurrentAreaId();
  const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(mPlayerId));
  if (player && areaId != player->GetCurrentAreaId()) {
    return false;
  }

  return mgr.GetWorld()->GetAreaAlways(areaId).GetOcclusionState() == CGameArea::kOS_Visible;
}

bool CPlayerTargeting::HasStaticGeometry(const CStateManager& mgr, TUniqueId id) {
  const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(id));
  if (!actor || !TCastToConstPtr< CScriptPointOfInterest >(actor)) {
    return false;
  }

  const TEditorId editorId = mgr.GetEditorIdForUniqueId(id);
  const CStaticGeometryMap* map = mgr.GetWorld()
                                      ->GetAreaAlways(actor->GetCurrentAreaId())
                                      .GetPostConstructed()
                                      ->mStaticGeometryMap.get();
  if (!map) {
    return false;
  }

  const rstl::vector< CStaticGeometryMapData::TMapping >& mappings = map->GetData().GetMappings();
  for (rstl::vector< CStaticGeometryMapData::TMapping >::const_iterator it = mappings.begin();
       it != mappings.end(); ++it) {
    if (it->second == editorId) {
      return true;
    }
  }

  return false;
}

CAABox CPlayerTargeting::GetTargetBounds(const CStateManager& mgr, TUniqueId id) {
  const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(id));
  CAABox bounds = CAABox::MakeMaxInvertedBox();
  if (actor && !TCastToConstPtr< CScriptPointOfInterest >(actor) && actor->HasModelData()) {
    const CAABox& actorBounds = actor->GetOtherBounds();
    bounds.AccumulateBounds(actorBounds.GetMinPoint());
    bounds.AccumulateBounds(actorBounds.GetMaxPoint());
  }

  return bounds;
}

bool CPlayerTargeting::AddScanObject(const CActor& actor, const CStateManager& mgr) {
  if (mScanObjects.size() == mScanObjects.capacity()) {
    return false;
  }

  const TUniqueId id = actor.GetUniqueId();
  if (!(mScanObjectMembership[id.Value() >> 3] & (1 << (id.Value() & 7)))) {
    ResolveScanTarget(mgr, id);
    const rstl::vector< SScanObject >::iterator it =
        rstl::lower_bound(mScanObjects.begin(), mScanObjects.end(), id, SScanObjectLess());
    const CColor previous = !close_enough(mRefreshTimer, 0.f) ? skScanPulseStart : CColor::Black();
    mScanObjects.insert(it, SScanObject(id, previous, gpTweakGui->GetScanVisorFadeOutTime()));
    mScanObjectMembership[id.Value() >> 3] |= 1 << (id.Value() & 7);
  }

  return true;
}

void CPlayerTargeting::UpdateScanObjects(float dt, CStateManager& mgr) {
  const CFrustumPlanes frustum(GetScanFrustum(mgr));
  const bool cull = !close_enough(mRefreshTimer, 0.f);
  rstl::vector< SScanObject >::iterator it = mScanObjects.begin();
  while (it != mScanObjects.end()) {
    const CEntity* entity = mgr.GetObjectById(it->mId);
    bool keep = false;
    if (IsInVisibleArea(mgr, entity)) {
      if (!cull) {
        keep = true;
      } else if (entity) {
        keep = TCastToConstPtr< CScriptPointOfInterest >(entity) ||
               frustum.BoxFrustumPlanesCheck(GetTargetBounds(mgr, it->mId)) != 0;
      }
    }

    if (!entity || !entity->GetActive() || !keep) {
      const TUniqueId id = it->mId;
      it = mScanObjects.erase(it);
      mScanObjectMembership[id.Value() >> 3] &= ~(1 << (id.Value() & 7));
    } else {
      if (!close_enough(it->mFadeTime, 0.f)) {
        it->mFadeTime = rstl::max_val(0.f, it->mFadeTime - dt);
      }
      ++it;
    }
  }

  if (mgr.IsMultiplayer()) {
    for (uint i = 0; i < mgr.GetNumPlayers(); ++i) {
      const CPlayer* player = mgr.GetPlayer(i);
      if (player->GetUniqueId() != mPlayerId) {
        AddScanObject(*player, mgr);
      }
    }
  } else {
    const CObjectList& actors = mgr.GetObjectListById(kOL_Actor);
    for (int index = actors.GetFirstObjectIndex(); index != -1;
         index = actors.GetNextObjectIndex(index)) {
      const CActor* actor = TCastToConstPtr< CActor >(actors[index]);
      if (!actor || !actor->GetMaterialList().HasMaterial(kMT_Scannable) || !actor->GetActive() ||
          !IsInVisibleArea(mgr, actor)) {
        continue;
      }

      const TUniqueId id = actor->GetUniqueId();
      if (mScanObjectMembership[id.Value() >> 3] & (1 << (id.Value() & 7))) {
        continue;
      }

      bool add = false;
      if (actor->HasModelData()) {
        add = id != mPlayerId &&
              (!cull || frustum.BoxFrustumPlanesCheck(GetTargetBounds(mgr, id)) != 0);
      } else if (TCastToConstPtr< CScriptPointOfInterest >(actor)) {
        add = !cull || HasStaticGeometry(mgr, id);
      }
      if (add && !AddScanObject(*actor, mgr)) {
        return;
      }
    }
  }
}

void CPlayerTargeting::Update(float dt, CStateManager& mgr) {
  // Native lazy initialization is retained; this TU has no observed use of the resulting filter.
  static const CMaterialFilter skScanFilter =
      CMaterialFilter::MakeExclude(CMaterialList(kMT_Player, kMT_ScanPassthrough));
  const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(mPlayerId));
  mScanTime += dt;
  if (player) {
    if (player->GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Scan) {
      mTargetTime += dt;
      mRefreshTimer = rstl::max_val(0.f, mRefreshTimer - dt);
      UpdateScanObjects(dt, mgr);
      if (close_enough(mRefreshTimer, 0.f) && mScanObjects.size() == mScanObjects.capacity()) {
        mRefreshTimer = 10.f;
        UpdateScanObjects(dt, mgr);
      }

      const TUniqueId nextTarget = player->GetOrbitNextTargetId();
      if (nextTarget != mTargetId) {
        rstl::vector< SScanObject >::iterator it = rstl::binary_find(
            mScanObjects.begin(), mScanObjects.end(), mTargetId, SScanObjectLess());
        if (it != mScanObjects.end()) {
          it->mPreviousColor = GetScanObjectColor(mgr, it - mScanObjects.begin());
          it->mFadeTime = gpTweakGui->GetScanVisorFadeOutTime();
        }

        mTargetId = nextTarget;
        mResolvedTargetId = ResolveScanTarget(mgr, nextTarget);
        mTargetTime = 0.f;
      }
    } else {
      mRefreshTimer = 0.f;
    }
  }
}

bool SScanObjectLess::operator()(TUniqueId id, const CPlayerTargeting::SScanObject& object) const {
  return id < object.mId;
}

int CPlayerTargeting::GetScanTargetIndex(const CStateManager& mgr, TUniqueId id) const {
  const TUniqueId resolved = ResolveScanTarget(mgr, id);
  if (resolved == kInvalidUniqueId) {
    return 0;
  }

  if (!IsInVisibleArea(mgr, mgr.GetObjectById(id))) {
    return 0;
  }

  rstl::vector< SScanObject >::const_iterator it =
      rstl::binary_find(mScanObjects.begin(), mScanObjects.end(), id, SScanObjectLess());
  if (it != mScanObjects.end()) {
    return it - mScanObjects.begin() + 2;
  }

  it = rstl::binary_find(mScanObjects.begin(), mScanObjects.end(), resolved, SScanObjectLess());
  if (it != mScanObjects.end()) {
    return it - mScanObjects.begin() + 2;
  }

  const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(resolved));
  return actor->GetMaterialList().HasMaterial(kMT_Scannable) ? 1 : 0;
}

CColor CPlayerTargeting::GetScanObjectColor(CStateManager& mgr, int index) const {
  const SScanObject& object = mScanObjects[index];
  const TUniqueId resolved = ResolveScanTarget(mgr, object.mId);
  if (resolved == mTargetId) {
    const float factor = rstl::min_val(mTargetTime / gpTweakGui->GetScanVisorBurnInTime(), 1.f);
    return CColor::Lerp(gpTweakGui->GetScanVisorBurnInColor(), GetHighlightColor(mgr, mTargetId),
                        factor);
  }

  const CColor colors[] = {CColor(0.f, 0.f, 0.f, 0.f),
                           CColor(0.f, 0.f, 0.f, 0.f),
                           gpTweakGui->GetScanVisorNonCriticalColor(),
                           gpTweakGui->GetScanVisorCriticalColor(),
                           gpTweakGui->GetScanVisorPreviouslyScannedColor(),
                           gpTweakGui->GetScanVisorCriticalPreviouslyScannedColor(),
                           gpTweakGui->GetScanVisorHackedColor()};
  const float factor = rstl::max_val(object.mFadeTime / gpTweakGui->GetScanVisorFadeOutTime(), 0.f);
  return CColor::Lerp(colors[GetScanState(mgr, resolved)], object.mPreviousColor, factor);
}

CColor CPlayerTargeting::GetHighlightColor(CStateManager& mgr, const TUniqueId& id) const {
  const CColor colors[] = {CColor(0.f, 0.f, 0.f, 0.f),
                           CColor(0.f, 0.f, 0.f, 0.f),
                           gpTweakGui->GetScanVisorHighlightColor(),
                           gpTweakGui->GetScanVisorCriticalHighlightColor(),
                           gpTweakGui->GetScanVisorPreviouslyScannedHighlightColor(),
                           gpTweakGui->GetScanVisorCriticalPreviouslyScannedHighlightColor(),
                           gpTweakGui->GetScanVisorHackedHighlightedColor()};
  return colors[GetScanState(mgr, id)];
}

CPlayerTargeting::EScanState CPlayerTargeting::GetScanState(CStateManager& mgr,
                                                            const TUniqueId& id) const {
  if (id == kInvalidUniqueId) {
    return kSS_Invalid;
  }

  if (mgr.IsMultiplayer()) {
    const CPlayer* player =
        TCastToConstPtr< CPlayer >(mgr.GetObjectById(ResolveScanTarget(mgr, id)));
    if (player) {
      return player->GetPlayerState()->GetItemCapacity(CPlayerState::kIT_HackedEffect) < 1
                 ? kSS_Unscanned
                 : kSS_Hacked;
    }
  }

  const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(ResolveScanTarget(mgr, id)));
  if (!actor) {
    return kSS_Invalid;
  }

  const CScannableObjectInfo* scan = actor->GetScannableObjectInfo();
  if (!scan) {
    return kSS_Invalid;
  }

  if (mgr.PlayerState(mgr.MaskUIdNumPlayers(mPlayerId))
          ->GetScanTime(scan->GetScannableObjectId()) >= 0.9999999f) {
    return scan->IsCritical() ? kSS_CriticalScanned : kSS_Scanned;
  }

  return scan->IsCritical() ? kSS_CriticalUnscanned : kSS_Unscanned;
}

void CPlayerTargeting::PrepareStaticGeometry(const CStateManager& mgr,
                                             const TAreaId& areaId) const {
  const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(mPlayerId));
  if (!player || player->GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::kPV_Scan) {
    return;
  }
  const CStaticGeometryMap* map =
      mgr.GetWorld()->GetAreaAlways(areaId).GetPostConstructed()->mStaticGeometryMap.get();
  if (!map) {
    gpRender->DisablePVS(areaId.Value());
    return;
  }
  const rstl::vector< CStaticGeometryMapData::TMapping >& mappings = map->GetData().GetMappings();
  TEditorId previousId = kInvalidEditorId;
  int paletteIndex = -1;
  rstl::vector< rstl::pair< int, int > > visible;
  visible.reserve(mappings.size());
  for (rstl::vector< CStaticGeometryMapData::TMapping >::const_iterator it = mappings.begin();
       it != mappings.end(); ++it) {
    if (it->second != previousId) {
      paletteIndex = GetScanTargetIndex(mgr, mgr.GetIdForScript(it->second));
      previousId = it->second;
    }
    if (paletteIndex > 0) {
      visible.push_back_unsafe(rstl::pair< int, int >(it->first, paletteIndex));
    }
  }
  gpRender->EnablePVS(areaId.Value(), visible);
}

void CPlayerTargeting::Draw(CStateManager& mgr, const CInGameGuiManagerSet& gui) const {
  CColor palette[64] = {
      CColor(0.f, 0.f, 0.f, 0.f),
      CColor::Lerp(skScanPulseStart, skScanPulseEnd,
                   (1.f + CMath::FastCosR(3.f * CGraphics::GetSecondsMod900())) * 0.5f)};
  const int count = mScanObjects.size();
  for (int i = 2; i < count + 2; ++i) {
    palette[i] = GetScanObjectColor(mgr, i - 2);
  }

  const float transition = mgr.GetPlayerState()->GetVisorTransitionFactor();
  if (transition < 1.f) {
    for (uint i = 0; i < 64; ++i) {
      palette[i] = CColor::Lerp(CColor::Black(), palette[i], transition);
    }
  }

  const uint playerIndex = mgr.MaskUIdNumPlayers(mPlayerId);
  const CGameCamera* camera = mgr.GetCameraManager(playerIndex)->GetCurrentCamera(mgr, true);
  const CVector3f direction = camera->GetTransform().GetForward();
  gui.GetPlayerGuiManager(mgr.GetCurrentRenderPlayerIndex())
      .DrawScanVisor(
          mScanTime, mgr,
          CColor::Lerp(CColor::Black(), gpTweakGui->GetScanVisorSweepBarColor(), transition),
          CColor::Lerp(CColor::White(), gpTweakGui->GetScanVisorInactiveColor(), transition),
          CColor::Lerp(CColor::White(), gpTweakGui->GetScanVisorInactiveExternalColor(),
                       transition),
          palette, count + 2, direction);
}

TUniqueId CPlayerTargeting::ResolveScanTarget(const CStateManager& mgr, TUniqueId id) const {
  const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(id));
  if (!actor) {
    return kInvalidUniqueId;
  }

  if (actor->GetScannableObjectInfo() && actor->GetActive()) {
    return id;
  }

  return actor->CheckConnectedObject_if(mgr, kSS_ScanSource, kSM_None, CActiveScanPointPredicate());
}

TUniqueId CPlayerTargeting::GetScanTargetId(const CStateManager& mgr, int paletteIndex) const {
  const int index = paletteIndex - 2;
  if (index >= 0 && index < mScanObjects.size()) {
    const TUniqueId id = mScanObjects[index].mId;
    return ResolveScanTarget(mgr, id);
  }

  return kInvalidUniqueId;
}
