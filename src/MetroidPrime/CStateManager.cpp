#include "MetroidPrime/CStateManager.hpp"

#include "Collision/CMRay.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Collision/CollisionUtil.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CActorModelParticles.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDecalManager.hpp"
#include "MetroidPrime/CEchoEmitter.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CFluidPlaneManager.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CGameHint.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CHintManager.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CMapWorld.hpp"
#include "MetroidPrime/CMapWorldInfo.hpp"
#include "MetroidPrime/CMemoryCard.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CPortalArea.hpp"
#include "MetroidPrime/CPortalTransition.hpp"
#include "MetroidPrime/CProjectedShadow.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CSafeZoneManager.hpp"
#include "MetroidPrime/CSaveGameScreen.hpp"
#include "MetroidPrime/CScriptMailbox.hpp"
#include "MetroidPrime/CSortedLists.hpp"
#include "MetroidPrime/CStateManagerContainer.hpp"
#include "MetroidPrime/CWeaponMgr.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/CWorldLayerState.hpp"
#include "MetroidPrime/Cameras/CCinematicCamera.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Enemies/CMetroidAlpha.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CSwarmBasics.hpp"
#include "MetroidPrime/GameObjectLists.hpp"
#include "MetroidPrime/HUD/CHUDMemoParms.hpp"
#include "MetroidPrime/HUD/CSamusHud.hpp"
#include "MetroidPrime/Player/CGameMode.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CGameStateEnvVarManager.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Player/CPlayerTargeting.hpp"
#include "MetroidPrime/Player/CWorldState.hpp"
#include "MetroidPrime/Player/CWorldTransManager.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDock.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDoor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDynamicLight.hpp"
#include "MetroidPrime/ScriptObjects/CScriptEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptForgottenObject.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpecialFunction.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpawnPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"

#include "Kyoto/Audio/CAudioGroupSet.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/Basics/RAssertDolphin.hpp"
#include "Kyoto/CARAMManager.hpp"
#include "Kyoto/CARAMToken.hpp"
#include "Kyoto/CDependencyGroup.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/CTimeProvider.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CGraphicsPalette.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/PVS/CPVSVisSet.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"
#include "Kyoto/Particles/CParticleSpawnSystem.hpp"
#include "Kyoto/Particles/CSortedParticleSystem.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetaRender/AmbientLightScale.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CSimpleShadow.hpp"
#include "Weapons/CDecal.hpp"
#include "Weapons/CProjectileWeapon.hpp"
#include "WorldFormat/CPVSAreaSet.hpp"

#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"
#include "rstl/vector.hpp"

#include <alloca.h>
#include <float.h>

const int gkPVSEnabled = 1;

// Prime-correlated role; the selected original stores false.
extern const bool gkWorldOnlyReflection;

// Guessed class/name. Native callers construct stack scopes around named profiling
// regions; the release initializer has no observable state or cleanup.
class CScopedProfiler {
public:
  CScopedProfiler(const rstl::string& name, bool enabled);
};

static s64 sPreRenderStepTime;

// Prime-correlated lazy initialization; Echoes schedules rumble rather than camera shakes.
static float sNextEscapeRumble;
static char sEscapeRumbleInitialized;

// Prime-correlated name; native underwater ranges for the two bomb attributes.
static const float skBombUnderwaterRanges[2] = {2.f, 4.f};

static const char* const skAudioGroupDependencies[3] = {
    "audio_groups_single_player_DGRP", "audio_groups_front_end_DGRP",
    "audio_groups_multi_player_DGRP"};
static const char* const skSinglePlayerAnimController = "SinglePlayerAnimCtrl";
static const char* const skMultiplayerAnimController = "PlayerAnimCtrl";
static const char* const skUnusedViewportTexture = "TXTR_Metroid2LogoSm";

// Guessed name. The original immutable flag is false; its defining TU is unresolved.
static const bool skDisablePlayerTargeting = false;

// Both retained release hooks contain only a return instruction. Their sole known
// callers pass this manager; no exported name or body establishes a semantic name.
extern "C" void fn_8003FF1C(CStateManager*);
extern "C" void fn_8003FF20(CStateManager*);

bool CStateManager::CanCreateProjectile(TUniqueId owner, EWeaponType type, int maxAllowed) const {
  return mWeaponMgr->GetNumActive(owner, type) < maxAllowed;
}

CStateManagerContainer::CStateManagerContainer()
: mCameraManager0(kInvalidUniqueId, CPlayerState::kPI_Player1)
, mCameraManager1(kInvalidUniqueId, CPlayerState::kPI_Player2)
, mCameraManager2(kInvalidUniqueId, CPlayerState::kPI_Player3)
, mCameraManager3(kInvalidUniqueId, CPlayerState::kPI_Player4)
, mRumbleManager0(CPlayerState::kPI_Player1,
                  static_cast< EIOPort >(
                      gpGameState->GetPlayerState(CPlayerState::kPI_Player1)->GetPlayerSelection()))
, mRumbleManager1(CPlayerState::kPI_Player2,
                  static_cast< EIOPort >(
                      gpGameState->GetPlayerState(CPlayerState::kPI_Player2)->GetPlayerSelection()))
, mRumbleManager2(CPlayerState::kPI_Player3,
                  static_cast< EIOPort >(
                      gpGameState->GetPlayerState(CPlayerState::kPI_Player3)->GetPlayerSelection()))
, mRumbleManager3(
      CPlayerState::kPI_Player4,
      static_cast< EIOPort >(
          gpGameState->GetPlayerState(CPlayerState::kPI_Player4)->GetPlayerSelection())) {}

namespace {
// Reconstructed name for the native sentinel outside all valid player indices.
const int kInvalidRenderPlayerIndex = 2000000;

// Guessed local predicate names; priorities and equal-priority intensities descend.
struct CLightPredicate {
  bool operator()(const CLight& a, const CLight& b) const {
    if (a.GetPriority() > b.GetPriority()) {
      return true;
    }
    if (a.GetPriority() == b.GetPriority()) {
      return a.GetIntensity() > b.GetIntensity();
    }
    return false;
  }
};

struct CActorLightPredicate {
  bool operator()(const rstl::pair< TUniqueId, CLight >& a,
                  const rstl::pair< TUniqueId, CLight >& b) const {
    return CLightPredicate()(a.second, b.second);
  }
};

// Guessed local type/member names, correlated with Prime's area-ordering predicate.
class area_sorter {
public:
  area_sorter(const CVector3f& reference, TAreaId visibleAreaId)
  : mReference(reference), mVisibleAreaId(visibleAreaId) {}

  bool operator()(const CGameArea* a, const CGameArea* b) const;

private:
  CVector3f mReference;
  TAreaId mVisibleAreaId;
};
CHECK_SIZEOF(area_sorter, 0x10)

bool area_sorter::operator()(const CGameArea* a, const CGameArea* b) const {
  const TAreaId aId = a->GetId();
  const TAreaId bId = b->GetId();
  if (aId == bId) {
    return false;
  }
  if (aId == mVisibleAreaId) {
    return false;
  }
  if (bId == mVisibleAreaId) {
    return true;
  }

  const float aDot = CVector3f::Dot(mReference, a->GetAABB().GetCenterPoint());
  const float bDot = CVector3f::Dot(mReference, b->GetAABB().GetCenterPoint());
  return aDot > bDot;
}
} // namespace

bool CStateManager::IsActorVisible(const CActor& actor) const {
  if (actor.UsesPortalVisibility()) {
    const CPortalArea* portals =
        mWorld->GetAreaAlways(actor.GetCurrentAreaId()).GetPostConstructed()->mPortalArea.get();
    if (portals != nullptr) {
      return portals->GetVisibleActors().GetObjectById(actor.GetUniqueId()) != nullptr;
    }
  }

  for (rstl::reserved_vector< rstl::pair< int, CFrustumPlanes >, 10 >::const_iterator it =
           mAreaFrusta.begin();
       it != mAreaFrusta.end(); ++it) {
    if (it->first == actor.GetCurrentAreaId().Value()) {
      if (!it->second.BoxInFrustumPlanes(actor.GetOtherBounds())) {
        return false;
      }
      break;
    }
  }

  return mPlanes.BoxInFrustumPlanes(actor.GetOtherBounds());
}

ushort CStateManager::ReturnFirstIfSingleElseSecond(uint single, uint multi) const {
  return IsMultiplayer() ? multi : single;
}

void CStateManager::AddDrawableActor(const CActor& actor, const CVector3f& pos,
                                     const CAABox& bounds) const {
  actor.SetAddedToken(mObjectDrawToken + 1);
  gpRender->AddDrawable(&actor, pos, bounds, 0,
                        actor.UsesAlphaSorting() ? IRenderer::kDS_AlphaSortedCallback
                                                 : IRenderer::kDS_SortedCallback);
}

void CStateManager::AddDrawableActorPlane(const CActor& actor, const CPlane& plane,
                                          const CAABox& bounds) const {
  actor.SetAddedToken(mObjectDrawToken + 1);
  gpRender->AddPlaneObject(&actor, bounds, plane, 0);
}

void CStateManager::CalculatePlayerViewport(int viewportIndex, int* left, int* bottom, int* width,
                                            int* height) const {
  int numPlayers = GetNumPlayers();
  if (mCameraManagers[0]->IsInFullScreenCinematic()) {
    numPlayers = 1;
  }

  const int fullWidth = CGraphics::GetRenderMode().fbWidth;
  const int fullHeight = CGraphics::GetRenderMode().xfbHeight;
  int viewportLeft = 0;
  int viewportBottom = 0;
  int viewportWidth = fullWidth / 2;
  int viewportHeight = fullHeight / 2;

  if (numPlayers == 1) {
    viewportWidth = fullWidth;
    viewportHeight = fullHeight;
    viewportLeft = 0;
    viewportBottom = 0;
  } else if (numPlayers == 2) {
    viewportWidth = fullWidth;
    viewportLeft = 0;
    switch (viewportIndex) {
    case 0:
      viewportBottom = viewportHeight;
      break;
    case 1:
      viewportBottom = 0;
      break;
    }
  } else {
    switch (viewportIndex) {
    case 0:
      viewportLeft = 0;
      viewportBottom = viewportHeight;
      break;
    case 1:
      viewportLeft = viewportWidth;
      viewportBottom = viewportHeight;
      break;
    case 2:
      viewportLeft = 0;
      viewportBottom = 0;
      break;
    case 3:
      viewportLeft = viewportWidth;
      viewportBottom = 0;
      break;
    }
  }

  if (left != nullptr) {
    *left = viewportLeft;
  }
  if (bottom != nullptr) {
    *bottom = viewportBottom;
  }
  if (width != nullptr) {
    *width = viewportWidth;
  }
  if (height != nullptr) {
    *height = viewportHeight;
  }
}

void CStateManager::SetupPlayerViewport(uint playerIndex) {
  mCurrentRenderPlayerIndex = playerIndex;

  int left, bottom, width, height;
  CalculatePlayerViewport(mNumPlayers > 2u ? mPlayerStates[playerIndex]->GetPlayerSelection()
                                           : playerIndex,
                          &left, &bottom, &width, &height);
  CGraphics::SetViewport(left, bottom, width, height);
  CGraphics::SetScissor(left, bottom, width, height);

  const CViewport& viewport = CGraphics::GetViewport();
  const float pixelAspect = CGraphics::GetPixelAspectRatio();
  const float viewportAspect = float(viewport.mWidth) / float(viewport.mHeight);
  mCameraManagers[mCurrentRenderPlayerIndex]->SetAspectRatio(pixelAspect * viewportAspect, *this);

  mCurrentRenderPlayer = mPlayers[playerIndex];
  mPlayerState = mPlayerStates[playerIndex];
  mCameraManager = mCameraManagers[playerIndex];
}

void CStateManager::DrawUnusedViewport(int viewportIndex) {
  int left, bottom, width, height;
  CalculatePlayerViewport(viewportIndex, &left, &bottom, &width, &height);
  CGraphics::SetViewport(left, bottom, width, height);
  CGraphics::SetScissor(left, bottom, width, height);

  const CTexture* texture = mUnusedViewportTexture->GetObject();
  if (texture != nullptr) {
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    gpRender->SetBlendMode_Replace();
    gpRender->SetDepthReadWrite(false, false);
    CGraphics::Render2D(*texture, 0, 0, width, height, CColor::White().WithAlphaOf(1.f));
  }
}

int CStateManager::GetViewportLayoutIndex() const {
  if (mNumPlayers == 1u) {
    return 0;
  }

  int layout = 2;
  if (mNumPlayers == 2u) {
    layout = 1;
  }
  return layout;
}

void CStateManager::AddProjectedShadow(CProjectedShadow* shadow) {
  shadow->SetNextShadow(mProjectedShadows);
  mProjectedShadows = shadow;
}

void CStateManager::BuildDynamicLightListForWorld() {
  if (mRenderVisorMode != kRVM_Normal || mNumPlayers >= 3u) {
    mDynamicLights = rstl::vector< CLight >();
    mDynamicActorLights = rstl::vector< rstl::pair< TUniqueId, CLight > >();
    return;
  }

  const CObjectList& list = GetObjectListById(kOL_GameLight);
  const int listSize = list.size();
  if (listSize == 0) {
    return;
  }

  if (mDynamicLights.capacity() != listSize) {
    mDynamicLights = rstl::vector< CLight >();
    mDynamicLights.reserve(listSize);
    mDynamicActorLights = rstl::vector< rstl::pair< TUniqueId, CLight > >();
    mDynamicActorLights.reserve(listSize);
  } else {
    mDynamicLights.clear();
    mDynamicActorLights.clear();
  }

  for (int idx = list.GetFirstObjectIndex(); idx != -1; idx = list.GetNextObjectIndex(idx)) {
    const CGameLight* light = static_cast< const CGameLight* >(list[idx]);
    if (light && light->GetActive()) {
      const CLight& value = light->GetLight();
      if (value.GetIntensity() > FLT_EPSILON && value.GetRadius() > FLT_EPSILON) {
        if (const CScriptDynamicLight* dynamicLight = TCastToConstPtr< CScriptDynamicLight >(light)) {
          if (dynamicLight->UsesLayerOne() || dynamicLight->UsesLayerTwo()) {
            mDynamicActorLights.push_back_unsafe(
                rstl::pair< TUniqueId, CLight >(dynamicLight->GetUniqueId(), value));
          }
          if (dynamicLight->UsesWorld()) {
            mDynamicLights.push_back_unsafe(value);
          }
        } else {
          mDynamicActorLights.push_back_unsafe(
              rstl::pair< TUniqueId, CLight >(kInvalidUniqueId, value));
          mDynamicLights.push_back_unsafe(value);
        }
      }
    }
  }

  rstl::sort(mDynamicLights.begin(), mDynamicLights.end(), CLightPredicate());
  rstl::sort(mDynamicActorLights.begin(), mDynamicActorLights.end(), CActorLightPredicate());
}

void CStateManager::UpdateObjectInLists(CEntity& entity) {
  for (rstl::reserved_vector< CObjectList*, 8 >::iterator it = mDynamicObjectLists.begin();
       it != mDynamicObjectLists.end(); ++it) {
    const bool contained =
        static_cast< const CObjectList* >(*it)->GetObjectById(entity.GetUniqueId()) != nullptr;
    if (contained && !(*it)->IsQualified(entity)) {
      (*it)->RemoveObject(entity.GetUniqueId());
    } else if (!contained) {
      (*it)->AddObject(entity);
    }
  }

  for (rstl::reserved_vector< CFilteredObjectList*, 6 >::iterator it =
           mDynamicFilteredObjectLists.begin();
       it != mDynamicFilteredObjectLists.end(); ++it) {
    CFilteredObjectList* list = *it;
    const bool contained = list->Contains(entity);
    if (contained && !list->IsQualified(entity)) {
      list->RemoveObject(entity);
    } else if (!contained) {
      list->AddObject(entity);
    }
  }
}

CRayCastResult CStateManager::RayWorldIntersection(
    TUniqueId& idOut, const CVector3f& position, const CVector3f& direction, float length,
    const CMaterialFilter& filter, const rstl::reserved_vector< TUniqueId, 1024 >& nearList) const {
  return CGameCollision::RayWorldIntersection(*this, idOut, position, direction, length, filter,
                                              nearList);
}

CRayCastResult CStateManager::RayStaticIntersection(const CVector3f& position,
                                                    const CVector3f& direction, float length,
                                                    const CMaterialFilter& filter) const {
  return CGameCollision::RayStaticIntersection(*this, position, direction, length, filter);
}

bool CStateManager::RayCollideWorld(const CVector3f& start, const CVector3f& end,
                                    const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                    const CMaterialFilter& filter,
                                    const CActor* ignoreActor) const {
  return RayCollideWorldInternal(start, end, filter, nearList, ignoreActor);
}

bool CStateManager::RayCollideWorld(const CVector3f& start, const CVector3f& end,
                                    const CMaterialFilter& filter, const CActor* ignoreActor) {
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  CVector3f direction = end - start;
  const float length = direction.Magnitude();
  direction *= 1.f / length;

  BuildNearList(nearList, start, direction, length, filter, ignoreActor);
  return RayCollideWorldInternal(start, end, filter, nearList, ignoreActor);
}

const bool CStateManager::RayCollideWorldInternal(
    const CVector3f& start, const CVector3f& end, const CMaterialFilter& filter,
    const rstl::reserved_vector< TUniqueId, 1024 >& nearList, const CActor* ignoreActor) const {
  CVector3f direction = end - start;
  bool visible = true;
  if (direction.CanBeNormalized()) {
    const float length = direction.Magnitude();
    direction *= 1.f / length;
    visible = CGameCollision::RayStaticLineOfSightTest(*this, start, direction, length, filter);
    if (visible) {
      visible = CGameCollision::RayDynamicLineOfSightTest(*this, start, direction, length, filter,
                                                          nearList, ignoreActor);
    }
  }
  return visible;
}

void CStateManager::AddObject(CEntity* entity) {
  if (entity != nullptr) {
    AddObject(*entity);
  }
}

void CStateManager::BuildNearList(rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                  const CAABox& bounds, const CMaterialFilter& filter,
                                  const CActor* ignoreActor) const {
  mSortedListManager->BuildNearList(nearList, bounds, filter, ignoreActor);
}

void CStateManager::BuildColliderList(rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                      const CActor& actor, const CAABox& bounds) const {
  mSortedListManager->BuildNearList(nearList, actor, bounds);
}

void CStateManager::BuildNearList(rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                  const CVector3f& position, const CVector3f& direction,
                                  float length, const CMaterialFilter& filter,
                                  const CActor* ignoreActor) const {
  mSortedListManager->BuildNearList(nearList, position, direction, length, filter, ignoreActor);
}

void CStateManager::AreaLoaded(TAreaId area) {
  mMailbox->SendMsgs(area, *this);
  mEnvFxManager->AreaLoaded();
}

void CStateManager::PrepareAreaUnload(TAreaId area) {
  const rstl::list< CEntity* >& doors = GetDoorList();
  for (rstl::list< CEntity* >::const_iterator it = doors.begin(); it != doors.end(); ++it) {
    CScriptDoor* door = static_cast< CScriptDoor* >(*it);
    if (door->IsConnectedToArea(*this, area)) {
      door->ForceClosed(*this);
    }
  }

  ScriptObjectLoaderHelper().FreeScriptObjects(area, *this);
}

void CStateManager::AreaUnloaded(TAreaId area) {}

CEntity* CStateManager::ObjectById(TUniqueId uid) {
  return mObjectLists[kOL_All]->GetObjectById(uid);
}

void CStateManager::DeleteObjectRequest(TUniqueId uid) {
  SendScriptMsg(uid, kInvalidUniqueId, kSM_Delete, kInvalidUniqueId);
}

void CStateManager::SetCurrentAreaId(TAreaId area) {
  if (mNextAreaId != area) {
    mPreviousAreaId = mNextAreaId;
    mNextAreaId = area;
  }

  const TAreaId& currentArea = area;
  if (currentArea != kInvalidAreaId) {
    if (!mMapWorldInfo->IsAreaVisited(currentArea)) {
      mMapWorldInfo->SetAreaVisited(currentArea, true);
      CMapWorldInfo* mapInfo = mMapWorldInfo.GetPtr();
      CWorld* world = mWorld.get();
      CMapWorld* mapWorld = world->GetMapWorld();
      mapWorld->RecalculateWorldSphere(*mapInfo, *world);
    }
  }
}

void CStateManager::FrameEnd() {
  CModel::FrameDone();
  gpSimplePool->Flush();
}

bool CStateManager::GetVisSetForArea(const TAreaId area, const TAreaId visibleArea,
                                     CPVSVisSet& visibility) const {
  if (visibleArea == kInvalidAreaId) {
    return false;
  }

  CVector3f closestDockPoint = CGraphics::GetViewMatrix().GetTranslation();
  const CVector3f viewPoint = closestDockPoint;
  bool hasClosestDock = false;
  if (area == visibleArea) {
    hasClosestDock = true;
  } else {
    const CGameArea* visArea = mWorld->GetArea(visibleArea);
    if (visArea->IsLoaded()) {
      const int dockCount = visArea->GetDockCount();
      for (int i = 0; i < dockCount; ++i) {
        const IGameArea::Dock& dock = visArea->GetDock(i);
        const int connectionCount = dock.GetDockRefs().size();
        for (int connection = 0; connection < connectionCount; ++connection) {
          if (dock.GetConnectedAreaId(connection) != area) {
            continue;
          }

          const rstl::reserved_vector< CVector3f, 4 >& vertices = dock.GetPlaneVertices();
          const CVector3f center = 0.25f * (vertices[0] + vertices[1] + vertices[2] + vertices[3]);
          if (hasClosestDock &&
              !((center - viewPoint).MagSquared() < (closestDockPoint - viewPoint).MagSquared())) {
            continue;
          }

          closestDockPoint = center;
          hasClosestDock = true;
        }
      }
    }
  }

  int setState = 0;
  if (hasClosestDock) {
    setState = 1;
    const CGameArea* targetArea = mWorld->GetArea(area);
    const CPVSAreaSet* areaSet = targetArea->GetPostConstructed()->mPvs.get();
    if (areaSet != nullptr) {
      setState = 2;
      CPVSVisOctree& octree = areaSet->GetVisOctree();
      const CTransform4f& inverseTransform =
          mWorld->GetArea(area)->GetPostConstructed()->mInverseTransform;
      const CVector3f localPoint = inverseTransform * closestDockPoint;
      CPVSVisSet set = octree.GetVisSet(localPoint);
      if (set.GetState() == kVSS_NodeFound) {
        setState = 3;
        visibility = set;
      }
    }
  }

  return setState == 3;
}

void CStateManager::Touch() {
  TouchSky();
  TouchPlayerActor();

  for (uint i = 0; i < mNumPlayers; ++i) {
    SetupPlayerViewport(i);
    const CPlayer* const player = mPlayers[i];
    bool touchModel = false;
    bool touchBall = false;
    bool touchGun = false;
    switch (player->GetMorphballTransitionState()) {
    case CPlayer::kMS_Unmorphed:
      touchGun = true;
      break;
    case CPlayer::kMS_Morphed:
      touchBall = true;
      break;
    case CPlayer::kMS_Morphing:
      touchBall = true;
      touchModel = true;
      break;
    case CPlayer::kMS_Unmorphing:
      touchGun = true;
      touchModel = true;
      break;
    default:
      break;
    }

    if (touchGun) {
      player->GetPlayerGun()->TouchModel(*this);
    }
    if (touchModel) {
      player->GetModelData()->Touch(*this, 0);
    }
    if (touchBall) {
      player->GetMorphBall()->TouchModel(*this);
    }
  }

  EndPlayerRender();
}

void CStateManager::PreRender(uint playerIndex) {
  CTimeProvider timeProvider(mCurTimeMod900);
  SetupPlayerViewport(playerIndex);
  if (!mReadyToRender) {
    return;
  }

  CStopwatch timer;
  switch (mPlayerState->GetActiveVisor(*this)) {
  case CPlayerState::kPV_Combat:
  case CPlayerState::kPV_Scan:
    mRenderVisorMode = kRVM_Normal;
    break;
  case CPlayerState::kPV_Echo:
    mRenderVisorMode = kRVM_Echo;
    break;
  case CPlayerState::kPV_Dark:
    mRenderVisorMode = kRVM_Dark;
    break;
  default:
    break;
  }

  mStateManagerContainer->mRenderBeforeAreas.clear();
  mStateManagerContainer->mRenderFirstSorted.clear();
  mStateManagerContainer->mRenderLast.clear();
  mStateManagerContainer->mRenderLastUnderGun.clear();
  mStateManagerContainer->mRenderLastAfterCameraFilters.clear();
  mProjectedShadows = nullptr;

  mWorld->PreRender();
  BuildDynamicLightListForWorld();
  const CGameCamera* camera = mCameraManager->GetCurrentCamera(*this, true);
  const CTransform4f cameraTransform = mCameraManager->GetCurrentCameraTransform(*this, true);
  CFrustumPlanes frustum(cameraTransform, CRelAngle::FromDegrees(camera->GetFov()).AsRadians(),
                         camera->GetAspectRatio(), camera->GetNearClipDistance(), false, 100.f);
  mPlanes = frustum;
  SetupAreaFrusta();

  for (CGameArea::CChainIterator it = GetWorld()->ChainHead(CWorld::kC_Alive);
       it != CWorld::AliveAreasEnd(); ++it) {
    CGameArea& area = *it;
    if (area.GetOcclusionState() != CGameArea::kOS_Visible) {
      continue;
    }
    CObjectList* const objects = area.GetPostConstructed()->mAreaObjectList.get();
    CObjectList* const visibleActors = area.GetPostConstructed()->mVisibleActorList.get();
    if (playerIndex == 0u) {
      visibleActors->Clear();
      CScopedProfiler profile(rstl::string_l("*PreRender:PreRenderAllViewports"), true);
      for (int i = objects->GetFirstObjectIndex(); i != -1; i = objects->GetNextObjectIndex(i)) {
        CActor* actor = TCastToPtr< CActor >((*objects)[i]);
        if (actor != nullptr && actor->GetActive() && actor->GetDrawEnabled()) {
          visibleActors->AddObject(*actor);
          actor->PreRenderAllViewports(*this);
        }
      }
    }

    {
      CScopedProfiler profile(rstl::string_l("*PreRender:Portals"), true);
      if (CPortalArea* portal = area.GetPostConstructed()->mPortalArea.get()) {
        const CCameraManager* cameraManager = mCameraManager;
        portal->PreRender(*this, *cameraManager->GetCurrentCamera(*this, true),
                          cameraManager->GetCurrentCameraTransform(*this, true));
      }
    }
    {
      CScopedProfiler profile(rstl::string_l("*PreRender:Actors"), true);
      for (int i = visibleActors->GetFirstObjectIndex(); i != -1;
           i = visibleActors->GetNextObjectIndex(i)) {
        static_cast< CActor* >((*visibleActors)[i])->PreRender(*this);
      }
    }
  }

  if (!gkWorldOnlyReflection) {
    CacheReflection();
  }
  mCameraManagers[playerIndex]->UpdateFogState(*this);
  sPreRenderStepTime = timer.GetElapsedMicros();
}

bool CStateManager::SetupFogForDraw() const {
  switch (mPlayerState->GetActiveVisor(*this)) {
  case CPlayerState::kPV_Echo: {
    const CTweakGui* tweak = gpTweakGui.get();
    gpRender->SetWorldFog(tweak->GetEchoFogMode(), tweak->GetEchoFogNearZ(),
                          tweak->GetEchoFogFarZ(), CColor::White());
    return true;
  }
  default:
    return false;
  case CPlayerState::kPV_Combat:
  case CPlayerState::kPV_Scan:
  case CPlayerState::kPV_Dark: {
    const CGameArea::CAreaFog* fog = &mCameraManager->GetFog();
    if (fog->IsFogDisabled()) {
      return false;
    }
    fog->SetCurrent();
    return true;
  }
  }
}

void CStateManager::SetupFogForArea(const CGameArea& area) const {
  if (!SetupFogForDraw()) {
    area.GetAreaFog()->SetCurrent();
  }
}

void CStateManager::SetParticleAlphaUpdate(bool disable) {
  CDecal::SetDisableAlphaUpdate(disable);
  CElementGen::sMoveRedToAlphaBuffer = disable;
  CProjectileWeapon::SetDisableAlphaUpdates(disable);
}

void CStateManager::SetAreaClipPlane(TAreaId area, const CPlane& plane) {
  rstl::reserved_vector< rstl::pair< int, CFrustumPlanes >, 10 >::iterator it = mAreaFrusta.begin();
  for (; it != mAreaFrusta.end(); ++it) {
    if (it->first == area.Value()) {
      CFrustumPlanes& frustum = it->second;
      if (frustum.GetPlanes().size() < 6u) {
        frustum.AddPlane(plane);
      }
      break;
    }
  }

  if (it == mAreaFrusta.end()) {
    CFrustumPlanes frustum = CFrustumPlanes();
    frustum.AddPlane(plane);
    mAreaFrusta.push_back(rstl::pair< int, CFrustumPlanes >(area.Value(), frustum));
  }
}

void CStateManager::SetupAreaFrusta() {
  mVisAreaId = GetVisAreaId();
  mAreaFrusta.clear();
  const CTransform4f cameraTransform = mCameraManager->GetCurrentCameraTransform(*this, true);

  for (CGameArea::CConstChainIterator it = GetWorld()->GetChainHead(CWorld::kC_Alive);
       it != CWorld::GetAliveAreasEnd(); ++it) {
    const CGameArea& area = *it;
    if (area.GetOcclusionState() != CGameArea::kOS_Visible || area.GetId() == mVisAreaId) {
      continue;
    }

    bool foundDock = false;
    const IGameArea::Dock* selectedDock = nullptr;
    for (int i = 0; i < area.GetDockCount(); ++i) {
      const IGameArea::Dock& dock = area.GetDock(i);
      if (dock.GetConnectedAreaId(dock.GetReferenceCount()) == mVisAreaId) {
        if (!foundDock) {
          selectedDock = &dock;
          foundDock = true;
        } else {
          foundDock = false;
          break;
        }
      }
    }
    if (!foundDock) {
      continue;
    }

    const CVector3f position = cameraTransform.GetTranslation();
    const CVector3f* vertices = selectedDock->GetPlaneVertices().data();
    const CVector3f normal = CVector3f::Cross(vertices[1] - vertices[0], vertices[2] - vertices[0]);
    const CVector3f delta = position - vertices[0];
    if (CVector3f::Dot(delta, normal) > 0.f) {
      CFrustumPlanes frustum;
      for (int i = 0; i < 4; ++i) {
        frustum.AddPlane(CPlane(position, vertices[i], vertices[(i + 1) % 4]));
      }
      mAreaFrusta.push_back(rstl::pair< int, CFrustumPlanes >(area.GetId().Value(), frustum));
    }
  }
}

CGameArea::CConstChainIterator CWorld::GetAliveAreasEnd() { return skGlobalEnd; }

void CStateManager::GatherVisibleAreas(TVisibleAreas& areas, TAreaVisibility& visibility) {
  for (CGameArea::CConstChainIterator it = mWorld->GetChainHead(CWorld::kC_Alive);
       it != CWorld::GetAliveAreasEnd() && areas.size() != areas.capacity(); ++it) {
    if (it->GetOcclusionState() == CGameArea::kOS_Visible) {
      areas.push_back(&*it);
    }
  }

  rstl::sort(areas.begin(), areas.end(),
             area_sorter(CGraphics::GetViewMatrix().GetForward(), mVisAreaId));

  for (TVisibleAreas::iterator it = areas.begin(); it != areas.end(); ++it) {
    CPVSVisSet set(kVSS_OutOfBounds);
    GetVisSetForArea((*it)->GetId(), mVisAreaId, set);
    visibility.push_back(set);
  }
}

void CStateManager::PrepareWorldRendering(const TVisibleAreas& areas,
                                          const TAreaVisibility& visibility) {
  rstl::reserved_vector< rstl::pair< int, const CPVSVisSet* >, 5 > pvsSets;
  for (int i = 0; i < areas.size(); ++i) {
    pvsSets.push_back(
        rstl::pair< int, const CPVSVisSet* >(areas[i]->GetId().Value(), &visibility[i]));
  }

  const rstl::reserved_vector< CSafeZoneManager::SZone, 64 >& zones = mSafeZoneManager->GetZones();
  rstl::reserved_vector< rstl::pair< int, float >, 64 > ambientLights;
  const int count = rstl::min_val(zones.capacity(), zones.size());
  for (int i = 0; i < count; ++i) {
    const TEditorId editorId = GetEditorIdForUniqueId(zones[i].mId);
    if (editorId != kInvalidEditorId) {
      ambientLights.push_back(MakeAmbientLightScale(editorId.value, zones[i].mScaleFactor));
    }
  }

  gpRender->PrepareWorldRendering(
      pvsSets.data(), pvsSets.size(), mPlanes, &mAreaFrusta, mDynamicLights,
      !ambientLights.empty() ? ambientLights.data() : nullptr, ambientLights.size());
}

void CStateManager::GetWorldGeometryMasks(uint& mask, uint& targetMask,
                                          CPlayerState::EPlayerVisor visor) {
  int bit = 1;
  if (visor == CPlayerState::kPV_Dark) {
    bit = 2;
  }
  mask = 1u << bit;
  targetMask = 0;
}

void CStateManager::RenderActorQueue(rstl::reserved_vector< TUniqueId, 20 >& queue,
                                     const rstl::string& profileName) {
  if (!queue.empty()) {
    CScopedProfiler profile(profileName, true);
    for (rstl::reserved_vector< TUniqueId, 20 >::const_iterator it = queue.begin();
         it != queue.end(); ++it) {
      static_cast< const CActor* >(GetObjectById(*it))->Render(*this);
    }
  }
}

void CStateManager::DrawUnsortedGeometry(const TVisibleAreas& areas, uint mask, uint targetMask,
                                         CPlayerState::EPlayerVisor visor) {
  CScopedProfiler profile(rstl::string_l("*UnsortedStaticGeom"), true);
  const CPlayer* const player = mCurrentRenderPlayer;
  const CPlayerTargeting* const targeting = player->GetTargeting();

  for (int i = areas.size() - 1; i >= 0; --i) {
    const CGameArea& area = *areas[i];
    const TAreaId id = area.GetId();
    SetupFogForArea(area);
    gpRender->SetWorldLightFadeLevel(area.GetPostConstructed()->mWorldLightingLevel);
    targeting->PrepareStaticGeometry(*this, id);

    switch (visor) {
    case CPlayerState::kPV_Echo:
      gpRender->DrawEchoVisorGeometry(
          player->GetEchoPulsePhase(), gpTweakGui->GetEchoBigRingScale(),
          gpTweakGui->GetEchoBigRingFadeStart(), gpTweakGui->GetEchoAuraSmallSize(),
          gpTweakGui->GetEchoAuraBigSize(), id.Value());
      break;
    case CPlayerState::kPV_Scan:
      gpRender->DrawUnsortedGeometryAlpha(id.Value());
      break;
    default:
      gpRender->DrawUnsortedGeometry(id.Value());
      break;
    }
  }
}

void CStateManager::DrawSky(const TVisibleAreas& areas, CPlayerState::EPlayerVisor visor) {
  mWorld->TouchSky();
  if (visor == CPlayerState::kPV_Echo) {
    return;
  }

  bool fogEnabled = false;
  CScopedProfiler profile(rstl::string_l("*Sky"), true);
  if (visor == CPlayerState::kPV_Scan) {
    gpRender->SetDestinationAlpha(0);
  }
  if (mCameraManager->IsFogEnabled()) {
    gpRender->SetWorldFog(kRFM_PerspLin, 0.f, 0.05f, mCameraManager->GetFog().GetColor());
    fogEnabled = true;
  } else {
    gpRender->SetWorldFog(kRFM_None, 0.f, 1.f, CColor::Black());
  }
  mWorld->DrawSky(CTransform4f::Translate(CGraphics::GetViewMatrix().GetTranslation()), fogEnabled);
  if (!areas.empty()) {
    SetupFogForArea(*areas[areas.size() - 1]);
  }
}

void CStateManager::RenderAreaActors(bool& deferPlayerRender, CEchoEmitter*& emitters,
                                     const CGameArea& area, const CPVSVisSet& visibility) {
  CScopedProfiler profile(rstl::string_l("*AllActorRender"), true);
  const CObjectList* const actors = area.GetPostConstructed()->mVisibleActorList.get();
  CPlayer* const player = mCurrentRenderPlayer;
  for (int index = actors->GetFirstObjectIndex(); index != -1;
       index = actors->GetNextObjectIndex(index)) {
    const CActor* actor = static_cast< const CActor* >((*actors)[index]);
    if (actor->GetPvsIndex() != -1 &&
        visibility.GetVisible(actor->GetPvsIndex()) == kVSS_EndOfTree) {
      continue;
    }
    if (actor == player) {
      if (player->GetMorphballTransitionState() == CPlayer::kMS_Morphed ||
          player->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
        CScopedProfiler playerProfile(rstl::string_l("*1stPlayerRender"), true);
        player->AddToRenderer(*this);
      } else {
        deferPlayerRender = true;
      }
    } else {
      actor->AddToRenderer(*this);
      if (CEchoEmitter* emitter = actor->EchoEmitter()) {
        emitter->SetNextEmitter(emitters);
        emitters = emitter;
      }
    }
  }
}

void CStateManager::DrawSpecialGeometry(TAreaId area, CPlayerState::EPlayerVisor visor, uint,
                                        uint) {
  switch (visor) {
  case CPlayerState::kPV_Echo:
    break;
  case CPlayerState::kPV_Scan:
    gpRender->DrawSpecialGeometryAlpha(area.Value());
    break;
  default:
    gpRender->DrawSpecialGeometry(area.Value());
    break;
  }
}

void CStateManager::DrawDarkWorldEffects(CPlayerState::EPlayerVisor visor) {
  if (mIsDarkWorld) {
    switch (visor) {
    case CPlayerState::kPV_Dark:
      mSafeZoneManager->Render(*this);
      break;
    case CPlayerState::kPV_Combat:
    case CPlayerState::kPV_Scan:
      mSafeZoneManager->Render(*this);
      gpRender->DrawDarkWorldFilter(mSafeZoneManager->GetDarkWorldFilterAmount(
          mCameraManager->GetCurrentCamera(*this, true)->GetTransform()));
      break;
    default:
      break;
    }
  }
}

void CStateManager::DrawDarkWorldCloud(CPlayerState::EPlayerVisor visor) {
  if (mDarkWorldCloudTime > 0.f) {
    switch (visor) {
    case CPlayerState::kPV_Combat:
    case CPlayerState::kPV_Scan:
      gpRender->DrawDarkWorldCloud(mDarkWorldCloudTime, mDarkWorldCloudScale, mDarkWorldCloudColor);
      break;
    default:
      break;
    }
  }
}

void CStateManager::RenderEchoEmitters(const CEchoEmitter* emitters) const {
  CEchoEmitter::RenderEmitters(*this, emitters);
}

void CStateManager::SetupViewForDraw(const CViewport& viewport) {
  const CPlayer* player = mCurrentRenderPlayer;
  const CGameCamera* camera = mCameraManager->GetCurrentCamera(*this, true);
  const CTransform4f cameraTransform = mCameraManager->GetCurrentCameraTransform(*this, true);
  gpRender->SetWorldViewpoint(cameraTransform);
  CCubeModel::SetNewPlayerPositionAndTime(player->GetTranslation(), CStopwatch::GetGlobalTimerObj());

  float widthScale = player->GetViewportScaleX();
  float heightScale = player->GetViewportScaleY();
  for (int i = 0; i < 11; ++i) {
    const CCameraFilterPass& filter = mCameraFilterPasses[mCurrentRenderPlayerIndex][i];
    const float filterWidth = filter.GetWidthScale();
    if (filterWidth < widthScale) {
      widthScale = filterWidth;
    }
    const float filterHeight = filter.GetHeightScale();
    if (filterHeight < heightScale) {
      heightScale = filterHeight;
    }
  }

  const float scaledWidth = widthScale * static_cast< float >(viewport.mWidth);
  const float scaledHeight = heightScale * static_cast< float >(viewport.mHeight);
  const int width = static_cast< int >(scaledWidth);
  const int height = (static_cast< int >(scaledHeight) / 2) * 2;
  const int left = viewport.mLeft + (viewport.mWidth - width) / 2;
  const bool splitScreen = IsMultiplayer() && !mCameraManagers[0]->IsInFullScreenCinematic();
  const float topScale = splitScreen ? 0.25f : 0.5f;
  const int top = viewport.mTop + static_cast< int >(topScale * (viewport.mHeight - height));
  const float aspect = (widthScale * camera->GetAspectRatio()) / heightScale;
  const float tangent = static_cast< float >(tan(CMath::Deg2Rad(0.5f * camera->GetFov())));
  const float fieldOfView = 2.f * static_cast< float >(atan(tangent * heightScale));

  gpRender->SetViewport(left, top, width, height);
  CGraphics::SetDepthRange(0.125f, 1.f);
  gpRender->SetPerspective(360.f * CMath::Rad2Rev(fieldOfView), scaledWidth, scaledHeight,
                           camera->GetNearClipDistance(), camera->GetFarClipDistance());
  mPlanes = CFrustumPlanes(cameraTransform, fieldOfView, aspect,
                           camera->GetNearClipDistance(), false, 100.f);
  gpRender->PrimColor(CColor::White());
  gpRender->SetModelMatrix(CTransform4f::Identity());
  mFluidPlaneManager->StartFrame(false);
  gpRender->SetDebugOption(IRenderer::kDO_PVSState, 1);
}

void CStateManager::DrawWorld(const CInGameGuiManagerSet& gui) {
  CScopedProfiler profile(rstl::string_l("*TotalDrawWorld"), true);
  SetRendererWorkspace(alloca(GetRendererWorkspaceSize()));

  const CPlayerState::EPlayerVisor visor = mPlayerState->GetActiveVisor(*this);
  CTimeProvider timeProvider(mCurTimeMod900);
  CViewport viewport = CGraphics::GetViewport();
  viewport.mTop = CGraphics::GetViewportTop(viewport.mTop);
  SetupViewForDraw(viewport);
  const CTransform4f viewMatrix(CGraphics::GetViewMatrix());

  TVisibleAreas areas;
  TAreaVisibility visibility;
  GatherVisibleAreas(areas, visibility);
  CPlayer* const player = mCurrentRenderPlayer;
  PrepareWorldRendering(areas, visibility);
  uint mask = 0;
  uint targetMask = 0;
  GetWorldGeometryMasks(mask, targetMask, visor);
  gpRender->SetRequestedMaterialMode(visor == CPlayerState::kPV_Dark ? 1 : 0);
  SetupParticleDrawMask();
  SetParticleAlphaUpdate(visor == CPlayerState::kPV_Echo);

  RenderActorQueue(mStateManagerContainer->mRenderBeforeAreas,
                   rstl::string_l("*RenderBeforeAreas"));
  DrawUnsortedGeometry(areas, mask, targetMask, visor);
  DrawSky(areas, visor);
  RenderActorQueue(mStateManagerContainer->mRenderFirstSorted,
                   rstl::string_l("*renderFirstSorted"));

  bool deferPlayerRender = false;
  CEchoEmitter* emitters = nullptr;
  for (int i = 0; i < areas.size(); ++i) {
    const CGameArea& area = *areas[i];
    SetupFogForArea(area);
    gpRender->SetWorldLightFadeLevel(area.GetPostConstructed()->mWorldLightingLevel);
    RenderAreaActors(deferPlayerRender, emitters, area, visibility[i]);
    DrawSpecialGeometry(area.GetId(), visor, mask, targetMask);
    ++mObjectDrawToken;

    if (area.GetId() == mVisAreaId) {
      CScopedProfiler decalProfile(rstl::string_l("*Decal+ActorParticleStragglers"), true);
      CDecalManager::AddToRenderer(*this);
      mActorModelParticles->AddStragglersToRenderer(*this);
      if (mProjectedShadows) {
        CScopedProfiler shadowProfile(rstl::string_l("*Projected"), true);
        for (const CProjectedShadow* shadow = mProjectedShadows; shadow;
             shadow = shadow->GetNextShadow()) {
          shadow->Render(*this);
        }
      }
    }
    if (area.GetId() == mCurrentRenderPlayer->GetCurrentAreaId()) {
      CScopedProfiler shadowProfile(rstl::string_l("*IDBasedShadows"), true);
      player->GetMorphBall()->DrawBallShadow(*this);
    }

    CScopedProfiler sortedProfile(rstl::string_l("*AllSortedGeometry"), true);
    const int mode = visor == CPlayerState::kPV_Scan ? 1 : 0;
    if (visor != CPlayerState::kPV_Echo) {
      gpRender->DrawSortedGeometry(mode, area.GetId().Value());
    } else {
      gpRender->DrawSortedGeometry(mode, -2);
    }
  }

  {
    CScopedProfiler envProfile(rstl::string_l("*EnvFxManager"), true);
    mEnvFxManager->Render(*this);
  }
  if (deferPlayerRender) {
    CScopedProfiler playerProfile(rstl::string_l("*2ndPlayerRender"), true);
    player->Render(*this);
  }
  if (visor == CPlayerState::kPV_Scan) {
    CScopedProfiler visorProfile(rstl::string_l("*GameVisor"), true);
    CapturePlayerTextures();
    mCurrentRenderPlayer->GetTargeting()->Draw(*this, gui);
  }
  {
    CScopedProfiler fogProfile(rstl::string_l("*PostRenderFogs"), true);
    gpRender->PostRenderFogs();
  }
  DrawDarkWorldEffects(visor);
  {
    CScopedProfiler fluidProfile(rstl::string_l("*FluidPlaneManagerEndFrame"), true);
    mFluidPlaneManager->EndFrame();
    gpRender->SetWorldFog(kRFM_None, 0.f, 1.f, CColor::Black());
  }
  {
    CScopedProfiler gunProfile(rstl::string_l("*PlayerGun"), true);
    player->RenderGun(*this, mCameraManager->GetGlobalCameraTranslation(*this, true));
  }
  RenderActorQueue(mStateManagerContainer->mRenderLastUnderGun,
                   rstl::string_l("*RenderLastUnderGun"));
  DrawDarkWorldCloud(visor);
  if (!mStateManagerContainer->mRenderLast.empty()) {
    CGraphics::SetDepthRange(4.f / 256.f, 8.f / 256.f);
    RenderActorQueue(mStateManagerContainer->mRenderLast, rstl::string_l("*RenderLast"));
    CGraphics::SetDepthRange(0.125f, 1.f);
  }
  if (gkWorldOnlyReflection) {
    CScopedProfiler reflectionProfile(rstl::string_l("*PlayerReflection"), true);
    CacheReflection();
  }

  SetParticleAlphaUpdate(false);
  CParticleGen::sDrawFlags = 0;
  CParticleGen::sDrawMask = 0;
  gpRender->SetRequestedMaterialMode(0);
  switch (visor) {
  case CPlayerState::kPV_Dark:
    RenderForgottenObjects();
    DrawDarkVisor(gui);
    break;
  case CPlayerState::kPV_Echo:
    gpRender->DrawScreenFilter(gpTweakGui->GetEchoBaseColor(), gpTweakGui->GetEchoOutlineColor(),
                               gpTweakGui->GetEchoRingColor());
    RenderEchoEmitters(emitters);
    break;
  default:
    break;
  }

  ResetViewAfterDraw(viewport, viewMatrix);
  {
    CScopedProfiler filterProfile(rstl::string_l("*DrawAdditionalFilters"), true);
    DrawAdditionalFilters();
  }
  RenderActorQueue(mStateManagerContainer->mRenderLastAfterCameraFilters,
                   rstl::string_l("*RenderLastAfterCameraFilters"));
  ReleaseRendererWorkspace();
}

void CStateManager::ResetViewAfterDraw(const CViewport& viewport, const CTransform4f& viewMatrix) {
  gpRender->SetViewport(viewport.mLeft, viewport.mTop, viewport.mWidth, viewport.mHeight);
  const CGameCamera* camera = mCameraManager->GetCurrentCamera(*this, true);
  CFrustumPlanes frustum(viewMatrix, CRelAngle::FromDegrees(camera->GetFov()).AsRadians(),
                         camera->GetAspectRatio(), camera->GetNearClipDistance(), false, 100.f);
  mPlanes = frustum;

  const float height = CGraphics::GetViewport().mHeight;
  const float width = CGraphics::GetViewport().mWidth;
  gpRender->SetPerspective(camera->GetFov(), width, height, camera->GetNearClipDistance(),
                           camera->GetFarClipDistance());
}

void CStateManager::DrawAdditionalFilters() {
  for (int i = 0; i < 11; ++i) {
    mCameraBlurPasses[mCurrentRenderPlayerIndex][i].Draw();
    mCameraFilterPasses[mCurrentRenderPlayerIndex][i].Draw();
  }

  if (gpGameState->GetEscapeTime() < 1.f && gpGameState->GetEscapeTime() > 0.f &&
      !mCameraManager->IsInCinematicCamera()) {
    const float escapeTime = gpGameState->GetEscapeTime();
    const CColor color = CColor::White().WithAlphaOf(1.f - escapeTime);
    CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Add, CCameraFilterPass::kFS_Fullscreen,
                                  color, nullptr, 1.f);
  }
}

void CStateManager::ReflectionDrawer(void* context, const CVector3f& point) {
  CStateManager* manager = static_cast< CStateManager* >(context);
  manager->DrawReflection(point);
}

void CStateManager::CacheReflection() {
  if (mRenderVisorMode == kRVM_Normal && !IsMultiplayer()) {
    gpRender->CacheReflection(ReflectionDrawer, this, !gkWorldOnlyReflection);
  }
}

void CStateManager::DrawReflection(const CVector3f& point) {
  CPlayer* player = mPlayers[mCurrentRenderPlayerIndex];
  CAABox playerBounds = player->GetBoundingBox();
  CVector3f playerPosition = playerBounds.GetCenterPoint();
  const CVector3f viewPosition =
      playerPosition - 3.5f * CVector3f(playerPosition.GetX() - point.GetX(),
                                        playerPosition.GetY() - point.GetY(),
                                        playerPosition.GetZ() - playerPosition.GetZ())
                                  .AsNormalized();
  CTransform4f reflectionTransform =
      CTransform4f::LookAt(viewPosition, playerPosition, CVector3f(0.f, 0.f, -1.f));
  const CTransform4f backupView = CGraphics::GetViewMatrix();
  CGraphics::SetViewPointMatrix(reflectionTransform);

  const CGameCamera& camera = *mCameraManager->GetCurrentCamera(*this, true);
  const CViewport& viewport = CGraphics::GetViewport();
  const float height = static_cast< float >(viewport.mHeight);
  const float width = static_cast< float >(viewport.mWidth);
  const CGraphics::CProjectionState backupProjection = CGraphics::GetProjectionState();
  gpRender->SetPerspective(camera.GetFov(), width, height, camera.GetNearClipDistance(),
                           camera.GetFarClipDistance());
  player->RenderReflectedPlayer(*this);

  CGraphics::SetViewPointMatrix(backupView);
  CGraphics::SetProjectionState(backupProjection);
}

void CStateManager::DrawSpaceWarp(const CVector3f& position, float strength) const {
  switch (mPlayerState->GetActiveVisor(*this)) {
  case CPlayerState::kPV_Echo:
  case CPlayerState::kPV_Scan:
    return;
  default:
    break;
  }

  const CGameCamera* camera = mCameraManager->GetCurrentCamera(*this, true);
  gpRender->DrawSpaceWarp(camera->ConvertToScreenSpace(position), strength);
}

void CStateManager::TouchSky() { GetWorld()->TouchSky(); }

void CStateManager::TouchPlayerActor() {
  if (mPlayerActorHead != kInvalidUniqueId) {
    const CEntity* entity = GetObjectById(mPlayerActorHead);
    if (entity != nullptr) {
      PlayerActor_TouchModels(*const_cast< CEntity* >(entity), *this);
    }
  }
}

// Guessed local names. The target excludes these materials when testing dock visibility.
static EMaterialTypes VisAreaExcludeMaterial1 = kMT_NoPlatformCollision;
static EMaterialTypes VisAreaExcludeMaterial2 = kMT_CameraPassthrough;
static EMaterialTypes VisAreaIncludeMaterial = kMT_Unknown59;

TAreaId CStateManager::GetVisAreaId() const {
  const TAreaId currentArea = GetWorld()->GetCurrentAreaId();
  if (IsMultiplayer()) {
    return currentArea;
  }

  const CWorld* world = GetWorld();
  int visibleAreas = 0;
  for (CGameArea::CConstChainIterator it = world->GetChainHead(CWorld::kC_Alive);
       it != CWorld::skGlobalEnd; ++it) {
    if (it->GetOcclusionState() == CGameArea::kOS_Visible) {
      ++visibleAreas;
    }
  }
  if (visibleAreas == 1) {
    return currentArea;
  }

  const CGameCamera* camera = mCameraManagers[0]->GetCurrentCamera(*this, true);
  bool checkDocks = false;
  if (mCameraManagers[0]->IsInBallCamera()) {
    checkDocks = true;
  }
  if (mCameraManagers[0]->GetHintManager()->HasHint(*this) &&
      mCameraManagers[0]->GetHintManager()->GetCurrentHint(*this)->GetAcrossAreas()) {
    checkDocks = true;
  }
  if (mCameraManagers[0]->IsInCinematicCamera()) {
    checkDocks = false;
  }
  if (!checkDocks) {
    return currentArea;
  }

  const CVector3f position = camera->GetTranslation();
  CAABox bounds(position, position);
  const rstl::optional_object< CAABox >& playerBounds = mPlayers[0]->GetTouchBounds();
  bounds.AccumulateBounds(playerBounds->GetMinPoint());
  bounds.AccumulateBounds(playerBounds->GetMaxPoint());

  const rstl::list< CEntity* >& docks = GetDockList();
  for (rstl::list< CEntity* >::const_iterator it = docks.begin(); it != docks.end(); ++it) {
    const CScriptDock* dock = static_cast< const CScriptDock* >(*it);
    if (dock == nullptr || dock->GetAreaId() != currentArea) {
      continue;
    }
    const CAABox dockBounds = dock->GetBoundingBox();
    if (!bounds.DoBoundsOverlap(dockBounds)) {
      continue;
    }
    const TAreaId connectedArea = dock->GetCurrentConnectedAreaId(*this);
    if (world->GetArea(connectedArea)->GetOcclusionState() != CGameArea::kOS_Visible ||
        !dock->HasPointCrossedDock(*this, position)) {
      continue;
    }

    const CGameArea& currentAreaObject = *world->GetArea(currentArea);
    const CVector3f delta = dockBounds.GetCenterPoint() - position;
    const float distance = delta.Magnitude();
    if (distance < FLT_EPSILON) {
      return connectedArea;
    }
    const CVector3f direction = (1.f / distance) * delta;
    const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
        CMaterialList(VisAreaIncludeMaterial),
        CMaterialList(VisAreaExcludeMaterial1, VisAreaExcludeMaterial2));
    if (CGameCollision::RayStaticLineOfSightTest(currentAreaObject, position, direction, distance,
                                                 filter)) {
      return connectedArea;
    }
  }
  return currentArea;
}

void CStateManager::SetActorAreaId(CActor& actor, const TAreaId area) {
  const int oldArea = actor.GetCurrentAreaId().Value();
  if (oldArea != area.Value()) {
    CWorld* world = mWorld.get();
    if (oldArea != kInvalidAreaId.Value()) {
      CGameArea* oldAreaObject = world->Area(TAreaId(oldArea));
      if (oldAreaObject->GetPhase() > CGameArea::kP_FinishScriptObjects) {
        oldAreaObject->ObjectList()->RemoveObject(actor.GetUniqueId());
        if (oldAreaObject->GetPostConstructed()->mPortalArea.get() != nullptr) {
          oldAreaObject->GetPostConstructed()->mPortalArea->RemoveActor(*this, actor.GetUniqueId());
        }
      }
    }

    actor.SetCurrentAreaId(area);
    if (area != kInvalidAreaId) {
      CGameArea* newAreaObject = world->Area(area);
      if (newAreaObject->IsLoaded()) {
        if (newAreaObject->GetObjectList()->GetObjectById(actor.GetUniqueId()) == nullptr) {
          newAreaObject->ObjectList()->AddObject(actor);
        }
      }
    }
  }
}

void CStateManager::SetupParticleHook(const CActor& actor) const {
  mActorModelParticles->SetupHook(actor.GetUniqueId());
}

void CStateManager::ResetEscapeSequenceTimer(float time) {
  gpGameState->SetEscapeTime(time);
  mEscapeTotalTime = time;
}

float CStateManager::GetEscapeSequenceTimer() const { return gpGameState->GetEscapeTime(); }

void CStateManager::UpdateEscapeSequenceTimer(float dt) {
  if (close_enough(mEscapeTotalTime, 0.f)) {
    mEscapeTotalTime = gpGameState->GetEscapeTime();
  }
  const float totalTime = mEscapeTotalTime;
  if (gpGameState->GetEscapeTime() > 0.f) {
    gpGameState->SetEscapeTime(rstl::max_val(FLT_EPSILON, gpGameState->GetEscapeTime() - dt));
    if (gpGameState->GetEscapeTime() <= FLT_EPSILON && mPlayerStates[0]->IsPlayerAlive()) {
      KillPlayer(0.f, mPlayers[0]->GetUniqueId(), kInvalidUniqueId);
    }

    if (!sEscapeRumbleInitialized) {
      sEscapeRumbleInitialized = true;
      sNextEscapeRumble = 0.f;
    }
    sNextEscapeRumble -= dt;
    if (sNextEscapeRumble < 0.f) {
      const float factor = 1.f - gpGameState->GetEscapeTime() / totalTime;
      mRumbleManagers[0]->Rumble(*this, kRFX_PlayerBump, 0.75f, kRP_One);
      sNextEscapeRumble = -12.f * (factor * factor) + 15.f;
    }
  }
}

void CStateManager::UpdateHintState(float dt) {
  CHintOptions& hintOptions = gpGameState->HintOptions();
  hintOptions.Update(dt, *this);

  int nextHintIdx = -1;
  int hintPeriods = -1;
  const CHintOptions::SHintState* currentHint = hintOptions.GetCurrentDisplayedHint();
  if (currentHint != nullptr) {
    const CGameHintInfo::CGameHint& nextHint =
        gpMemoryCard->GetHints()[hintOptions.GetNextHintIdx()];
    const rstl::vector< CGameHintInfo::SHintLocation >& locations = nextHint.GetLocations();
    for (int i = 0; i < static_cast< int >(locations.size()); ++i) {
      const CGameHintInfo::SHintLocation& location = locations[i];
      const int areaId = location.mAreaId.Value();
      const CAssetId worldId = location.mMlvlId;
      CWorldState& worldState = gpGameState->StateForWorld(worldId);
      rstl::rc_ptr< CMapWorldInfo > mapWorldInfo = worldState.MapWorldInfo();
      mapWorldInfo->SetIsMapped(TAreaId(areaId), true);
    }

    if (currentHint->mTime < nextHint.GetTextTime()) {
      nextHintIdx = hintOptions.GetNextHintIdx();
      hintPeriods = static_cast< int >(currentHint->mTime / CGameHintInfo::skHintTextTime);
    }
  }

  if (nextHintIdx != mHintIdx || hintPeriods != static_cast< int >(mHintPeriods)) {
    if (nextHintIdx == -1) {
      CSamusHud::DisplayHudMemo(rstl::wstring_l(L""),
                               CHUDMemoParms(0.f, true, true, true, 15, true));
    } else {
      const CAssetId stringId = gpMemoryCard->GetHints()[nextHintIdx].GetStringId();
      CSamusHud::DeferHintMemo(stringId, hintPeriods,
                             CHUDMemoParms(0.f, true, false, true, 15, true));
    }

    mHintIdx = nextHintIdx;
    mHintPeriods = hintPeriods;
  }
}

void CStateManager::AddWeaponId(TUniqueId owner, EWeaponType type) {
  mWeaponMgr->IncrCount(owner, type);
}

void CStateManager::RemoveWeaponId(TUniqueId owner, EWeaponType type) {
  mWeaponMgr->DecrCount(owner, type);
}

int CStateManager::GetWeaponIdCount(TUniqueId owner, EWeaponType type) {
  return mWeaponMgr->GetNumActive(owner, type);
}

bool CStateManager::RenderLastHUD(const TUniqueId& uid) {
  CStateManagerContainer* container = mStateManagerContainer.get();
  if (container->mRenderLast.size() == container->mRenderLast.capacity()) {
    return false;
  }
  container->mRenderLast.push_back(uid);
  return true;
}

bool CStateManager::RenderLast(TUniqueId uid) {
  CStateManagerContainer* container = mStateManagerContainer.get();
  if (container->mRenderLastUnderGun.size() == container->mRenderLastUnderGun.capacity()) {
    return false;
  }
  container->mRenderLastUnderGun.push_back(uid);
  return true;
}

bool CStateManager::RenderLastOverlay(const TUniqueId& uid) {
  CStateManagerContainer* container = mStateManagerContainer.get();
  if (container->mRenderBeforeAreas.size() == container->mRenderBeforeAreas.capacity()) {
    return false;
  }
  container->mRenderBeforeAreas.push_back(uid);
  return true;
}

int CStateManager::SpecialSkipCinematic() {
  int result = 0;
  if (mSpecialFunctionId != kInvalidUniqueId) {
    CEntity* entity = ObjectById(TUniqueId(mSpecialFunctionId));
    if (entity == nullptr) {
      SetSkipCinematicSpecialFunction(kInvalidUniqueId);
    } else if (CScriptSpecialFunction* special = TCastToPtr< CScriptSpecialFunction >(entity)) {
      const bool randomWasAvailable = IsRandomAvailable();
      mRandomAvailable = true;

      if (special->GetFunction() == CScriptSpecialFunction::kSF_CinematicSkip) {
        mCameraManagers[0]->StopCinematics(*this);
        result = 1;
      } else {
        result = 2;
      }
      special->SkipCinematic(*this);
      mRandomAvailable = randomWasAvailable;
    }
  }
  return result;
}

void CStateManager::DeleteSaveGameScreen() {
  mInSaveUI = mSaveGameScreen->GetMessageReturn() == CIOWin::kMR_Exit;
  mSaveGameScreen = nullptr;
}

void CStateManager::SetGameState(EGameState state) {
  if (mGameState == state) {
    return;
  }

  if (mGameState == kGS_SoftPaused) {
    mWorld->SetLoadPauseState(false);
  }

  switch (state) {
  case kGS_Running:
    for (uint i = 0; i < mNumPlayers; ++i) {
      if (mRumbleManagers[i]->GetDisabled()) {
        mRumbleManagers[i]->SetDisabled(false);
      }
    }
    if (CSfxManager::GetChannel() == CSfxManager::kSC_SoftPaused) {
      CSfxManager::KillAll(CSfxManager::kSC_SoftPaused);
    }
    CSfxManager::SetChannel(CSfxManager::kSC_Game);
    break;
  case kGS_SoftPaused:
    for (uint i = 0; i < mNumPlayers; ++i) {
      if (!mRumbleManagers[i]->GetDisabled()) {
        mRumbleManagers[i]->SetDisabled(true);
      }
    }
    CSfxManager::SetChannel(CSfxManager::kSC_SoftPaused);
    mWorld->SetLoadPauseState(true);
    break;
  default:
    break;
  }

  mGameState = state;
}

void CStateManager::SetBossParams(TUniqueId bossId, float maxEnergy, uint stringIdx) {
  mBossId = bossId;
  mBossHealth = maxEnergy;
  mBossLanguageTableIndex = stringIdx;
}

void CStateManager::DeliverScriptMsg(const CScriptMsg& msg) {
  if (CEntity* entity = ObjectById(msg.GetId())) {
    entity->AcceptScriptMsg(*this, msg);
  }
}

void CStateManager::SendScriptMsg(CEntity* target, TUniqueId sender, EScriptObjectMessage message,
                                  TUniqueId actor) {
  if (target != nullptr) {
    SendScriptMsg(CScriptMsg(sender, actor, target->GetUniqueId(), message, kSS_InvalidState));
  }
}

void CStateManager::SendScriptMsg(TUniqueId target, TUniqueId sender, EScriptObjectMessage message,
                                  TUniqueId actor) {
  SendScriptMsg(CScriptMsg(sender, actor, target, message, kSS_InvalidState));
}

float CStateManager::IntegrateVisorFog(float fog) const {
  const CPlayerState* playerState = mPlayerState;
  if (playerState->GetActiveVisor(*this) == CPlayerState::kPV_Scan) {
    return fog * (1.f - playerState->GetVisorTransitionFactor());
  }
  return fog;
}

uint CStateManager::MaskUIdNumPlayers(TUniqueId id) const {
  const uint index = id.Value();
  return index < static_cast< uint >(mNumPlayers) ? index : 0;
}

void CStateManager::SetupParticleDrawMask() {
  const CPlayerState::EPlayerVisor visor = mPlayerState->GetActiveVisor(*this);
  uint flags = 0;
  uint mask = 8;
  switch (visor) {
  case CPlayerState::kPV_Echo:
    flags |= 8;
    break;
  case CPlayerState::kPV_Combat:
  case CPlayerState::kPV_Scan:
    mask |= 1;
    break;
  case CPlayerState::kPV_Dark:
    mask |= 2;
    break;
  default:
    break;
  }

  uint drawMask = mask | 0x10;
  if (mIsDarkWorld) {
    drawMask = mask | 4;
  }
  CParticleGen::sDrawFlags = flags;
  CParticleGen::sDrawMask = drawMask;
}

void CStateManager::CapturePlayerTextures() {
  uint textureWidth = 32;
  uint textureHeight = 64;
  if (IsMultiplayer()) {
    textureHeight /= 2;
  }
  if (mNumPlayers >= 3u) {
    textureWidth /= 2;
  }

  int left, bottom, width, height;
  if (!gpRender->IsRGBA6Current()) {
    const uint textureSize = textureWidth * textureHeight;
    for (uint i = 0; i < mNumPlayers; ++i) {
      CalculatePlayerViewport(mNumPlayers > 2u ? mPlayerStates[i]->GetPlayerSelection() : i, &left,
                              &bottom, &width, &height);
      CBasics::ZeroMemory(mPlayers[i]->GetReflectionTextureData(), textureSize);
      CBasics::ZeroMemory(mPlayers[i]->GetIndirectTextureData(), textureSize);
      CBasics::ZeroMemory(mPlayers[i]->GetMaskTextureData(), textureSize);
    }
  } else {
    for (uint i = 0; i < mNumPlayers; ++i) {
      CalculatePlayerViewport(mNumPlayers > 2u ? mPlayerStates[i]->GetPlayerSelection() : i, &left,
                              &bottom, &width, &height);
      const int textureLeft = left + width / 2 - textureWidth / 2;
      const int textureTop = CGraphics::GetViewportTop(bottom) + height / 2 - textureHeight / 2;
      gpRender->CopyTextureRegion(mPlayers[i]->GetReflectionTextureData(), 0, textureLeft,
                                  textureTop, textureWidth, textureHeight);
      gpRender->CopyTextureRegion(mPlayers[i]->GetIndirectTextureData(), 1, textureLeft, textureTop,
                                  textureWidth, textureHeight);
      gpRender->CopyTextureRegion(mPlayers[i]->GetMaskTextureData(), 2, textureLeft, textureTop,
                                  textureWidth, textureHeight);
    }
  }
}

void CStateManager::RenderForgottenObjects() {
  const float depthFar = CGraphics::GetDepthFar();
  const float depthNear = CGraphics::GetDepthNear();
  const CFilteredObjectList* list = mFilteredObjectLists[kFOL_ForgottenObject].get();

  CGraphics::SetDepthRange(1.f / 256.f, 2.5f / 256.f);
  for (rstl::list< CEntity* >::const_iterator it = list->GetObjects().begin();
       it != list->GetObjects().end(); ++it) {
    TCastToPtr< CScriptForgottenObject >(*it)->RenderDepthOnly(*this);
  }

  CGraphics::SetDepthRange(2.5f / 256.f, 4.f / 256.f);
  for (rstl::list< CEntity* >::const_iterator it = list->GetObjects().begin();
       it != list->GetObjects().end(); ++it) {
    TCastToPtr< CScriptForgottenObject >(*it)->RenderAlphaMask(*this);
  }
  CGraphics::SetDepthRange(depthNear, depthFar);
}

void CStateManager::UpdateDynamicLayers() {
  for (CGameArea::CChainIterator it = mWorld->ChainHead(CWorld::kC_Alive);
       it != CWorld::AliveAreasEnd(); ++it) {
    it->UpdateDynamicLayers(*this);
  }
}

bool CStateManager::HasPendingLayerLoads() const {
  for (CGameArea::CConstChainIterator it = mWorld->GetChainHead(CWorld::kC_Alive);
       it != CWorld::skGlobalEnd; ++it) {
    if (it->HasPendingLayerLoads()) {
      return true;
    }
  }
  return false;
}

void CStateManager::SetPortalTransition(rstl::single_ptr< CPortalTransition >& transition) {
  mPortalTransition = transition;
}

rstl::single_ptr< CPortalTransition >& CStateManager::TakePortalTransition() {
  return mPortalTransition;
}

CScriptObjectLoaderHelper& CStateManager::ScriptObjectLoaderHelper() {
  return mStateManagerContainer->mScriptObjectLoader;
}

CScopedProfiler::CScopedProfiler(const rstl::string& name, bool enabled) {}

const bool gkWorldOnlyReflection = false;

CStateManager::CStateManager(
    const rstl::ncrc_ptr< CScriptMailbox >& mailbox,
    const rstl::ncrc_ptr< CMapWorldInfo >& mapWorldInfo,
    const rstl::reserved_vector< rstl::ncrc_ptr< CPlayerState >, 4 >& playerStates,
    const rstl::ncrc_ptr< CWorldTransManager >& worldTransManager,
    const rstl::ncrc_ptr< CWorldLayerState >& worldLayerState)
: mNextFreeIndex(0)
, mObjectIndexArray(0)
, mObjectLists(rstl::auto_ptr< CObjectList >())
, mFilteredObjectLists(rstl::auto_ptr< CFilteredObjectList >())
, mAllocatedObjectIndices(kMaxObjects, false)
, mArchQueue(nullptr)
, mNumPlayers(0)
, mForceTriggerIds(kInvalidUniqueId)
, mCurrentRenderPlayer(nullptr)
, mPlayerState(nullptr)
, mCameraManager(nullptr)
, mWorld(nullptr)
, mStateManagerContainer(rs_new CStateManagerContainer())
, mSortedListManager(&mStateManagerContainer->mSortedListManager)
, mWeaponMgr(&mStateManagerContainer->mWeaponManager)
, mFluidPlaneManager(&mStateManagerContainer->mFluidPlaneManager)
, mEnvFxManager(&mStateManagerContainer->mEnvFxManager)
, mActorModelParticles(&mStateManagerContainer->mActorModelParticles)
, mSafeZoneManager(&mStateManagerContainer->mSafeZoneManager)
, mAudioGroupDependencies(static_cast< CDependencyGroup* >(nullptr))
, mPlayerStateOwners(playerStates)
, mMailbox(mailbox)
, mMapWorldInfo(mapWorldInfo)
, mWorldTransManager(worldTransManager)
, mCurrentWorldLayerState(worldLayerState)
, mNextAreaId(0)
, mPreviousAreaId(kInvalidAreaId)
, mRenderFrameIndex(0)
, mUpdateFrameIdx(0)
, mObjectDrawToken(0)
, mUnknown0x16b4(0)
, mShadowTex(gpSimplePool->GetObj("DefaultShadow"))
, mRandom(0)
, mRandomAvailable(false)
, mGameState(kGS_Running)
, mInitPhase(kIP_LoadAudioGroups)
, mCameraFilterPasses(4, rstl::reserved_vector< CCameraFilterPass, 11 >(11, CCameraFilterPass()))
, mCameraBlurPasses(4, rstl::reserved_vector< CCameraBlurPass, 11 >(11, CCameraBlurPass()))
, mHintIdx(-1)
, mHintPeriods(0)
, mPauseHudMessage(kInvalidAssetId)
, mEscapeTotalTime(0.f)
, mCurTimeMod900(0.f)
, mBossId(kInvalidUniqueId)
, mBossHealth(0.f)
, mBossLanguageTableIndex(0)
, mRenderVisorMode(kRVM_Normal)
, mSpecialFunctionId(kInvalidUniqueId)
, mPlayerActorHead(kInvalidUniqueId)
, mHudMessageTime(0.f)
, mProjectedShadows(nullptr)
, mHudMessageFrameCount(0)
, mPausedHudMemoFrameCount(-1)
, mPausedHudMemoAssetId(kInvalidAssetId)
, mQueuedHudMemoDismissalDelay(0.f)
, mMapTeleportWorldId(kInvalidAssetId)
, mDeferredTransition(kSMT_InGame)
, mPlayerLineOfSightPairs(0)
, mNextPlayerLineOfSightPair(0)
, mPlanes()
, mCurrentRenderPlayerIndex(kInvalidRenderPlayerIndex)
, mVisAreaId(-1)
, mPendingDockArea(kInvalidAreaId)
, mPendingDock(0)
, mUnknown0x2908(CTransform4f::Identity())
, mDarkWorldCloudScale(CVector3f::Zero())
, mDarkWorldCloudTime(0.f)
, mDarkWorldCloudColor(CColor::Black())
, mReadyToRender(false)
, mQuitGame(false)
, mUnkFlagA3(true)
, mInMapScreen(false)
, mInSaveUI(false)
, mCinematicPause(false)
, mIsFullThreat(false)
, mIsDarkWorld(false)
, mShowSoftTransition(true)
, mTearingDown(false)
, mDispatchingScriptMessages(false)
, mLayerRestartPending(false)
, mLightAmmoDepletedPlayers(0)
, mDarkAmmoDepletedPlayers(0) {
  mRumbleManagers[0] = &mStateManagerContainer->mRumbleManager0;
  mRumbleManagers[1] = &mStateManagerContainer->mRumbleManager1;
  mRumbleManagers[2] = &mStateManagerContainer->mRumbleManager2;
  mRumbleManagers[3] = &mStateManagerContainer->mRumbleManager3;

  mObjectLists[kOL_All] = rs_new CObjectList(kOL_All, false);
  mObjectLists[kOL_Actor] = rs_new CActorList();
  mObjectLists[kOL_PhysicsActor] = rs_new CPhysicsActorList();
  mObjectLists[kOL_GameLight] = rs_new CGameLightList();
  mObjectLists[kOL_ListeningAi] = rs_new CListeningAiList();
  mObjectLists[kOL_AiWaypoint] = rs_new CAiWaypointList();
  mObjectLists[kOL_Platform] = rs_new CPlatformList();
  mObjectLists[kOL_Trigger] = rs_new CTriggerList();

  mFilteredObjectLists[kFOL_Dock] = rs_new CFilteredDockList();
  mFilteredObjectLists[kFOL_Door] = rs_new CFilteredDoorList();
  mFilteredObjectLists[kFOL_Type124] = rs_new CFilteredType124List();
  mFilteredObjectLists[kFOL_ForgottenObject] = rs_new CFilteredForgottenObjectList();
  mFilteredObjectLists[kFOL_GameCamera] = rs_new CFilteredGameCameraList();
  mFilteredObjectLists[kFOL_GrapplePoint] = rs_new CFilteredGrapplePointList();

  for (int i = 0; i < mObjectLists.size(); ++i) {
    CObjectList* list = mObjectLists[i].get();
    if (list->IsDynamic()) {
      mDynamicObjectLists.push_back(list);
    }
  }
  for (int i = 0; i < mFilteredObjectLists.size(); ++i) {
    CFilteredObjectList* list = mFilteredObjectLists[i].get();
    if (list->IsDynamic()) {
      mDynamicFilteredObjectLists.push_back(list);
    }
  }

  gpRender->SetDrawableCallback(RendererDrawCallback, this);
  CMemory::SetOutOfMemoryCallback(MemoryAllocatorAllocationFailedCallback, this);
  CGameCollision::InitCollision(this);
  CMemory::OffsetFakeStatics(mObjectLists.size() * sizeof(CObjectList) +
                             mFilteredObjectLists.size() * sizeof(CFilteredObjectList) + 0x12c);
  mShadowTex.Lock();
  gpMain->SetThirtyFps(gpGameState->GetGameMode().GetNumPlayers() == 2);
}

CStateManager::~CStateManager() {
  mTearingDown = true;
  CMemory::OffsetFakeStatics(-(mObjectLists.size() * sizeof(CObjectList) +
                              mFilteredObjectLists.size() * sizeof(CFilteredObjectList) + 0x12c));
  for (uint i = 0; i < mNumPlayers; ++i) {
    mRumbleManagers[i]->HardStopAll();
  }
  mEnvFxManager->Cleanup();
  mRandomAvailable = true;

  CObjectList& objects = *mObjectLists[kOL_All];
  ClearGraveyard();
  for (int i = 0; i != kMaxObjects; ++i) {
    CEntity* entity = objects[i];
    if (entity != nullptr && TCastToPtr< CPlayer >(entity) == nullptr &&
        TCastToPtr< CGameCamera >(entity) == nullptr) {
      DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, kInvalidUniqueId, entity->GetUniqueId(),
                                 kSM_Delete, kSS_InvalidState));
    }
  }
  for (int i = 0; i != kMaxObjects; ++i) {
    CEntity* entity = objects[i];
    if (entity != nullptr && TCastToPtr< CPlayer >(entity) == nullptr &&
        TCastToPtr< CGameCamera >(entity) == nullptr) {
      RemoveObject(entity->GetUniqueId());
      delete entity;
    }
  }
  ClearGraveyard();

  const CFilteredObjectList cameras(*mFilteredObjectLists[kFOL_GameCamera]);
  const rstl::list< CEntity* >& cameraObjects = cameras.GetObjects();
  for (rstl::list< CEntity* >::const_iterator it = cameraObjects.begin();
       it != cameraObjects.end(); ++it) {
    if (CGameCamera* camera = TCastToPtr< CGameCamera >(*it)) {
      DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, kInvalidUniqueId, camera->GetUniqueId(),
                                 kSM_Delete, kSS_InvalidState));
      RemoveObject(camera->GetUniqueId());
      delete camera;
    }
  }
  for (uint i = 0; i < mNumPlayers; ++i) {
    if (CPlayer* player = mPlayers[i]) {
      DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, kInvalidUniqueId, player->GetUniqueId(),
                                 kSM_Delete, kSS_InvalidState));
      RemoveObject(player->GetUniqueId());
      delete player;
    }
  }

  CGameCollision::UninitializeCollision();
  CMemory::SetOutOfMemoryCallback(nullptr, nullptr);
  gpMain->SetThirtyFps(false);
  CAudioGrpSetLoc::sInSinglePlayer = false;
}

void CStateManager::ClearGraveyard() {
  for (rstl::list< rstl::reserved_vector< CEntity*, 32 > >::iterator it = mGraveyard.begin();
       it != mGraveyard.end(); ++it) {
    rstl::reserved_vector< CEntity*, 32 >& batch = *it;
    for (rstl::reserved_vector< CEntity*, 32 >::iterator entity = batch.begin();
         entity != batch.end(); ++entity) {
      delete *entity;
    }
  }
  mGraveyard.clear();
}

void CStateManager::FrameBegin(uint frame) {
  mRenderFrameIndex = frame;
  CTexture::sCurrentFrameCount = mRenderFrameIndex;
  CGraphicsPalette::sCurrentFrameCount = mRenderFrameIndex;
  SwapOutTexturesToARAM(2, 0x180000);
}

void CStateManager::SwapOutTexturesToARAM(int, uint) {}

const bool CStateManager::MemoryAllocatorAllocationFailedCallback(const void* context, uint) {
  return static_cast< CStateManager* >(const_cast< void* >(context))->SwapOutAllPossibleMemory();
}

bool CStateManager::SwapOutAllPossibleMemory() {
  CFrameDelayedKiller::StallAndFlushAllAllocations();
  CARAMManager::WaitForAllDMAsToComplete();
  CARAMToken::UpdateAllDMAs();
  return true;
}

void CStateManager::RecursiveDrawTree(TUniqueId uid) const {
  CActor* actor = TCastToPtr< CActor >(const_cast< CEntity* >(GetObjectById(uid)));
  if (actor != nullptr && mObjectDrawToken != actor->GetDrawToken()) {
    const TUniqueId nextNode = actor->GetDrawParent();
    if (nextNode != kInvalidUniqueId) {
      RecursiveDrawTree(nextNode);
    }
    if (mObjectDrawToken == actor->GetAddedToken()) {
      actor->Render(*this);
    }
    actor->SetDrawToken(mObjectDrawToken);
  }
}

void CStateManager::RendererDrawCallback(const void* drawable, const void* context, int type) {
  const CStateManager& mgr = *static_cast< const CStateManager* >(context);
  switch (type) {
  case 0: {
    const CActor& actor = *static_cast< const CActor* >(drawable);
    if (mgr.mObjectDrawToken == actor.GetDrawToken()) {
      break;
    }
    const TUniqueId nextNode = actor.GetDrawParent();
    if (nextNode != kInvalidUniqueId) {
      mgr.RecursiveDrawTree(nextNode);
    }
    actor.Render(mgr);
    actor.SetDrawToken(mgr.mObjectDrawToken);
    break;
  }
  case 1:
    static_cast< const CSimpleShadow* >(drawable)->Render(mgr.mShadowTex.GetObject());
    break;
  case 2:
    static_cast< const CDecal* >(drawable)->Render();
    break;
  }
}

TUniqueId CStateManager::AllocateUniqueId() {
  ushort lastIndex = mNextFreeIndex;
  ushort ourIndex;
  do {
    ourIndex = mNextFreeIndex;
    mNextFreeIndex = (ourIndex + 1) % 1024;
    if (mNextFreeIndex == lastIndex) {
      rs_debugger_printf("Object list full!");
    }
  } while (mAllocatedObjectIndices[ourIndex]);

  mObjectIndexArray[ourIndex] = (mObjectIndexArray[ourIndex] + 1) & 0x3f;
  if (TUniqueId(mObjectIndexArray[ourIndex], ourIndex) == kInvalidUniqueId) {
    mObjectIndexArray[ourIndex] = 0;
  }

  mAllocatedObjectIndices[ourIndex] = true;

  return TUniqueId(mObjectIndexArray[ourIndex], ourIndex);
}

void CStateManager::CreateStandardGameObjects() {
  mNumPlayers = gpGameState->GetGameMode().GetNumPlayers();
  CTweakPlayerGun* playerGunTweak = gpTweakPlayerGunSingle.get();
  if (mNumPlayers > 1u) {
    playerGunTweak = gpTweakPlayerGunMulti.get();
  }
  gpTweakPlayerGun = playerGunTweak;

  mCameraManagers[0] = &mStateManagerContainer->mCameraManager0;
  mCameraManagers[1] = &mStateManagerContainer->mCameraManager1;
  mCameraManagers[2] = &mStateManagerContainer->mCameraManager2;
  mCameraManagers[3] = &mStateManagerContainer->mCameraManager3;
  for (uint i = 0; i < mNumPlayers; ++i) {
    const TUniqueId uid = AllocateUniqueId();
    const CVector3f position(5.f * (i & 1), 0.f, 5.f * (i & 2));
    const CRelAngle angle = CRelAngle::FromDegrees(129.6f);
    const CMatrix3f matrix =
        CQuaternion::AxisAngle(CUnitVector3f(0.f, 0.f, 1.f, CUnitVector3f::kN_Yes), angle)
            .BuildTransform();
    const CTransform4f transform = CTransform4f::FromColumns(
        matrix.GetColumn(kDX), matrix.GetColumn(kDY), matrix.GetColumn(kDZ), position);

    CTweakPlayer* tweak = gpTweakPlayerA.get();
    const int controlScheme = mPlayerStateOwners[i]->GetControlScheme();
    if (controlScheme == 1) {
      tweak = gpTweakPlayerB.get();
    }
    const float stepUp = tweak->GetStepUpHeight();
    const float stepDown = tweak->GetStepDownHeight();
    const float height = tweak->GetPlayerHeight();
    const float radius = tweak->GetPlayerRadius();
    const float ballRadius = tweak->GetBallRadius();
    const CAABox bounds(CVector3f(-radius, -radius, 0.f), CVector3f(radius, radius, height));

    const SObjectTag* animationController = gpResourceFactory->GetResourceIdByName(
        IsMultiplayer() ? skMultiplayerAnimController : skSinglePlayerAnimController);
    const CAssetId stateMachine = animationController ? animationController->id : kInvalidAssetId;
    mPlayerStates[i] = mPlayerStateOwners[i].GetPtr();
    int characterIndex = 3;
    if (IsMultiplayer()) {
      characterIndex = 0;
    }
    mPlayers[i] = rs_new CPlayer(
        uid, transform, bounds, gpTweakPlayerRes->GetBallTransitionANCSId(), stateMachine,
        200.f, stepUp, stepDown, ballRadius,
        CMaterialList(kMT_Player, kMT_Unknown59, kMT_GroundCollider, kMT_Target, kMT_NoPlayerCollision),
        mPlayerStates[i], mCameraManagers[i], mNumPlayers > 1u, i,
        mPlayerStateOwners[i]->GetControlScheme(), characterIndex);
  }

  for (uint i = 0; i < mNumPlayers; ++i) {
    AddObject(*mPlayers[i]);
    if (!skDisablePlayerTargeting) {
      mPlayers[i]->AddMaterial(kMT_Orbit, kMT_Scannable, *this);
    }
  }

  float fieldOfView = gpTweakGame->GetFieldOfView();
  if (mNumPlayers == 2u) {
    fieldOfView = gpTweakGame->GetTwoPlayerFieldOfView();
  }
  for (uint i = 0; i < mNumPlayers; ++i) {
    mCameraManagers[i]->SetFirstPersonFOV(fieldOfView);
    mCameraManagers[i]->CreateCameras(*this);
  }
  mEnvFxManager->AsyncLoadResources(*this);
}

void CStateManager::SpawnPlayer(CScriptSpawnPoint& spawnPoint, uint playerIndex) {
  CPlayer& player = *mPlayers[playerIndex];
  CPlayerState& state = *player.GetPlayerState();
  const CVector3f position = spawnPoint.GetTransform().GetTranslation();
  CVector3f forward = spawnPoint.GetTransform().GetForward();
  forward.SetZ(0.f);
  if (forward.CanBeNormalized()) {
    player.Teleport(CTransform4f::LookAt(position, position + forward, CVector3f::Up()),
                    *this, true);
    player.SetSpawnedMorphBallState(
        spawnPoint.IsMorphed() ? CPlayer::kMS_Morphed : CPlayer::kMS_Unmorphed, *this);
    player.ResetPlayerState(*this, 1);
  }

  if (gpGameState->GetGameMode().IsMultiplayer() || gpGameState->GetInitPowerupsAtFirstSpawn()) {
    gpGameState->SetDeferPowerupInit(false);
    for (int i = CPlayerState::kIT_PowerBeam; i < CPlayerState::kIT_Max; ++i) {
      const CPlayerState::EItemType item = static_cast< CPlayerState::EItemType >(i);
      if (state.GetItemCapacity2(item) < spawnPoint.GetItemCapacity(item)) {
        state.AddPowerUp(item, spawnPoint.GetItemCapacity(item) - state.GetItemCapacity2(item));
      }
      if (state.GetItemAmount(item, true) < spawnPoint.GetItemAmount(item)) {
        state.IncrPickUp(item, spawnPoint.GetItemAmount(item) - state.GetItemAmount(item, true));
      }
    }
  }

  const uint spawnedPlayerIndex = player.GetPlayerIndex();
  const CGameState& gameState = *gpGameState;
  gameState.GetGameMode().OnPlayerSpawned(*this, spawnedPlayerIndex);
  spawnPoint.SendSpawnMessage(*this, player);
  player.AsyncLoadSuit(*this);
}

void CStateManager::InitializeState(CAssetId worldId, TAreaId areaId, CAssetId mreaId) {
  const bool randomWasAvailable = IsRandomAvailable();
  mRandomAvailable = true;
  if (mInitPhase == kIP_LoadAudioGroups) {
    if (!IsMultiplayer()) {
      CAudioGrpSetLoc::sInSinglePlayer = true;
    }
    if (gpGameState->AudioGroups().empty()) {
      const int gameMode = gpGameState->GetGameMode().GetGameModeType();
      int dependencyIndex = 2;
      if (gameMode == 'SNGL') {
        dependencyIndex = 0;
      }
      if (gameMode == 'FRND') {
        dependencyIndex = 1;
      }
      mAudioGroupDependencies =
          TToken< CDependencyGroup >(gpSimplePool->GetObj(skAudioGroupDependencies[dependencyIndex]));
      mAudioGroupDependencies.Lock();
    }
    mInitPhase = kIP_LoadWorld;
  }

  if (mInitPhase == kIP_LoadWorld) {
    if (gpGameState->AudioGroups().empty()) {
      if (!mAudioGroupDependencies.IsLoaded()) {
        return;
      }
      CDependencyGroup& dependencies = **mAudioGroupDependencies;
      rstl::vector< TCachedToken< CAudioGrpSetLoc > >& audioGroups = gpGameState->AudioGroups();
      const rstl::vector< SObjectTag >& tags = dependencies.GetObjectTagVector();
      audioGroups.reserve(tags.size());
      for (rstl::vector< SObjectTag >::const_iterator it = tags.begin(); it != tags.end(); ++it) {
        if (it->type == 'AGSC') {
          audioGroups.push_back_unsafe(TCachedToken< CAudioGrpSetLoc >(gpSimplePool->GetObj(*it)));
        }
      }
      mAudioGroupDependencies.Unlock();
      for (rstl::vector< TCachedToken< CAudioGrpSetLoc > >::iterator it = audioGroups.begin();
           it != audioGroups.end(); ++it) {
        it->Lock();
      }
    }
    CreateStandardGameObjects();
    mWorld = rs_new CWorld(*gpSimplePool, *gpResourceFactory, worldId);
    mInitPhase = kIP_LoadFirstArea;
  }

  if (mInitPhase == kIP_LoadFirstArea) {
    if (!mShadowTex.IsLoaded()) {
      return;
    }
    if (mNumPlayers == 3u && mUnusedViewportTexture.null()) {
      mUnusedViewportTexture =
          rs_new TCachedToken< CTexture >(gpSimplePool->GetObj(skUnusedViewportTexture));
      mUnusedViewportTexture->Lock();
      return;
    }
    if (!mUnusedViewportTexture.null() && !mUnusedViewportTexture->IsLoaded()) {
      return;
    }
    if (!mWorld->CheckWorldComplete(this, areaId, mreaId)) {
      return;
    }
    mNextAreaId = mWorld->GetCurrentAreaId();
    CGameArea* area = mWorld->Area(GetNextAreaId());
    if (mWorld->ScheduleAreaToLoad(area, *this)) {
      area->StartStreamIn(*this);
      return;
    }
    mInitPhase = kIP_Done;
  }

  SetCurrentAreaId(mNextAreaId);
  gpGameState->CurrentWorldState().SetAreaId(mNextAreaId);
  mWorld->TravelToArea(mNextAreaId, *this, CWorld::kATT_SkipAdjacent);
  CObjectList* allObjects = mObjectLists[kOL_All].get();
  for (int i = allObjects->GetFirstObjectIndex(); i != -1; i = allObjects->GetNextObjectIndex(i)) {
    DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, kInvalidUniqueId, (*allObjects)[i]->GetUniqueId(),
                               kSM_WorldLoaded, kSS_InvalidState));
  }

  if (mPendingDockArea != kInvalidAreaId) {
    CObjectList* objects = mObjectLists[kOL_All].get();
    for (int i = objects->GetFirstObjectIndex(); i != -1; i = objects->GetNextObjectIndex(i)) {
      CActor& actor = *static_cast< CActor* >((*objects)[i]);
      const TAreaId actorArea = actor.GetCurrentAreaId();
      SetActorAreaId(actor, kInvalidAreaId);
      SetActorAreaId(actor, actorArea);
    }
  }

  int nextSpawn = 0;
  if (mPendingDockArea != kInvalidAreaId) {
    CScriptDock* dock = nullptr;
    CObjectList* objects = mObjectLists[kOL_All].get();
    for (int i = objects->GetFirstObjectIndex(); i != -1; i = objects->GetNextObjectIndex(i)) {
      if (CScriptDock* candidate = TCastToPtr< CScriptDock >((*objects)[i])) {
        if (candidate->GetDockId() == mPendingDock) {
          dock = candidate;
          break;
        }
      }
    }
    CScriptSpawnPoint* spawn = nullptr;
    if (dock != nullptr) {
      spawn = TCastToPtr< CScriptSpawnPoint >(
          ObjectById(dock->FindConnectedObject(*this, kSS_MaxReached, kSM_Activate)));
    }
    if (spawn != nullptr) {
      SpawnPlayer(*spawn, 0);
      nextSpawn = 1;
    }
  }

  if (!IsMultiplayer()) {
    if (nextSpawn == 0) {
      CObjectList* objects = mObjectLists[kOL_All].get();
      for (int i = objects->GetFirstObjectIndex(); i != -1; i = objects->GetNextObjectIndex(i)) {
        if (CScriptSpawnPoint* spawn = TCastToPtr< CScriptSpawnPoint >((*objects)[i])) {
          if (spawn->GetActive() && spawn->IsFirstSpawn()) {
            SpawnPlayer(*spawn, nextSpawn);
            break;
          }
        }
      }
    }
  } else {
    rstl::vector< TUniqueId > spawnPoints;
    spawnPoints.reserve(mNumPlayers);
    CObjectList* objects = mObjectLists[kOL_All].get();
    for (int i = objects->GetFirstObjectIndex(); i != -1; i = objects->GetNextObjectIndex(i)) {
      if (CScriptSpawnPoint* spawn = TCastToPtr< CScriptSpawnPoint >((*objects)[i])) {
        if (spawn->GetActive() && spawn->IsFirstSpawn()) {
          spawnPoints.reserve(spawnPoints.size() + 1);
          spawnPoints.push_back_unsafe(spawn->GetUniqueId());
        }
      }
    }
    while (!spawnPoints.empty() && spawnPoints.size() < spawnPoints.capacity()) {
      spawnPoints.push_back_unsafe(spawnPoints.front());
    }
    CRandom16 random(CStopwatch::GetGlobalMicros());
    rstl::random_shuffle(spawnPoints.begin(), spawnPoints.end(), random);
    for (uint i = 0; i < mNumPlayers; ++i) {
      const int startingSpawn = nextSpawn;
      do {
        if (CScriptSpawnPoint* spawn = TCastToPtr< CScriptSpawnPoint >(ObjectById(spawnPoints[nextSpawn]))) {
          SpawnPlayer(*spawn, i);
          nextSpawn = (nextSpawn + 1) % spawnPoints.size();
          break;
        }
        nextSpawn = (nextSpawn + 1) % spawnPoints.size();
      } while (nextSpawn != startingSpawn);
    }
  }
  mRandomAvailable = randomWasAvailable;
}

bool CStateManager::PrepareAreaTransition(TAreaId areaId) {
  TAreaId areaToKeep = areaId;
  if (!mWorld->UnloadAllAreasExcept(*this, areaToKeep)) {
    return false;
  }
  for (uint i = 0; i < mNumPlayers; ++i) {
    mRumbleManagers[i]->HardStopAll();
  }
  mPendingDockArea = areaId;
  mInitPhase = kIP_LoadFirstArea;

  CObjectList* objects = mObjectLists[kOL_All].get();
  for (int i = objects->GetFirstObjectIndex(); i != -1; i = objects->GetNextObjectIndex(i)) {
    CEntity& entity = *(*objects)[i];
    if (entity.GetCurrentAreaId() != kInvalidAreaId) {
      entity.SetCurrentAreaId(areaId);
    }
    if (TCastToPtr< CWeapon >(&entity) != nullptr) {
      DeleteObjectRequest(entity.GetUniqueId());
    }
  }

  const bool randomWasAvailable = IsRandomAvailable();
  mRandomAvailable = true;
  mPlayers[0]->GetPlayerGun()->Reset(*this);
  mRandomAvailable = randomWasAvailable;
  mPlayers[0]->StopSounds();
  SetIsDarkWorld(!mIsDarkWorld);
  mUnknown0x2908 = CTransform4f::Identity();
  mDarkWorldCloudScale = CVector3f::Zero();
  mDarkWorldCloudTime = 0.f;
  mDarkWorldCloudColor = CColor::Black();
  return true;
}

const CEntity* CStateManager::GetObjectById(TUniqueId uid) const {
  return GetObjectListById(kOL_All).GetObjectById(uid);
}

void CStateManager::SetIsDarkWorld(bool b) {
  mIsDarkWorld = b;
  gpGameState->SetIsDarkWorld(mIsDarkWorld);
}

bool CStateManager::HasWorld() const { return !mWorld.null(); }

void CStateManager::UpdateSortedLists() {
  if (mWorld.get() == nullptr) {
    return;
  }

  CObjectList* actorList = mObjectLists[kOL_Actor].get();
  for (int i = actorList->GetFirstObjectIndex(); i != -1; i = actorList->GetNextObjectIndex(i)) {
    CActor* actor = static_cast< CActor* >((*actorList)[i]);
    if (actor != nullptr) {
      UpdateActorInSortedLists(actor);
    }
  }
}

void CStateManager::AddObject(CEntity& entity) {
  const TUniqueId id = entity.GetUniqueId();
  if (entity.GetEditorId() != kInvalidEditorId) {
    mScriptIdMap.insert(rstl::pair< TEditorId, TUniqueId >(entity.GetEditorId(), id));
  }

  for (rstl::reserved_vector< rstl::auto_ptr< CObjectList >, 8 >::iterator it =
           mObjectLists.begin();
       it != mObjectLists.end(); ++it) {
    (*it)->AddObject(entity);
  }
  for (rstl::reserved_vector< rstl::auto_ptr< CFilteredObjectList >, 6 >::iterator it =
           mFilteredObjectLists.begin();
       it != mFilteredObjectLists.end(); ++it) {
    (*it)->AddObject(entity);
  }
  mNewObjectIds.push_back(id);

  if (entity.GetCurrentAreaId() == kInvalidAreaId && TCastToPtr< CPlayer >(entity) == nullptr) {
    entity.SetCurrentAreaId(mPlayers[0]->GetCurrentAreaId());
  }

  CActor* actor = TCastToPtr< CActor >(entity);
  const TAreaId areaId = entity.GetCurrentAreaId();
  if (areaId != kInvalidAreaId) {
    CGameArea* area = mWorld->Area(areaId);
    if (area->GetPhase() > CGameArea::kP_FinishDependencies) {
      area->ObjectList()->AddObject(entity);
    }
  }
  if (actor != nullptr) {
    UpdateActorInSortedLists(actor);
  }

  DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, kInvalidUniqueId, entity.GetUniqueId(), kSM_Create,
                              kSS_InvalidState));
  if (entity.GetCurrentAreaId() != kInvalidAreaId && HasWorld()) {
    CGameArea* area = mWorld->Area(entity.GetCurrentAreaId());
    if (area->GetPhase() > CGameArea::kP_FinishDependencies &&
        area->GetPostConstructed()->mScriptObjectsInitialized) {
      DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, kInvalidUniqueId, entity.GetUniqueId(),
                                  kSM_AreaLoaded, kSS_InvalidState));
    }
  }
}

void CStateManager::RemoveObject(TUniqueId id) {
  CEntity* entity = mObjectLists[kOL_All]->GetObjectById(id);
  if (entity != nullptr) {
    const TEditorId editorId = entity->GetEditorId();
    if (editorId != kInvalidEditorId) {
      const rstl::pair< TIdList::iterator, TIdList::iterator > range =
          mScriptIdMap.equal_range(editorId);
      TIdList::iterator it = range.first;
      while (it != range.second) {
        if (it->second == id) {
          it = mScriptIdMap.erase(it);
        } else {
          ++it;
        }
      }
    }

    const TAreaId areaId = entity->GetCurrentAreaId();
    if (areaId != kInvalidAreaId) {
      CGameArea* area = mWorld->Area(areaId);
      if (area->IsLoaded()) {
        area->ObjectList()->RemoveObject(id);
        if (area->GetPostConstructed()->mPortalArea.get() != nullptr) {
          area->GetPostConstructed()->mPortalArea->RemoveActor(*this, id);
        }
      }
    }

    if (CActor* actor = TCastToPtr< CActor >(entity)) {
      mSortedListManager->Remove(actor);
      actor->SetUseInSortedLists(false);
    }
  }

  for (int i = 0; i < mObjectLists.size(); ++i) {
    mObjectLists[i]->RemoveObject(id);
  }
  for (int i = 0; i < mFilteredObjectLists.size(); ++i) {
    mFilteredObjectLists[i]->RemoveObject(id);
  }
  mAllocatedObjectIndices[id.Value()] = false;
}

void CStateManager::SendDamageScriptMsgs(CActor& damagee, TUniqueId source,
                                         const CDamageInfo& damage) {
  damagee.SendScriptMsgs(kSS_Damage, *this, kSM_None);
  EScriptObjectState state = kSS_InvalidState;
  switch (damage.GetWeaponMode1()) {
  case kWT_Power:
    state = kSS_PowerDamage;
    break;
  case kWT_Dark:
    state = kSS_DarkDamage;
    break;
  case kWT_Light:
    state = kSS_LightDamage;
    break;
  case kWT_Annihilator:
    state = kSS_AnnihilatorDamage;
    break;
  case kWT_Bomb:
    state = kSS_BombDamage;
    break;
  case kWT_PowerBomb:
    state = kSS_PowerBombDamage;
    break;
  case kWT_Missile:
    state = kSS_MissileDamage;
    break;
  case kWT_BoostBall:
    state = kSS_BoostBallDamage;
    break;
  case kWT_CannonBall:
    state = kSS_CannonBallDamage;
    break;
  case kWT_ScrewAttack:
    state = kSS_ScrewAttackDamage;
    break;
  case kWT_Phazon:
    state = kSS_PhazonDamage;
    break;
  case kWT_AI:
    state = kSS_AIDamage;
    break;
  case kWT_PoisonWater1:
    state = kSS_PoisonWaterDamage;
    break;
  case kWT_PoisonWater2:
    state = kSS_PoisonWaterDamage;
    break;
  case kWT_Lava:
    state = kSS_LavaDamage;
    break;
  case kWT_Heat:
    state = kSS_HeatDamage;
    break;
  case kWT_Unused1:
    state = kSS_ColdDamage;
    break;
  case kWT_AreaDark:
    state = kSS_AreaDarkDamage;
    break;
  case kWT_AreaLight:
    state = kSS_AreaLightDamage;
    break;
  case kWT_UnknownSource:
    state = kSS_UnknownSourceDamage;
    break;
  case kWT_SafeZone:
    state = kSS_InvalidState;
    break;
  default:
    break;
  }
  if (state != kSS_InvalidState) {
    damagee.SendScriptMsgs(state, *this, kSM_None);
  }
}

void CStateManager::ApplyDamage(TUniqueId damagerId, TUniqueId damageeId, TUniqueId owner,
                                 const CDamageInfo& damage, const CMaterialFilter& filter,
                                 const CVector3f& knockbackDirection) {
  // The native comparison uses the raw unsigned halfword, not the signed enum getter.
  if (damage.GetWeaponMode().GetRawType() == kWT_None) {
    return;
  }

  const CDamageInfo info = GetModifiedDamageInfo(damagerId, owner, damageeId, damage);
  const CEntity* damagerEntity = GetObjectById(damagerId);
  CEntity* damageeEntity = ObjectById(damageeId);
  const CActor* const damager = TCastToConstPtr< CActor >(damagerEntity);
  CActor* const damagee = TCastToPtr< CActor >(damageeEntity);
  const bool isPlayer = TCastToConstPtr< CPlayer >(damageeEntity) != nullptr;
  if (damagee == nullptr) {
    return;
  }

  if (damagee->GetHealthInfo() != nullptr) {
    CVector3f position(0.f, 0.f, 0.f);
    CVector3f direction(1.f, 0.f, 0.f);
    if (damager != nullptr) {
      position = damager->GetTransform().GetTranslation();
      direction = damager->GetTransform().GetForward();
    }

    const bool useWeaponDirection = damager != nullptr || isPlayer;
    const CDamageVulnerability* vulnerability =
        useWeaponDirection ? damagee->GetDamageVulnerability(position, direction, info)
                           : damagee->GetDamageVulnerability();
    if (info.GetWeaponMode().GetRawType() == kWT_None ||
        vulnerability->WeaponHits(info.GetWeaponMode(), 0)) {
      const float localDamage = info.GetDamage(*vulnerability);
      if (localDamage > 0.f) {
        ApplyLocalDamage(position, direction, *damagee, localDamage, damagerId, owner, info, 0);
      }
      SendDamageScriptMsgs(*damagee, damagerId, info);
      SendScriptMsg(damagee, damagerId, kSM_Damage);
    } else {
      damagee->SendScriptMsgs(kSS_ResistedDamage, *this, kInvalidUniqueId, kSM_None);
      SendScriptMsg(damagee, damagerId, kSM_ResistedDamage);
    }

    float knockbackX = knockbackDirection.GetX();
    float knockbackY = knockbackDirection.GetY();
    if (damager != nullptr && info.GetKnockBackPower(*vulnerability, 0.f) > 0.f) {
      const CVector3f defaultDirection =
          damagee->GetTransform().GetTranslation() - damager->GetTransform().GetTranslation();
      const CVector3f& useDirection =
          knockbackDirection.IsNonZero() ? knockbackDirection : defaultDirection;
      knockbackX = useDirection.GetX();
      knockbackY = useDirection.GetY();
    }
    const CVector3f knockback(knockbackX, knockbackY, 0.0001f);
    ApplyKnockBack(*damagee, damagerId, owner, info, *vulnerability, knockback.AsNormalized(), 0.f);
  }

  if (damager != nullptr && info.GetRadius() > 0.f) {
    ProcessRadiusDamage(*damager, *damagee, owner, info, filter);
  }
  CSwarmBasics* swarm = TCastToPtr< CSwarmBasics >(damageeEntity);
  if (swarm != nullptr && damager != nullptr) {
    swarm->ApplyRadiusDamage(damager->GetTranslation(), info, *this);
  }
}

void CStateManager::KillPlayer(float previousHealth, TUniqueId victim, TUniqueId killer) {
  CPlayer* player = TCastToPtr< CPlayer >(ObjectById(victim));
  if (player != nullptr) {
    PlayerState(MaskUIdNumPlayers(victim))->SetPlayerAlive(false);

    if (previousHealth >= 0.f) {
      const CGameState& gameState = *gpGameState;
      CGameMode& gameMode = gameState.GetGameMode();
      gameMode.OnPlayerKilled(*this, victim, killer);
    }

    if (!IsMultiplayer()) {
      CSfxManager::KillAll(CSfxManager::kSC_Game);
      CStreamAudioManager::FadeOutSoftwareAudio(CStreamAudioManager::kSC_Default, 0.5f);
    }
  }
}

CDamageInfo CStateManager::GetModifiedDamageInfo(TUniqueId damager, TUniqueId owner,
                                                 TUniqueId damagee,
                                                 const CDamageInfo& damage) const {
  const CPatterned* patterned = TCastToConstPtr< CPatterned >(GetObjectById(damager));
  if (patterned == nullptr) {
    patterned = TCastToConstPtr< CPatterned >(GetObjectById(owner));
  }

  if (patterned != nullptr) {
    if (patterned->IsIngPossessed()) {
      CDamageInfo result = damage;
      result.SetDamage(damage.GetDamage() * patterned->GetIngPossessedDamageMultiplier());
      return result;
    }
    return damage;
  }

  if (gpGameState->GetHardModeEnabled() &&
      TCastToConstPtr< CPlayer >(GetObjectById(owner)) != nullptr) {
    bool aiDamage = false;
    if (TCastToConstPtr< CPatterned >(GetObjectById(damagee)) != nullptr) {
      aiDamage = true;
    } else if (const CCollisionActor* collision =
                   TCastToConstPtr< CCollisionActor >(GetObjectById(damagee))) {
      if (TCastToConstPtr< CPatterned >(GetObjectById(collision->GetOwnerId())) != nullptr) {
        aiDamage = true;
      }
    }
    if (aiDamage) {
      return NGunUtils::DifficultyModifyDamageInfo(damage);
    }
  }
  return damage;
}

void CStateManager::RecordDamageSource(CActor& damagee, TUniqueId source, const CDamageInfo& info,
                                       bool lethal, bool radiusDamage) {
  CHealthInfo* const health = damagee.HealthInfo();
  if (health == nullptr) {
    return;
  }

  bool frozen = false;
  if (lethal) {
    CPatterned* const patterned = TCastToPtr< CPatterned >(&damagee);
    if (patterned != nullptr) {
      frozen = patterned->BodyController()->IsFrozen();
    } else {
      CPlayer* const player = TCastToPtr< CPlayer >(&damagee);
      if (player != nullptr) {
        frozen = player->GetFrozenState();
      }
    }
  }

  const CWeapon* const weapon = TCastToConstPtr< CWeapon >(GetObjectById(source));
  if (weapon != nullptr) {
    health->SetLastDamageWeapon(weapon->GetCurrentDamageInfo().GetWeaponMode(),
                                weapon->GetOwnerId(), source, radiusDamage);
    if (lethal) {
      health->SetCauseOfDeathWeapon(weapon->GetCurrentDamageInfo().GetWeaponMode(),
                                    weapon->GetOwnerId(), source, frozen, radiusDamage);
    }
  } else {
    health->SetLastDamageWeapon(info.GetWeaponMode(), source, source, radiusDamage);
    if (lethal) {
      health->SetCauseOfDeathWeapon(info.GetWeaponMode(), source, source, frozen, radiusDamage);
    }
  }
}

bool CStateManager::ApplyLocalDamage(const CVector3f& pos, const CVector3f& dir, CActor& damagee,
                                     float damage, TUniqueId source, TUniqueId owner,
                                     const CDamageInfo& damageInfo, bool radiusDamage) {
  CHealthInfo* healthInfo = damagee.HealthInfo();
  if (!healthInfo || damage < 0.0f) {
    return false;
  }

  const float oldHp = healthInfo->GetHP();
  float hp = oldHp;
  if (oldHp <= 0.0f) {
    RecordDamageSource(damagee, source, damageInfo, false, radiusDamage);
    return true;
  }

  float useDamage = damage;
  CPlayer* player = TCastToPtr< CPlayer >(damagee);
  CPatterned* patterned = TCastToPtr< CPatterned >(damagee);
  CScriptDoor* door = TCastToPtr< CScriptDoor >(damagee);

  if (player && player->GetTurretState() != CPlayer::kTS_None) {
    if (player->GetTurretState() != CPlayer::kTS_Active) {
      return false;
    }
    // These inherited turret helpers forward the damage position and test its direction.
    // Their complete interfaces remain unresolved; see the radius-damage research notes.
    player->fn_8000d3ac(pos, *this);
    if (!player->fn_8000d40c(dir, *this)) {
      return false;
    }
  }
  TUniqueId playerId = player ? player->GetUniqueId() : kInvalidUniqueId;
  if (player) {
    int playerIndex = MaskUIdNumPlayers(playerId);
    CPlayerState& playerState = *PlayerState(playerIndex);

    if (GetCameraManager(playerIndex)->IsInCinematicCamera()) {
      return false;
    }

    if (gpGameState->GetHardModeEnabled()) {
      switch (damageInfo.GetWeaponMode().GetRawType()) {
      case kWT_Phazon:
      case kWT_PoisonWater1:
      case kWT_PoisonWater2:
      case kWT_Lava:
      case kWT_Heat:
      case kWT_Unused1:
      case kWT_AreaDark:
      case kWT_AreaLight:
        break;
      default:
        useDamage *= gpGameState->GetHardModeDamageMultiplier();
        break;
      }
    }

    float damageReduction = 0.0f;
    const EWeaponType weaponType = EWeaponType(damageInfo.GetWeaponMode().GetRawType());
    const bool lightDamage = weaponType == kWT_Light;
    const bool darkDamage = weaponType == kWT_Dark;

    if (playerState.HasPowerUp(CPlayerState::kIT_VariaSuit)) {
      damageReduction = player->GetTweakPlayer()->GetVariaSuitDamageReduction();
    }
    if (playerState.HasPowerUp(CPlayerState::kIT_DarkSuit)) {
      float reduction = player->GetTweakPlayer()->GetDarkSuitDamageReduction();
      damageReduction = rstl::max_val(reduction, damageReduction);
    }
    if (playerState.HasPowerUp(CPlayerState::kIT_LightSuit)) {
      float reduction = player->GetTweakPlayer()->GetLightSuitDamageReduction();
      damageReduction = rstl::max_val(reduction, damageReduction);
    }
    if (playerState.GetItemAmount(CPlayerState::kIT_AbsorbAttack, true) != 0) {
      float reduction = 1.5f;
      damageReduction = rstl::max_val(reduction, damageReduction);
    }
    if (playerState.GetItemAmount(CPlayerState::kIT_LightShield, true) != 0 && !darkDamage) {
      float reduction = 0.75f;
      damageReduction = rstl::max_val(reduction, damageReduction);
    }
    if (playerState.GetItemAmount(CPlayerState::kIT_DarkShield, true) != 0 && !lightDamage) {
      float reduction = 0.75f;
      damageReduction = rstl::max_val(reduction, damageReduction);
    }
    hp = playerState.CalculateHealth();
    useDamage = -(damageReduction * useDamage - useDamage);
  }

  const float damagedHp = oldHp - useDamage;
  if (damagedHp < hp) {
    hp = damagedHp;
  }
  healthInfo->SetHP(hp);
  const bool significant = hp < oldHp;
  RecordDamageSource(damagee, source, damageInfo, hp <= 0.f, radiusDamage);

  if (player != nullptr) {
    if (damageInfo.GetWeaponMode().IsInstantKill()) {
      useDamage = hp;
      hp = 0.f;
      healthInfo->SetHP(hp);
    }
    // The native caller snapshots the complete damage record before player reactions.
    player->TakeDamage(significant, pos, useDamage, source, owner, CDamageInfo(damageInfo), *this);
    const CGameState& gameState = *gpGameState;
    gameState.GetGameMode().OnPlayerDamaged(*this, playerId, owner, useDamage);
    if (hp <= 0.f) {
      KillPlayer(oldHp, playerId, owner);
    }
  } else if (patterned != nullptr) {
    if (significant) {
      patterned->TakeDamage(dir, useDamage);
      hp = patterned->GetHealthInfo()->GetHP();
    }
    if (hp <= 0.f) {
      patterned->Death(*this, dir, kSS_DeathRattle);
    }
  } else if (door != nullptr && significant && hp <= 0.f) {
    door->SetBurnOrigin(pos);
  }
  return significant;
}

void CStateManager::TestBombHittingWater(const CActor& source, const CVector3f& position,
                                         CActor& damagee) {
  int index = 0;
  if (const CWeapon* weapon = TCastToConstPtr< CWeapon >(source)) {
    const int attributes = weapon->GetAttribField();
    if ((attributes & (CWeapon::kPA_TriggerBomb | CWeapon::kPA_PowerBombs)) != 0) {
      if ((attributes & CWeapon::kPA_PowerBombs) != 0) {
        index = 1;
      }
      if (CScriptWater* const water = TCastToPtr< CScriptWater >(damagee)) {
        const CVector3f hitPosition(position.GetX(), position.GetY(),
                                    water->GetTriggerBoundsWR().GetMaxPoint().GetZ());
        const float depth = -water->GetWRSurfacePlane().GetHeight(position);
        if (depth <= skBombUnderwaterRanges[index] && depth > 0.f) {
          const float splashFactor = 1.f - depth / skBombUnderwaterRanges[index];
          if (index == 0) {
            mFluidPlaneManager->CreateSplash(source.GetUniqueId(), *this, *water, hitPosition,
                                             splashFactor, true);
          }
        }
      }
    }
  }
}

const bool CStateManager::MultiRayCollideWorld(const CMRay& ray,
                                               const CMaterialFilter& filter) const {
  CVector3f offset2 =
      CVector3f(ray.GetDirection().GetY(), -ray.GetDirection().GetZ(), ray.GetDirection().GetX());
  CVector3f offset = CVector3f::Cross(offset2, ray.GetDirection()).AsNormalized();
  offset2 = 0.35355338f * CVector3f::Cross(ray.GetDirection(), offset);
  offset *= 0.35355338f;

  bool visible = false;
  for (int i = 0; i < 4; ++i) {
    const CVector3f start =
        ray.GetStart() + ((i & 1) ? offset : -offset) + ((i & 2) ? -offset2 : offset2);
    visible = CGameCollision::RayStaticLineOfSightTest(*this, start, ray.GetDirection(),
                                                       ray.GetLength(), filter);
    if (visible) {
      break;
    }
  }
  return visible;
}

const bool
CStateManager::TestRayDamage(const CVector3f& position, const CActor& damagee,
                             const rstl::reserved_vector< TUniqueId, 1024 >& nearList) const {
  if (damagee.GetHealthInfo() == nullptr) {
    return false;
  }

  // Material 59's semantic name remains unresolved; the native filter uses it,
  // rather than Prime's Solid material, and excludes NoPlatformCollision.
  static const CMaterialList include = CMaterialList(kMT_Unknown59);
  static const CMaterialList exclude =
      CMaterialList(kMT_NoPlatformCollision, kMT_Player, kMT_Occluder, kMT_Character);
  static const CMaterialFilter filter =
      CMaterialFilter(include, exclude, CMaterialFilter::kFT_IncludeExclude);

  const rstl::optional_object< CAABox > bounds = damagee.GetTouchBounds();
  if (!bounds) {
    return false;
  }

  const CVector3f center = bounds->GetCenterPoint();
  CVector3f direction = center - position;
  if (direction.CanBeNormalized()) {
    const float length = direction.Magnitude();
    direction *= 1.f / length;
    if (RayCollideWorld(position, center, nearList, filter, &damagee)) {
      return true;
    }

    const CMRay ray = CMRay(position, direction, length);
    if (!MultiRayCollideWorld(ray, filter)) {
      return false;
    }

    float depth;
    CVector3f normal = CVector3f::Zero();
    const int count = CollisionUtil::RayAABoxIntersection(ray, *bounds, normal, depth);
    if (count == 0) {
      return true;
    }
    if (count == 1) {
      return true;
    }
    return CGameCollision::RayDynamicLineOfSightTest(*this, position, direction, depth * length,
                                                     filter, nearList, &damagee);
  }
  return true;
}

void CStateManager::ApplyRadiusDamage(const CActor& source, const CVector3f& position,
                                      CActor& damagee, TUniqueId owner, const CDamageInfo& damage) {
  const CDamageInfo info(
      GetModifiedDamageInfo(source.GetUniqueId(), owner, damagee.GetUniqueId(), damage));
  CVector3f delta = damagee.GetTranslation() - position;
  if (!(delta.MagSquared() < info.GetRadius() * info.GetRadius())) {
    if (!damagee.GetTouchBounds()) {
      return;
    }
    if (!CCollidableSphere::Sphere_AABox_Bool(CSphere(position, info.GetRadius()),
                                              *damagee.GetTouchBounds())) {
      return;
    }
  }

  float radius = info.GetRadius();
  radius = radius > FLT_EPSILON ? delta.Magnitude() / radius : 0.f;
  radius = rstl::min_val(radius, 1.f);
  if (radius > 0.f) {
    delta.Normalize();
  }

  const CDamageVulnerability* vulnerability =
      radius > 0.f ? damagee.GetDamageVulnerability(position, delta, info)
                   : damagee.GetDamageVulnerability();
  if (vulnerability->WeaponHits(info.GetWeaponMode(), 1)) {
    const float localDamage = info.GetRadiusDamage(*vulnerability);
    if (localDamage > 0.f) {
      ApplyLocalDamage(position, delta, damagee, localDamage, source.GetUniqueId(), owner, info, 1);
    }
    SendDamageScriptMsgs(damagee, source.GetUniqueId(), info);
    SendScriptMsg(&damagee, source.GetUniqueId(), kSM_Damage, kInvalidUniqueId);
  } else {
    damagee.SendScriptMsgs(kSS_ResistedDamage, *this, kInvalidUniqueId, kSM_None);
    SendScriptMsg(&damagee, source.GetUniqueId(), kSM_ResistedDamage, kInvalidUniqueId);
  }

  const CVector3f knockbackDelta =
      damagee.GetTransform().GetTranslation() - source.GetTransform().GetTranslation();
  const CVector3f knockbackDirection(knockbackDelta.GetX(), knockbackDelta.GetY(), 0.0001f);
  ApplyKnockBack(damagee, source.GetUniqueId(), owner, info, *vulnerability,
                 knockbackDirection.AsNormalized(), radius);
}

void CStateManager::ProcessRadiusDamage(const CActor& source, CActor& damagee, TUniqueId owner,
                                        const CDamageInfo& damage, const CMaterialFilter& filter) {
  CMaterialFilter localFilter = filter;
  const TUniqueId sourceId = source.GetUniqueId();
  const TUniqueId damageeId = damagee.GetUniqueId();
  const float radius = damage.GetRadius();
  const CVector3f position(source.GetTranslation());
  const float negativeRadius = -radius;
  const CAABox bounds(position + CVector3f(negativeRadius, negativeRadius, negativeRadius),
                      position + CVector3f(radius, radius, radius));

  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  BuildNearList(nearList, bounds, localFilter, nullptr);
  for (rstl::reserved_vector< TUniqueId, 1024 >::iterator it = nearList.begin();
       it != nearList.end(); ++it) {
    CActor* actor = static_cast< CActor* >(ObjectById(*it));
    if (actor != nullptr) {
      const TUniqueId actorId = actor->GetUniqueId();
      if (sourceId != actorId && owner != actorId && damageeId != actorId) {
        TestBombHittingWater(source, position, *actor);
        if (TestRayDamage(position, *actor, nearList)) {
          ApplyRadiusDamage(source, position, *actor, owner, damage);
        }
      }
    }
  }
}

void CStateManager::ApplyDamageToWorld(TUniqueId owner, CActor& projectile,
                                       const CVector3f& position, const CDamageInfo& info,
                                       const CMaterialFilter& filter) {
  const CMaterialFilter useFilter = filter;
  const float radius = info.GetRadius();
  const float negativeRadius = -radius;
  const CAABox bounds(position + CVector3f(negativeRadius, negativeRadius, negativeRadius),
                      position + CVector3f(radius, radius, radius));

  const CWeapon* const weapon = TCastToConstPtr< CWeapon >(&projectile);
  bool bomb = false;
  if (weapon != nullptr) {
    bomb =
        weapon->HasAttrib(CWeapon::kPA_TriggerBomb) || weapon->HasAttrib(CWeapon::kPA_PowerBombs);
  }

  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  BuildNearList(nearList, bounds, useFilter, &projectile);
  for (rstl::reserved_vector< TUniqueId, 1024 >::iterator it = nearList.begin();
       it != nearList.end(); ++it) {
    CActor* const actor = static_cast< CActor* >(ObjectById(*it));
    CPlayer* const player = TCastToPtr< CPlayer >(actor);
    CEntity* const snakeWeed = CastToSnakeWeedSwarm(actor);
    CSwarmBasics* const swarm = TCastToPtr< CSwarmBasics >(actor);

    if (bomb && player != nullptr && actor->GetUniqueId() == weapon->GetOwnerId()) {
      if (player->GetFrozenState()) {
        CEnvironmentVariable* const freezeInstructions =
            gpGameState->SystemOptions().FindEnvironmentVariable("FreezeInstructionsMorphBall");
        freezeInstructions->Set(freezeInstructions->GetValue() + 1);
        const int playerIndex = MaskUIdNumPlayers(player->GetUniqueId());
        CSamusHud::DisplayHudMemo(rstl::wstring_l(L""),
                                  CHUDMemoParms(0.f, true, true, true, 1 << playerIndex, true));
        player->BreakFrozenState(*this, CPlayer::kBFS_BreakWithEffects, false);
      } else if (weapon->HasAttrib(CWeapon::kPA_TriggerBomb)) {
        player->BombJump(position, *this);
      }
    } else if (actor != nullptr && actor->GetUniqueId() != owner) {
      TestBombHittingWater(projectile, position, *actor);
      if (TestRayDamage(position, *actor, nearList)) {
        ApplyRadiusDamage(projectile, position, *actor, owner, info);
      }
    }

    if (snakeWeed != nullptr) {
      SnakeWeed_ApplyRadiusDamage(*snakeWeed, position, info, *this);
    }
    if (swarm != nullptr) {
      swarm->ApplyRadiusDamage(position, info, *this);
    }
  }
}

void CStateManager::ApplyKnockBack(CActor& actor, TUniqueId source, TUniqueId owner,
                                   const CDamageInfo& damage,
                                   const CDamageVulnerability& vulnerability,
                                   const CVector3f& direction, float dampen) {
  const CWeaponTypeVulnerability weaponVulnerability =
      vulnerability.GetVulnerability(damage.GetWeaponMode());
  if (!weaponVulnerability.WeaponHurts()) {
    return;
  }

  const CHealthInfo* health = actor.GetHealthInfo();
  if (health == nullptr) {
    return;
  }

  const float power = (1.f - dampen) * damage.GetKnockBackPower();
  const float resistance = health->GetKnockBackResistance();
  CPlayer* player = TCastToPtr< CPlayer >(actor);
  CPatterned* patterned = TCastToPtr< CPatterned >(actor);
  const bool alive = health->GetHP() > 0.f;
  const CKnockBackInfo info(direction, source, owner, damage, dampen == 0.f);
  if (player != nullptr) {
    player->GetKnockBackManager().KnockBack(*this, *player, info);
    return;
  }

  if (patterned == nullptr && !alive) {
    if (power > resistance) {
      if (CPhysicsActor* const physics = TCastToPtr< CPhysicsActor >(actor)) {
        const CVector3f impulse = direction * (1.5f * ((power - resistance) * physics->GetMass()));
        // The native impulse gate uses material 59; its semantic name is unresolved.
        if (!physics->GetMaterialList().HasMaterial(kMT_Immovable) &&
            physics->GetMaterialList().HasMaterial(kMT_Unknown59)) {
          physics->ApplyImpulseWR(impulse, CAxisAngle::Identity());
        }
      }
    }
  } else if (patterned != nullptr) {
    patterned->KnockBack(*this, info);
  }
}

void CStateManager::InformListeners(const CVector3f& position, EListenNoiseType type) {
  CObjectList* list = mObjectLists[kOL_ListeningAi].get();
  for (int i = list->GetFirstObjectIndex(); i != -1; i = list->GetNextObjectIndex(i)) {
    CPatterned* patterned = TCastToPtr< CPatterned >((*list)[i]);
    if (patterned != nullptr && patterned->GetActive()) {
      CGameArea* area = mWorld->Area(patterned->GetCurrentAreaId());
      if (area->GetOcclusionState() != CGameArea::kOS_Occluded) {
        patterned->Listen(*this, position, type);
      }
    }
  }
}

CStateManager::TIdListResult CStateManager::GetIdListForScript(TEditorId editorId) const {
  const TIdListResult range = mScriptIdMap.equal_range(editorId);
  return range;
}

TUniqueId CStateManager::GetIdForScript(TEditorId editorId) const {
  TIdList::const_iterator it = mScriptIdMap.find(editorId);
  if (it != mScriptIdMap.end()) {
    return it->second;
  }
  return kInvalidUniqueId;
}

TEditorId CStateManager::GetEditorIdForUniqueId(TUniqueId uid) const {
  const CEntity* entity = GetObjectById(uid);
  if (entity != nullptr) {
    return entity->GetEditorId();
  }
  return kInvalidEditorId;
}

void CStateManager::EndPlayerRender() {
  mCurrentRenderPlayerIndex = kInvalidRenderPlayerIndex;
  mCurrentRenderPlayer = nullptr;
  mPlayerState = nullptr;
  mCameraManager = nullptr;
}

void CStateManager::AddToGraveyard(CEntity* entity) {
  if (mGraveyard.empty()) {
    rstl::reserved_vector< CEntity*, 32 > batch;
    mGraveyard.push_back(batch);
  } else if (mGraveyard.back().size() == mGraveyard.back().capacity()) {
    rstl::reserved_vector< CEntity*, 32 > batch;
    mGraveyard.push_back(batch);
  }

  mGraveyard.back().push_back(entity);
}

void CStateManager::DispatchScriptMessages() {
  while (!mScriptMsgs.empty()) {
    CScriptMsg msg = mScriptMsgs.Dequeue();
    CEntity* ent = ObjectById(msg.GetId());
    if (ent) {
      const bool wasActive = ent->GetActive();
      ent->AcceptScriptMsg(*this, msg);
      if (wasActive != ent->GetActive()) {
        if (CActor* actor = TCastToPtr< CActor >(ent)) {
          UpdateActorInSortedLists(actor);
        }
      }

      if (msg.GetMessage() == kSM_Delete) {
        AddToGraveyard(ent);
        RemoveObject(ent->GetUniqueId());
      }
    }
  }
}

void CStateManager::ThinkNewObjects(float dt) {
  for (;;) {
    DispatchScriptMessages();
    if (mNewObjectIds.empty()) {
      return;
    }

    rstl::list< TUniqueId >::iterator it = mNewObjectIds.begin();
    while (it != mNewObjectIds.end()) {
      if (CEntity* entity = ObjectById(*it)) {
        ThinkEntity(dt, *entity);
      }
      it = mNewObjectIds.erase(it);
    }
  }
}

void CStateManager::DeferStateTransition(EStateManagerTransition t) {
  if (!IsMultiplayer()) {
    if (t == kSMT_InGame) {
      if (mDeferredTransition != kSMT_InGame) {
        mWorld->SetLoadPauseState(false);
        mDeferredTransition = kSMT_InGame;
      }
    } else if (mDeferredTransition == kSMT_InGame) {
      mWorld->SetLoadPauseState(true);
      mDeferredTransition = t;
      if (mDeferredTransition == kSMT_SaveGame) {
        mSaveGameScreen = rs_new CSaveGameScreen(kSC_InGame, gpGameState->GetCardSerial());
      }
    }
  }
}

void CStateManager::ShowPausedHUDMemo(CAssetId strg, float time) {
  mHudMessageTime = time;
  mPauseHudMessage = strg;
  DeferStateTransition(kSMT_MessageScreen);
}

void CStateManager::SendScriptMsg(const CScriptMsg& msg) {
  mScriptMsgs.Append(msg);
  int v = mScriptMsgs.GetCount();
  if (0x80 < v && !mDispatchingScriptMessages) {
    mDispatchingScriptMessages = true;
    DispatchScriptMessages();
    mDispatchingScriptMessages = false;
  }
}

bool CStateManager::IsMultiplayer() const {
  int v = gpGameState->GetGameMode().GetGameModeType();
  return v != 'SNGL' && v != 'FRND';
}

void CStateManager::MovePlatforms(float dt) {
  CObjectList* platformList = mObjectLists[kOL_Platform].get();
  for (int i = platformList->GetFirstObjectIndex(); i != -1;
       i = platformList->GetNextObjectIndex(i)) {
    CPhysicsActor* actor = static_cast< CPhysicsActor* >((*platformList)[i]);
    if (actor != nullptr && actor->GetActive() && actor->GetMass() != 0.f) {
      CGameCollision::Move(*this, *actor, dt, nullptr);
    }
  }
}

void CStateManager::MoveActors(float dt) {
  CObjectList* physicsList = mObjectLists[kOL_PhysicsActor].get();
  for (int i = physicsList->GetFirstObjectIndex(); i != -1;
       i = physicsList->GetNextObjectIndex(i)) {
    CPhysicsActor* actor = static_cast< CPhysicsActor* >((*physicsList)[i]);
    if (actor == nullptr || !actor->GetActive() || actor->GetMass() == 0.f ||
        (!actor->GetUpdateDuringCinematicSkip() && gpMain->IsMaxSpeed())) {
      continue;
    }

    if (!actor->GetUpdateWhileOccluded() && actor->GetCurrentAreaId() != kInvalidAreaId) {
      const CGameArea& area = mWorld->GetAreaAlways(actor->GetCurrentAreaId());
      const float occludedTime = area.IsLoaded() ? area.GetPostConstructed()->mOccludedTime : 0.f;
      if (occludedTime > 5.f) {
        continue;
      }
    }

    CPatterned* patterned = TCastToPtr< CPatterned >(actor);
    if (patterned != nullptr && !ShouldUpdatePatterned(*patterned)) {
      SendScriptMsg(patterned->GetUniqueId(), kInvalidUniqueId, kSM_AIUpdateDisabled,
                    kInvalidUniqueId);
      continue;
    }

    if (TCastToPtr< CPlayer >(actor) == nullptr &&
        TCastToPtr< CScriptPlatform >(actor) == nullptr) {
      CGameCollision::Move(*this, *actor, dt, nullptr);
    }
  }
}

void CStateManager::CrossTouchActors() {
  CObjectList* actorList = mObjectLists[kOL_Actor].get();
  bool visits[kMaxObjects];
  memset(visits, 0, sizeof(visits));

  for (int i = actorList->GetFirstObjectIndex(); i != -1; i = actorList->GetNextObjectIndex(i)) {
    CActor* actor = static_cast< CActor* >((*actorList)[i]);
    if (actor != nullptr && actor->GetActive() && actor->GetCallTouch()) {
      const rstl::optional_object< CAABox > touchBounds = actor->GetTouchBounds();
      if (!touchBounds) {
        continue;
      }
      if (!actor->GetUpdateDuringCinematicSkip() && gpMain->IsMaxSpeed()) {
        continue;
      }

      rstl::reserved_vector< TUniqueId, kMaxObjects > nearList;
      const CMaterialFilter filter = actor->GetMaterialList().HasMaterial(kMT_Trigger) &&
                                             TCastToPtr< CMetroidAlpha >(actor) == nullptr
                                         ? CMaterialFilter::MakeExclude(CMaterialList(kMT_Trigger))
                                         : CMaterialFilter::GetPassEverything();
      BuildNearList(nearList, *touchBounds, filter, actor);

      for (const TUniqueId* uid = nearList.begin(); uid != nearList.end(); ++uid) {
        CActor* other = static_cast< CActor* >(ObjectById(*uid));
        if (other != nullptr) {
          const rstl::optional_object< CAABox > otherBounds = other->GetTouchBounds();
          if (!other->GetActive() || !otherBounds) {
            continue;
          }

          if (!visits[other->GetUniqueId().Value()]) {
            if (touchBounds->DoBoundsOverlap(*otherBounds)) {
              actor->Touch(*other, *this);
              other->Touch(*actor, *this);
            }
            visits[actor->GetUniqueId().Value()] = true;
          }
        }
      }
    }
  }
}

void CStateManager::ThinkEntity(float dt, CEntity& entity) { entity.Think(dt, *this); }

bool CStateManager::ShouldUpdatePatterned(const CPatterned& actor) {
  bool update = !mCinematicPause;
  if (update && actor.GetCurrentAreaId() != kInvalidAreaId) {
    const CGameArea& area = mWorld->GetAreaAlways(actor.GetCurrentAreaId());
    const float occludedTime = area.IsLoaded() ? area.GetPostConstructed()->mOccludedTime : 0.f;
    if (occludedTime > 5.f) {
      update = false;
    }
  }
  return update;
}

void CStateManager::Think(float dt) {
  if (!IsMultiplayer() && mPlayers[0]->GetDeathTime() > 0.f) {
    mPlayers[0]->DoThink(dt, *this);
    return;
  }

  CObjectList* allList = mObjectLists[kOL_All].get();
  if (mGameState == kGS_SoftPaused) {
    for (int i = allList->GetFirstObjectIndex(); i != -1; i = allList->GetNextObjectIndex(i)) {
      CScriptEffect* effect = TCastToPtr< CScriptEffect >((*allList)[i]);
      if (effect != nullptr) {
        effect->Think(dt, *this);
      }
    }
  } else {
    for (long i = allList->GetFirstObjectIndex(); i != -1; i = allList->GetNextObjectIndex(i)) {
      CEntity* entity = (*allList)[i];
      if (entity == nullptr || (!entity->GetUpdateDuringCinematicSkip() && gpMain->IsMaxSpeed())) {
        continue;
      }

      if (!entity->GetUpdateWhileOccluded() && entity->GetCurrentAreaId() != kInvalidAreaId) {
        const CGameArea& area = mWorld->GetAreaAlways(entity->GetCurrentAreaId());
        const float occludedTime = area.IsLoaded() ? area.GetPostConstructed()->mOccludedTime : 0.f;
        if (occludedTime > 5.f) {
          continue;
        }
      }

      CPatterned* patterned = TCastToPtr< CPatterned >((*allList)[i]);
      if (patterned != nullptr && !ShouldUpdatePatterned(*patterned)) {
        continue;
      }
      if (TCastToPtr< CGameCamera >(entity) == nullptr) {
        ThinkEntity(dt, *entity);
      }
    }
  }
}

void CStateManager::PreThinkObjects(float dt) {
  if (!IsMultiplayer() && mPlayers[0]->GetDeathTime() > 0.f) {
    mPlayers[0]->DoPreThink(dt, *this);
    return;
  }

  CObjectList* allList = mObjectLists[kOL_All].get();
  if (mGameState == kGS_SoftPaused) {
    for (int i = allList->GetFirstObjectIndex(); i != -1; i = allList->GetNextObjectIndex(i)) {
      CScriptEffect* effect = TCastToPtr< CScriptEffect >((*allList)[i]);
      if (effect != nullptr) {
        effect->PreThink(dt, *this);
      }
    }
  } else {
    for (int i = allList->GetFirstObjectIndex(); i != -1; i = allList->GetNextObjectIndex(i)) {
      CEntity* entity = (*allList)[i];
      if (entity != nullptr && TCastToPtr< CGameCamera >(entity) == nullptr) {
        entity->PreThink(dt, *this);
      }
    }
  }
}

void CStateManager::PostUpdatePlayer(float dt) {
  for (uint i = 0; i < mNumPlayers; ++i) {
    mPlayers[i]->PostUpdate(dt, *this);
  }
}

void CStateManager::Update(float inputDt, CArchitectureQueue& queue) {
  mArchQueue = &queue;
  float dt = inputDt;
  float cameraDt = inputDt;
  if (mCameraManagers[0]->IsInCinematicCamera()) {
    const CCinematicCamera* camera = mCameraManagers[0]->GetCinematicCamera();
    if ((camera->GetFlags() & 0x200) != 0) {
      dt *= camera->GetSlowMotionScale();
    }
    if (gpMain->IsMaxSpeed()) {
      dt *= 2.f;
      cameraDt *= 2.f;
    }
  }

  CElementGen::SetGlobalSeed(mUpdateFrameIdx);
  CParticleElectric::SetGlobalSeed(mUpdateFrameIdx);
  CParticleSpawnSystem::SetGlobalSeed(mUpdateFrameIdx);
  CSortedParticleSystem::SetGlobalSeed(mUpdateFrameIdx);
  CDecal::SetGlobalSeed(mUpdateFrameIdx);
  CProjectileWeapon::SetGlobalSeed(mUpdateFrameIdx);
  mCurTimeMod900 += dt;
  if (mCurTimeMod900 > 900.f) {
    mCurTimeMod900 -= 900.f;
  }
  mPauseHudMessage = kInvalidAssetId;
  mLightAmmoDepletedPlayers = 0;
  mDarkAmmoDepletedPlayers = 0;
  CScriptEffect::ResetParticleCounts();
  fn_8003FF1C(this);
  fn_8003FF20(this);

  const bool playerDead = !IsMultiplayer() && mPlayers[0]->GetDeathTime() > 0.f;
  if (mGameState == kGS_Running) {
    if (!IsMultiplayer() && !mCameraManagers[0]->IsInCinematicCamera()) {
      gpGameState->SetTotalPlayTime(dt + gpGameState->GetTotalPlayTime());
      UpdateHintState(dt);
    }

    for (int player = 0; player < 4; ++player) {
      for (int pass = 0; pass < 11; ++pass) {
        mCameraFilterPasses[player][pass].Update(dt);
        mCameraBlurPasses[player][pass].Update(dt);
      }
    }

    for (int item = 0; item < CPlayerState::kIT_Max; ++item) {
      for (uint player = 0; player < mNumPlayers; ++player) {
        const CPlayerState::EItemType type = static_cast< CPlayerState::EItemType >(item);
        CPlayerState::CPowerUp& powerUp = mPlayerStates[player]->PowerUp(type);
        if (powerUp.mTimeLeft > 0.f) {
          powerUp.mTimeLeft -= dt;
          if (powerUp.mTimeLeft < 0.f) {
            powerUp.mTimeLeft = 0.f;
            powerUp.mAmount = 0;
            if (item >= CPlayerState::kIT_SuperMissile && item <= CPlayerState::kIT_SonicBoom) {
              powerUp.mCapacity = 0;
            }
            switch (type) {
            case CPlayerState::kIT_SwitchVisorCombat:
            case CPlayerState::kIT_SwitchVisorScan:
            case CPlayerState::kIT_SwitchVisorDark:
            case CPlayerState::kIT_SwitchVisorEcho:
              mPlayerStates[player]->StartTransitionToVisor(CPlayerState::kPV_Combat);
              break;
            default:
              break;
            }
            if (item == CPlayerState::kIT_ScanVirus) {
              mPlayerStates[player]->StartTransitionToVisor(CPlayerState::kPV_Combat);
              mPlayerStates[player]->ReInitializePowerUp(CPlayerState::kIT_ScanVisor, 0);
            }
            DisplayAlertAboutOutOfAmmo(*mPlayers[player], type);
          }
        }
      }
    }
    mSafeZoneManager->Update(dt, *this);
  }

  if (mGameState != kGS_Paused && dt > FLT_EPSILON) {
    PreThinkObjects(dt);
    mFluidPlaneManager->Update(dt);
  }
  if (mGameState == kGS_Running) {
    if (!playerDead) {
      CDecalManager::Update(dt, *this);
    }
    UpdateSortedLists();
    if (dt > FLT_EPSILON && !playerDead) {
      MovePlatforms(dt);
      MoveActors(dt);
    }
    UpdatePlayerLineOfSight(dt);
    ProcessPlayerInput();
    if (mGameState != kGS_SoftPaused) {
      for (uint player = 0; player < mNumPlayers; ++player) {
        CGameCollision::Move(*this, *mPlayers[player], dt, nullptr);
      }
    }
    UpdateSortedLists();
    if (!playerDead) {
      CrossTouchActors();
    }
  } else {
    ProcessPlayerInput();
  }
  if (!playerDead && mGameState == kGS_Running) {
    mActorModelParticles->Update(dt, *this);
  }
  if ((mGameState == kGS_Running || mGameState == kGS_SoftPaused) && dt > FLT_EPSILON) {
    Think(dt);
  }

  if (mPausedHudMemoFrameCount == mHudMessageFrameCount) {
    ShowPausedHUDMemo(mPausedHudMemoAssetId, mQueuedHudMemoDismissalDelay);
    --mPausedHudMemoFrameCount;
    mPausedHudMemoAssetId = kInvalidAssetId;
  }
  if (!playerDead && mGameState == kGS_Running && !IsMultiplayer() &&
      !mCameraManagers[0]->IsInCinematicCamera()) {
    UpdateEscapeSequenceTimer(dt);
  }
  mWorld->Update(dt);
  UpdateDynamicLayers();
  for (uint player = 0; player < mNumPlayers; ++player) {
    mRumbleManagers[player]->Update(dt);
  }
  if (!playerDead) {
    mEnvFxManager->Update(dt, *this);
  }
  UpdateAreaSounds();
  mWorld->Area(GetNextAreaId())->UpdateDocks(*this);
  mReadyToRender = true;

  if (mInMapScreen) {
    CHintOptions& hintOptions = gpGameState->HintOptions();
    const CHintOptions::SHintState* hint = hintOptions.GetCurrentDisplayedHint();
    if (hint != nullptr && hint->CanContinue()) {
      hintOptions.DismissDisplayedHint();
    }
    mInMapScreen = false;
  }
  const CGameState& gameState = *gpGameState;
  gameState.GetGameMode().Update(dt, *this);
  ThinkNewObjects(dt);
  if (mGameState != kGS_SoftPaused) {
    for (uint player = 0; player < mNumPlayers; ++player) {
      mCameraManagers[player]->Update(cameraDt, *this);
    }
  }
  ThinkNewObjects(dt);
  if (mGameState != kGS_Paused) {
    PostUpdatePlayer(dt);
  }
  gpGameState->CurrentWorldState().SetAreaId(mNextAreaId);
  mWorld->TravelToArea(mNextAreaId, *this, CWorld::kATT_LoadAdjacent);
  ClearGraveyard();
  ++mUpdateFrameIdx;
  mArchQueue = nullptr;
}

void CStateManager::UpdateAreaSounds() {
  rstl::reserved_vector< int, 10 > areaIds;
  areaIds.clear();
  for (CGameArea::CConstChainIterator area = mWorld->GetChainHead(CWorld::kC_Alive);
       area != CWorld::GetAliveAreasEnd(); ++area) {
    if (area->GetOcclusionState() == CGameArea::kOS_Visible) {
      areaIds.push_back(area->GetId().Value());
    }
  }
  CSfxManager::SetActiveAreas(areaIds, mNextAreaId.Value());
}

void CStateManager::DisplayAlertAboutOutOfAmmo(const CPlayer& player,
                                            CPlayerState::EItemType type) {
  CObjectList* allList = mObjectLists[kOL_All].get();
  for (int i = allList->GetFirstObjectIndex(); i != -1; i = allList->GetNextObjectIndex(i)) {
    CScriptSpecialFunction* const special = TCastToPtr< CScriptSpecialFunction >((*allList)[i]);
    if (special != nullptr && special->GetFunction() == CScriptSpecialFunction::kSF_ItemDepletion) {
      special->OnItemDepleted(*this, player.GetPlayerIndex(), type);
    }
  }

  if (mPlayerStates[player.GetPlayerIndex()]->GetItemAmount(type, true) != 0) {
    return;
  }
  const uint playerIndex = MaskUIdNumPlayers(player.GetUniqueId());
  CHUDMemoParms memoInfo(3.f, true, false, false, 1 << playerIndex, true);
  switch (type) {
  case CPlayerState::kIT_LightAmmo:
    if (mDarkAmmoDepletedPlayers & (1 << player.GetPlayerIndex())) {
      CSamusHud::DisplayHudMemo(rstl::wstring(gpStringTable->GetString("BothAmmoDepleted")),
                               memoInfo);
    } else {
      CSamusHud::DisplayHudMemo(rstl::wstring(gpStringTable->GetString("LightAmmoDepleted")),
                               memoInfo);
    }
    mLightAmmoDepletedPlayers |= 1 << player.GetPlayerIndex();
    break;
  case CPlayerState::kIT_DarkAmmo:
    if (mLightAmmoDepletedPlayers & (1 << player.GetPlayerIndex())) {
      CSamusHud::DisplayHudMemo(rstl::wstring(gpStringTable->GetString("BothAmmoDepleted")),
                               memoInfo);
    } else {
      CSamusHud::DisplayHudMemo(rstl::wstring(gpStringTable->GetString("DarkAmmoDepleted")),
                               memoInfo);
    }
    mDarkAmmoDepletedPlayers |= 1 << player.GetPlayerIndex();
    break;
  default:
    break;
  }
}
