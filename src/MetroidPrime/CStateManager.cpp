#include "MetroidPrime/CStateManager.hpp"

#include "Collision/CRayCastResult.hpp"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CMapWorld.hpp"
#include "MetroidPrime/CMapWorldInfo.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CPortalArea.hpp"
#include "MetroidPrime/CPortalTransition.hpp"
#include "MetroidPrime/CProjectedShadow.hpp"
#include "MetroidPrime/CSaveGameScreen.hpp"
#include "MetroidPrime/CScriptMailbox.hpp"
#include "MetroidPrime/CSortedLists.hpp"
#include "MetroidPrime/CStateManagerContainer.hpp"
#include "MetroidPrime/CWeaponMgr.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/CWorldLayerState.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/GameObjectLists.hpp"
#include "MetroidPrime/Player/CGameMode.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Player/CWorldTransManager.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDoor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDynamicLight.hpp"
#include "MetroidPrime/ScriptObjects/CScriptEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpecialFunction.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"

#include "Kyoto/Audio/CAudioGroupSet.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/Basics/RAssertDolphin.hpp"
#include "Kyoto/CARAMManager.hpp"
#include "Kyoto/CARAMToken.hpp"
#include "Kyoto/CDependencyGroup.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CGraphicsPalette.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CSimpleShadow.hpp"
#include "Weapons/CDecal.hpp"

#include "rstl/algorithm.hpp"
#include "rstl/vector.hpp"

#include <float.h>

const int gkPVSEnabled = 1;

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
      const bool wasSkipping = IsSkippingCinematic();
      mSkippingCinematic = true;

      if (special->GetFunction() == CScriptSpecialFunction::kSF_CinematicSkip) {
        mCameraManagers[0]->StopCinematics(*this);
        result = 1;
      } else {
        result = 2;
      }
      special->SkipCinematic(*this);
      mSkippingCinematic = wasSkipping;
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
, mSkippingCinematic(false)
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
  mSkippingCinematic = true;

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

const CEntity* CStateManager::GetObjectById(TUniqueId uid) const {
  return GetObjectListById(kOL_All).GetObjectById(uid);
}

void CStateManager::SetIsDarkWorld(bool b) {
  mIsDarkWorld = b;
  gpGameState->SetIsDarkWorld(mIsDarkWorld);
}

bool CStateManager::HasWorld() const { return !mWorld.null(); }

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

bool CStateManager::ApplyLocalDamage(const CVector3f& pos, const CVector3f& dir, CActor& damagee,
                                     float damage, const TUniqueId& uid1, const TUniqueId& uid2,
                                     const CDamageInfo& damageInfo, int unkParam) {
  CHealthInfo* healthInfo = damagee.HealthInfo();
  if (!healthInfo || damage < 0.0f) {
    return false;
  }

  float hp = healthInfo->GetHP();
  if (hp <= 0.0f) {
    fn_8003dd88(damagee, uid1, damageInfo, false, unkParam);
    return true;
  }

  CPlayer* player = TCastToPtr< CPlayer >(damagee);

  if (player && player->Get_x12f8() != 0) {
    if (player->Get_x12f8() != 3) {
      return false;
    }
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
      switch ((EWeaponType)damageInfo.GetWeaponMode1()) {
      case kWT_Power:
      case kWT_Dark:
      case kWT_Light:
      case kWT_Annihilator:
      case kWT_Bomb:
      case kWT_PowerBomb:
      case kWT_Missile:
      case kWT_BoostBall:
      case kWT_CannonBall:
      case kWT_ScrewAttack:
      case kWT_AI:
      case kWT_PoisonWater1:
      case kWT_PoisonWater2:
      case kWT_Lava:
      case kWT_Heat:
      case kWT_Unused1:
      case kWT_AreaDark:
        damage *= gpGameState->GetHardModeDamageMultiplier();
        break;
      }
    }

    float damageReduction = 0.0f;

    if (playerState.HasPowerUp(CPlayerState::kIT_VariaSuit)) {
      damageReduction = player->GetTweakPlayer()->GetVariaSuitDamageReduction();
    }
    if (playerState.HasPowerUp(CPlayerState::kIT_DarkSuit)) {
      float reduction = player->GetTweakPlayer()->GetDarkSuitDamageReduction();
      if (reduction > damageReduction) {
        damageReduction = reduction;
      }
    }
    if (playerState.HasPowerUp(CPlayerState::kIT_LightSuit)) {
      float reduction = player->GetTweakPlayer()->GetLightSuitDamageReduction();
      if (reduction > damageReduction) {
        damageReduction = reduction;
      }
    }
    if (playerState.GetItemAmount(CPlayerState::kIT_AbsorbAttack, true) != 0) {
      float reduction = 1.5f;
      if (reduction > damageReduction) {
        damageReduction = reduction;
      }
    }
    if (playerState.GetItemAmount(CPlayerState::kIT_LightShield, true) != 0) {
      // TODO: flag
      float reduction = 0.75f;
      if (reduction > damageReduction) {
        damageReduction = reduction;
      }
    }
    if (playerState.GetItemAmount(CPlayerState::kIT_DarkShield, true) != 0) {
      // TODO: flag
      float reduction = 0.75f;
      if (reduction > damageReduction) {
        damageReduction = reduction;
      }
    }
    hp = playerState.CalculateHealth();
    damage = -(damageReduction * damage - damage);
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
