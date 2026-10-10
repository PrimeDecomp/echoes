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
    if (point) {
      return point->GetActive();
    }
    return false;
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
  if (actor && TCastToConstPtr< CScriptPointOfInterest >(actor)) {
    const TEditorId editorId = mgr.GetEditorIdForUniqueId(id);
    const TAreaId areaId = actor->GetCurrentAreaId();
    const CStaticGeometryMap* map =
        mgr.GetWorld()->GetAreaAlways(areaId).GetPostConstructed()->mStaticGeometryMap.get();
    if (map) {
      const rstl::vector< CStaticGeometryMapData::TMapping >& mappings =
          map->GetData().GetMappings();
      for (rstl::vector< CStaticGeometryMapData::TMapping >::const_iterator it = mappings.begin();
           it != mappings.end(); ++it) {
        if (it->second == editorId) {
          return true;
        }
      }
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
  const int bit = 1 << (id.Value() & 7);
  uchar* bits = mScanObjectMembership;
  uchar& membership = bits[id.Value() >> 3];
  if (membership & bit) {
    return true;
  }

  ResolveScanTarget(mgr, id);
  const rstl::vector< SScanObject >::iterator it =
      rstl::lower_bound(mScanObjects.begin(), mScanObjects.end(), id, SScanObjectLess());
  const CColor previous = close_enough(mRefreshTimer, 0.f) ? CColor::Black() : skScanPulseStart;
  mScanObjects.insert(it, SScanObject(id, previous, gpTweakGui->GetScanVisorFadeOutTime()));
  membership |= bit;
  return true;
}

void CPlayerTargeting::UpdateScanObjects(float dt, CStateManager& mgr) {
  const CFrustumPlanes frustum = GetScanFrustum(mgr);
  const bool cull = !close_enough(mRefreshTimer, 0.f);
  rstl::vector< SScanObject >::iterator it = mScanObjects.begin();
  while (it != mScanObjects.end()) {
    SScanObject& obj = *it;
    const float fadeTime = obj.mFadeTime;
    const CEntity* entity = mgr.GetObjectById(TUniqueId(obj.mId));
    bool keep = false;
    if (IsInVisibleArea(mgr, entity)) {
      if (!cull) {
        keep = true;
      } else if (entity) {
        if (TCastToConstPtr< CScriptPointOfInterest >(entity)) {
          keep = true;
        } else {
          const CAABox bounds = GetTargetBounds(mgr, TUniqueId(obj.mId));
          keep = frustum.BoxFrustumPlanesCheck(bounds) != 0;
        }
      }
    }

    if (!entity || !entity->GetActive() || !keep) {
      const TUniqueId id = it->mId;
      it = mScanObjects.erase(it);
      mScanObjectMembership[id.Value() >> 3] &= ~(1 << (id.Value() & 7));
    } else {
      if (!close_enough(obj.mFadeTime, 0.f)) {
        obj.mFadeTime = rstl::max_val(0.f, fadeTime - dt);
      }
      ++it;
    }
  }

  if (mgr.IsMultiplayer()) {
    for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
      const CPlayer* player = mgr.GetPlayer(i);
      if (player->GetUniqueId() != mPlayerId) {
        AddScanObject(*player, mgr);
      }
    }
  } else {
    const CObjectList& actors = mgr.GetObjectListById(kOL_Actor);
    for (int index = actors.GetFirstObjectIndex(); index != -1;
         index = actors.GetNextObjectIndex(index)) {
      const CActor* const actor = TCastToConstPtr< CActor >(actors[index]);
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
        if (mPlayerId != id) {
          if (cull) {
            const CAABox bounds = GetTargetBounds(mgr, id);
            add = frustum.BoxFrustumPlanesCheck(bounds) != 0;
          } else {
            add = true;
          }
        }
      } else if (TCastToConstPtr< CScriptPointOfInterest >(actor)) {
        if (cull) {
          add = HasStaticGeometry(mgr, id);
        } else {
          add = true;
        }
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
    switch (player->GetPlayerState()->GetActiveVisor(mgr)) {
    case CPlayerState::kPV_Scan: {
      mTargetTime += dt;
      mRefreshTimer = rstl::max_val(0.f, mRefreshTimer - dt);
      UpdateScanObjects(dt, mgr);
      if (close_enough(mRefreshTimer, 0.f) && mScanObjects.size() == mScanObjects.capacity()) {
        mRefreshTimer = 10.f;
        UpdateScanObjects(dt, mgr);
      }

      TUniqueId nextTarget = player->GetOrbitNextTargetId();
      if (nextTarget != mTargetId) {
        rstl::vector< SScanObject >::iterator it = rstl::binary_find(
            mScanObjects.begin(), mScanObjects.end(), mTargetId, SScanObjectLess());
        if (it != mScanObjects.end()) {
          it->mPreviousColor = GetScanObjectColor(mgr, rstl::distance(mScanObjects.begin(), it));
          it->mFadeTime = gpTweakGui->GetScanVisorFadeOutTime();
        }

        mTargetId = nextTarget;
        mResolvedTargetId = ResolveScanTarget(mgr, nextTarget);
        mTargetTime = 0.f;
      }
      break;
    }
    default:
      mRefreshTimer = 0.f;
      break;
    }
  }
}

bool SScanObjectLess::operator()(TUniqueId id, const CPlayerTargeting::SScanObject& object) const {
  return id < object.mId;
}

int CPlayerTargeting::GetScanTargetIndex(const CStateManager& mgr, const TUniqueId& id) const {
  const TUniqueId resolved = ResolveScanTarget(mgr, id);
  if (resolved == kInvalidUniqueId || !IsInVisibleArea(mgr, mgr.GetObjectById(id))) {
    return 0;
  }

  rstl::vector< SScanObject >::const_iterator it =
      rstl::binary_find(mScanObjects.begin(), mScanObjects.end(), id, SScanObjectLess());
  if (it != mScanObjects.end()) {
    return rstl::distance(mScanObjects.begin(), it) + 2;
  }

  rstl::vector< SScanObject >::const_iterator resolvedIt =
      rstl::binary_find(mScanObjects.begin(), mScanObjects.end(), resolved, SScanObjectLess());
  if (resolvedIt != mScanObjects.end()) {
    return rstl::distance(mScanObjects.begin(), resolvedIt) + 2;
  }

  const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(resolved));
  return actor->GetMaterialList().HasMaterial(kMT_Scannable) ? 1 : 0;
}

CColor CPlayerTargeting::GetScanObjectColor(CStateManager& mgr, int index) const {
  const TUniqueId resolved = ResolveScanTarget(mgr, TUniqueId(mScanObjects[index].mId));
  if (resolved == mTargetId) {
    const float factor = rstl::min_val(mTargetTime / gpTweakGui->GetScanVisorBurnInTime(), 1.f);
    const CColor color = CColor::Lerp(gpTweakGui->GetScanVisorBurnInColor(),
                                      GetHighlightColor(mgr, mTargetId), factor);
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
  const EScanState state = GetScanState(mgr, resolved);
  const float fadeTime = mScanObjects[index].mFadeTime;
  const float factor = rstl::max_val(fadeTime / gpTweakGui->GetScanVisorFadeOutTime(), 0.f);
  return CColor::Lerp(colors[state], mScanObjects[index].mPreviousColor, factor);
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
      return player->GetPlayerState()->GetItemCapacity(CPlayerState::kIT_HackedEffect) > 0
                 ? kSS_Hacked
                 : kSS_Unscanned;
    }
  }

  const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(ResolveScanTarget(mgr, id)));
  if (actor) {
    const CScannableObjectInfo* scan = actor->GetScannableObjectInfo();
    if (scan) {
      if (mgr.PlayerState(mgr.MaskUIdNumPlayers(mPlayerId))
              ->GetScanTime(scan->GetScannableObjectId()) >= 0.9999999f) {
        return static_cast< EScanState >(kSS_Scanned + (scan->IsCritical() ? 1 : 0));
      }
      return static_cast< EScanState >(kSS_Unscanned + (scan->IsCritical() ? 1 : 0));
    }
  }

  return kSS_Invalid;
}

void CPlayerTargeting::PrepareStaticGeometry(const CStateManager& mgr,
                                             const TAreaId& areaId) const {
  const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(mPlayerId));
  if (!player || player->GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::kPV_Scan) {
    return;
  }
  const CStaticGeometryMap* map =
      mgr.GetWorld()->GetAreaAlways(areaId).GetPostConstructed()->mStaticGeometryMap.get();
  if (map) {
    const rstl::vector< CStaticGeometryMapData::TMapping >& mappings = map->GetData().GetMappings();
    TEditorId previousId = kInvalidEditorId;
    int paletteIndex = -1;
    rstl::vector< rstl::pair< int, int > > visible;
    visible.reserve(mappings.size());
    for (rstl::vector< CStaticGeometryMapData::TMapping >::const_iterator it = mappings.begin();
         it != mappings.end(); ++it) {
      int index;
      if (it->second == previousId) {
        index = paletteIndex;
      } else {
        index = GetScanTargetIndex(mgr, mgr.GetIdForScript(it->second));
      }
      previousId = it->second;
      paletteIndex = index;
      if (index > 0) {
        visible.push_back_unsafe(rstl::pair< int, int >(it->first, index));
      }
    }
    gpRender->EnablePVS(areaId.Value(), visible);
  } else {
    gpRender->DisablePVS(areaId.Value());
  }
}

void CPlayerTargeting::Draw(CStateManager& mgr, const CInGameGuiManagerSet& gui) const {
  CColor palette[64] = {
      CColor(0.f, 0.f, 0.f, 0.f),
      CColor::Lerp(skScanPulseStart, skScanPulseEnd,
                   (1.f + CMath::FastCosR(3.f * CGraphics::GetSecondsMod900())) / 2.f)};
  const int count = mScanObjects.size() + 2;
  for (int i = 2; i < count; ++i) {
    palette[i] = GetScanObjectColor(mgr, i - 2);
  }

  const float transition = mgr.GetCurrentRenderPlayerState()->GetVisorTransitionFactor();
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
          palette, count, direction);
}

TUniqueId CPlayerTargeting::ResolveScanTarget(const CStateManager& mgr, TUniqueId id) const {
  if (const CActor* actor =
          static_cast< const CActor* >(TCastToConstPtr< CActor >(mgr.GetObjectById(id)))) {
    if (actor->GetScannableObjectInfo() && actor->GetActive()) {
      return id;
    }
    return actor->CheckConnectedObject_if(mgr, kSS_ScanSource, kSM_None,
                                          CActiveScanPointPredicate());
  }
  return kInvalidUniqueId;
}

TUniqueId CPlayerTargeting::GetScanTargetId(const CStateManager& mgr, int paletteIndex) const {
  const int index = paletteIndex - 2;
  if (index >= 0 && index < mScanObjects.size()) {
    return ResolveScanTarget(mgr, TUniqueId(mScanObjects[index].mId));
  }

  return kInvalidUniqueId;
}
