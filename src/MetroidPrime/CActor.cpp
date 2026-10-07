#include "MetroidPrime/CActor.hpp"

#include "Kyoto/Math/CAABox.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CActorModelParticles.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CEchoEmitter.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CPortalArea.hpp"
#include "MetroidPrime/CSimpleShadow.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Player/CPlayerTargeting.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"

#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Audio/CSfxPitchBend.hpp"
#include "Kyoto/CTimeProvider.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CPlane.hpp"

#include "MetroidPrime/TGameTypes.hpp"
#include "dolphin/types.h"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"

#include <float.h>

// TODO: how else would they end up in .data?
static EMaterialTypes SolidMaterial = kMT_Solid;

// Guessed name; sorts fluid volumes by their world-space surface height.
class CFluidHeightCompare {
public:
  explicit CFluidHeightCompare(CStateManager& mgr) : mManager(mgr) {}

  bool operator()(TUniqueId a, TUniqueId b) const;

private:
  CStateManager& mManager;
};

bool CFluidHeightCompare::operator()(TUniqueId a, TUniqueId b) const {
  const CScriptWater* waterA = TCastToConstPtr< CScriptWater >(mManager.GetObjectById(a));
  const CScriptWater* waterB = TCastToConstPtr< CScriptWater >(mManager.GetObjectById(b));
  if (waterA != nullptr && waterB != nullptr) {
    const float heightA = waterA->GetWRSurfacePlane().GetClosestPoint(CVector3f::Zero()).GetZ();
    const float heightB = waterB->GetWRSurfacePlane().GetClosestPoint(CVector3f::Zero()).GetZ();
    return heightA < heightB;
  }
  return false;
}

static CMaterialList MakeActorMaterialList(const CMaterialList& in,
                                           const CActorParameters& params) {
  CMaterialList ret = in;
  if (params.GetVisorParameters().GetScanPassthrough()) {
    ret.Add(kMT_ScanPassthrough);
  }
  return ret;
}

CActor::CActor(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, uint inGrave,
               const CTransform4f& xf, const CModelData& mData, const CMaterialList& list,
               const CActorParameters& params, TUniqueId nextDrawNode)
: CEntity(uid, info, name, inGrave | 1)
, mTransform(xf)
, mPosition(xf.GetTranslation())
, mModelData(mData.IsNull() ? nullptr : rs_new CModelData(mData))
, mMaterial(MakeActorMaterialList(list, params))
, mMaterialFilter(
      CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()))
, mLoopingSounds(4, TLoopingSound(InvalidSfxId, SSound(CSfxHandle(), CSegId::Invalid(), false)))
, mActorLights(mData.IsNull() ? nullptr : params.GetLighting().MakeActorLights().release())
, mOtherBounds(CAABox::MakeMaxInvertedBox())
, mRenderBounds(CAABox::MakeMaxInvertedBox())
, mDrawFlags(CModelFlags::Normal())
, mTime(0.f)
, mPitchBend(8192)
, mFluidIdsChanged(false)
, mNextDrawNode(nextDrawNode)
, mDrawnToken(-1)
, mAddedToken(-1)
, mPvsIndex(-1)
, mMaxVol(CAudioSys::kMaxVolume)
, mNormalVolume(params.GetMaxVolume())
, mEchoVolume(params.GetMaxEchoVolume())
, mNonLoopingSounds(2, SSound(CSfxHandle(), CSegId::Invalid(), false))
, mNextNonLoopingSfxHandle(0)
, mNotInSortedLists(true)
, mTransformDirty(true)
, mActorLightsDirty(true)
, mRenderBoundsDirty(true)
, mOutOfFrustum(false)
, mCalculateLighting(true)
, mShadowEnabled(false)
, mShadowDirty(false)
, mMuted(false)
, mUseInSortedLists(true)
, mUsePortalVisibility(true)
, mCallTouch(true)
, mGlobalTimeProvider(params.UseGlobalRenderTime())
, mRenderUnsorted(params.ForceRenderUnsorted())
, mPointGeneratorParticles(false)
, mRenderParticleDBInside(true)
, mEnablePitchBend(false)
, mTargetableVisorFlags(params.GetVisorParameters().GetMask())
, mEnableRender(true)
, mWorldLightingDirty(false)
, mDrawEnabled(info.GetActive())
, mDoTargetDistanceTest(true)
, mValidTargetPlayers(0xf)
, mEchoEmitterEnabled(false)
, mHighlightedInDarkVisor(params.IsHighlightedInDarkVisor())
, mDamageHighlight(false)
, mTakesProjectedShadow(params.TakesProjectedShadow())
, mLoopingSoundCount(0)
, mAlphaSorted(params.UseAlphaSorting()) {
  if (!mModelData.null()) {
    if (params.GetXRay().first != 0) {
      mModelData->SetEchoModel(params.GetXRay());
    }
    if (params.GetInfra().first != 0) {
      mModelData->SetDarkModel(params.GetInfra());
    }
    const CLightParameters& lighting = params.GetLighting();
    if (!lighting.ShouldMakeLights() || lighting.GetMaxAreaLights() == 0) {
      mModelData->SetAmbientColor(lighting.GetAmbientColor());
    }
    mModelData->SetRenderFullEchoModel(params.RenderFullEchoModel());
  }
  const CAssetId scanId = params.GetScannable().GetScannableObject0();
  if (scanId != kInvalidAssetId) {
    mScanObjectInfo = rs_new TCachedToken< CScannableObjectInfo >(
        gpSimplePool->GetObj(SObjectTag('SCAN', scanId)), true);
  }
}

CActor::~CActor() { StopLoopedSounds(); }

CAdvancementDeltas CActor::UpdateAnimation(float dt, CStateManager& mgr, bool advTree) {
  float cameraDistance = 0.f;
  if (!mgr.IsMultiplayer()) {
    const CGameCamera* camera = mgr.GetCameraManager(0)->GetCurrentCamera(mgr, false);
    cameraDistance = (camera->GetTranslation() - GetTranslation()).Magnitude();
  }
  CAdvancementDeltas result =
      ModelData()->AdvanceAnimation(dt, mgr, GetAreaIdForPersistence(), advTree, cameraDistance);
  ModelData()->AdvanceParticles(GetTransform(), dt, mgr);
  UpdateSfxEmitters(mgr);
  if (HasAnimation()) {
    const uchar maxVol = mMaxVol;
    const int area = GetCurrentAreaId().Value();
    const CVector3f position = GetTranslation();
    const float distanceSquared = GetDistanceToCamera(mgr);

    int soundNodeCount = 0;
    const CSoundPOINode* soundNodes =
        HasAnimation() ? GetAnimationData()->GetSoundPOIList(soundNodeCount) : nullptr;
    if (soundNodes != nullptr) {
      for (int i = 0; i < soundNodeCount; ++i) {
        const CSoundPOINode& node = soundNodes[i];
        if (node.GetPoiType() != kPT_Sound || GetMuted()) {
          continue;
        }
        if (node.GetCharacterIndex() != -1 &&
            node.GetCharacterIndex() != GetAnimationData()->GetCharacterIndex()) {
          continue;
        }
        ProcessSoundEvent(node.GetSoundId(), node.GetWeight(), node.GetFlags(), node.GetFallOff(),
                          node.GetMaxDistance(), node.GetLocator(), node.GetPitchStart(),
                          node.GetPitchEnd(), node.GetPitchDuration(), 20, maxVol, distanceSquared,
                          position, area, mgr, true);
      }
    }

    int intNodeCount = 0;
    const CInt32POINode* intNodes =
        HasAnimation() ? GetAnimationData()->GetInt32POIList(intNodeCount) : nullptr;
    if (intNodes != nullptr) {
      for (int i = 0; i < intNodeCount; ++i) {
        const CInt32POINode& node = intNodes[i];
        if (node.GetPoiType() == kPT_SoundInt32 && !GetMuted() &&
            (node.GetCharacterIndex() == -1 ||
             node.GetCharacterIndex() == GetAnimationData()->GetCharacterIndex())) {
          ProcessSoundEvent(node.GetValue(), node.GetWeight(), node.GetFlags(), 0.1f, 150.f,
                            CSegId(0), 0, 0, 0.f, 20, maxVol, distanceSquared, position, area, mgr,
                            true);
        } else if (node.GetPoiType() == kPT_UserEvent) {
          DoUserAnimEvent(mgr, node, static_cast< EUserEventType >(node.GetValue()), dt);
        } else if (node.GetPoiType() == kPT_StopLoopedSound) {
          StopLoopedSound(node.GetValue());
        }
      }
    }

    int particleNodeCount = 0;
    const CParticlePOINode* particleNodes =
        HasAnimation() ? GetAnimationData()->GetParticlePOIList(particleNodeCount) : nullptr;
    if (particleNodes != nullptr) {
      for (int i = 0; i < particleNodeCount; ++i) {
        const CParticlePOINode& node = particleNodes[i];
        if (node.GetCharacterIndex() != -1 &&
            node.GetCharacterIndex() != GetAnimationData()->GetCharacterIndex()) {
          continue;
        }
        if (cameraDistance < node.GetMaximumDistance() ||
            mgr.GetCameraManager(0)->IsInCinematicCamera()) {
          AnimationData()->GetParticleDB().SetParticleEffectState(node.GetNameHash(), true, &mgr);
        }
      }
    }
  }
  return result;
}

void CActor::StopLoopedSounds() {
  for (uint i = 0; i < mLoopingSoundCount; ++i) {
    if (const CSfxHandle& handle = mLoopingSounds[i].second.mHandle) {
      CSfxManager::RemoveEmitter(handle);
      mLoopingSounds[i].first = InvalidSfxId;
      mLoopingSounds[i].second = SSound(CSfxHandle(), CSegId::Invalid(), false);
    }
  }
  mLoopingSoundCount = 0;
}

void CActor::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                             float dt) {
  if (type == kUE_LoopedSoundStop) {
    StopLoopedSounds();
  }
}

float CActor::GetAverageAnimVelocity(int anim) {
  return HasAnimation() ? GetAnimationData()->GetAverageVelocity(anim) : 0.f;
}

void CActor::PreRenderAllViewports(CStateManager& mgr) {
  if (HasModelData()) {
    CAABox bounds = GetModelData()->GetBounds(GetTransform());
    SetRenderBounds(bounds);
    if (HasAnimation()) {
      rstl::optional_object< CAABox > new_bounds =
          GetModelData()->GetAnimationData()->GetParticleDB().GetTotalBounds();
      if (new_bounds) {
        const CAABox& particleBounds = *new_bounds;
        bounds.AccumulateBounds(particleBounds.GetMinPoint());
        bounds.AccumulateBounds(particleBounds.GetMaxPoint());
      }
    }
    mOtherBounds = bounds;
  } else {
    const CVector3f origin = GetTranslation();
    SetRenderBounds(CAABox(origin, origin));
    mOtherBounds = CAABox(origin, origin);
  }
  if (mRenderBoundsDirty) {
    UpdatePortalSystemState(mgr);
    mRenderBoundsDirty = 0;
  }
}

void CActor::SetModelData(const CModelData& data, CStateManager& mgr) {
  if (data.IsNull()) {
    if (HasAnimation()) {
      AnimationData()->GetParticleDB().DeleteAllLights(&mgr);
    }
    mModelData = nullptr;
  } else {
    mModelData = rs_new CModelData(data);
  }
}

void CActor::PreRender(CStateManager& mgr) {
  mOutOfFrustum = !mgr.IsActorVisible(*this);

  if (HasModelData()) {
    const bool moved = GetPreRenderHasMoved();
    if (moved) {
      SetPreRenderHasMoved(false);
    }
    if (!GetPreRenderClipped()) {
      bool lightsDirty = false;
      if (moved) {
        SetShadowDirty(true);
        lightsDirty = true;
      } else if (mWorldLightingDirty) {
        lightsDirty = true;
      } else if (HasActorLights() && GetActorLights()->GetNeedsRelight() == true) {
        lightsDirty = true;
      }

      if (GetCalculateLighting()) {
        CAABox bounds = GetModelData()->GetBounds(GetTransform());
        if (lightsDirty == true) {
          if (GetCurrentAreaId() != kInvalidAreaId) {
            TAreaId aid = GetCurrentAreaId();
            if (mgr.GetWorld()->IsAreaValid(aid)) {
              const CGameArea* area = mgr.GetWorld()->GetArea(aid);
              if (ActorLights()->BuildAreaLightList(mgr, *area, bounds)) {
                mWorldLightingDirty = false;
              }
            }
          }
        }
        ActorLights()->BuildDynamicLightList(mgr, bounds);
      }

      if (GetModelData()->HasAnimation()) {
        AnimationData()->PreRender();
      }
    } else if (moved) {
      SetShadowDirty(true);
    }

    if (GetShadowDirty() && ShouldDrawShadow(mgr)) {
      if (mgr.GetFrustumPlanes().BoxInFrustumPlanes(
              GetShadow()->GetMaxShadowBox(GetModelData()->GetBounds(GetTransform())))) {
        Shadow()->Calculate(GetModelData()->GetBounds(), GetTransform(), mgr);
        SetShadowDirty(false);
      }
    }
  }
}

bool CActor::ShouldDrawShadow(const CStateManager& mgr) const {
  return GetDrawShadow() && mgr.GetRenderVisorMode() == CStateManager::kRVM_Normal;
}

void CActor::AddToRenderer(const CStateManager& mgr) const {
  if (HasModelData()) {
    if (GetRenderParticleDatabaseInside()) {
      GetModelData()->RenderParticles(mgr.GetFrustumPlanes());
    }

    if (!GetPreRenderClipped()) {
      if (CanRenderUnsorted(mgr)) {
        Render(mgr);
      } else {
        EnsureRendered(mgr);
      }
    }

    if (mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::kPV_Echo) {
      if (ShouldDrawShadow(mgr)) {
        if (GetShadow()->Valid() &&
            mgr.GetFrustumPlanes().BoxInFrustumPlanes(GetShadow()->GetBounds())) {
          gpRender->AddDrawable(GetShadow(), GetShadow()->GetTransform().GetTranslation(),
                                GetShadow()->GetBounds(), 1, IRenderer::kDS_SortedCallback);
        }
      }
    }
  }
}

int CActor::GetRenderAlphaBufferAlpha(const CStateManager& mgr) const {
  const CPlayer* player = mgr.GetCurrentRenderPlayer();
  switch (player->GetPlayerState()->GetActiveVisor(mgr)) {
  case CPlayerState::kPV_Scan:
    if (!mMaterial.HasMaterial(kMT_ScanPassthrough)) {
      return player->GetTargeting()->GetScanTargetIndex(mgr, GetUniqueId()) << 2;
    }
    return -1;
  case CPlayerState::kPV_Dark:
    if (mHighlightedInDarkVisor) {
      return mDamageHighlight ? 0xff : 0xfb;
    }
    return 0;
  default:
    return -1;
  }
}

void CActor::EnsureRendered(const CStateManager& mgr, const CVector3f& pos,
                            const CAABox& bounds) const {
  if (GetModelData()) {
    const CModelData::EWhichModel which = CModelData::GetRenderingModel(mgr);
    int value = GetRenderAlphaBufferAlpha(mgr);
    if (value != -1) {
      gpRender->SetDestinationAlpha(value);
    }
    GetModelData()->RenderUnsortedParts(which, GetTransform(), GetActorLights(), GetModelFlags());

    if (value != -1) {
      gpRender->DisableDestinationAlpha();
    }
  }
  mgr.AddDrawableActor(*this, pos, bounds);
}

void CActor::EnsureRendered(const CStateManager& mgr) const {
  const CAABox bounds = GetSortingBounds(mgr);
  const CVector3f viewForward = CGraphics::GetViewMatrix().GetForward();
  const CVector3f pos = bounds.ClosestPointAlongVector(viewForward);
  EnsureRendered(mgr, pos, bounds);
}

bool CActor::CanRenderUnsorted(const CStateManager& mgr) const {
  bool result = HasAnimation();
  if (result && GetAnimationData()->GetParticleDB().AreAnySystemsDrawnWithModel() &&
      GetRenderParticleDatabaseInside()) {
    result = false;
  } else {
    result = mRenderUnsorted || IsModelOpaque(mgr);
  }
  return result;
}

void CActor::Render(const CStateManager& mgr) const {
  if (GetModelData() && !NullModel()) {
    bool renderPrePostParticles = GetRenderParticleDatabaseInside() && HasAnimation();
    if (renderPrePostParticles) {
      GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnFirst();
    }

    if (mEnableRender) {
      if (mPointGeneratorParticles) {
        mgr.SetupParticleHook(*this);
      }
      if (mGlobalTimeProvider) {
        RenderInternal(mgr);
      } else {
        const float timeSince = CGraphics::GetSecondsMod900() - mTime;
        CTimeProvider tp(CMath::FastFmod(timeSince, 900.f));
        RenderInternal(mgr);
      }
      if (mPointGeneratorParticles) {
        CSkinnedModel::ClearPointGeneratorFunc();
        mgr.GetActorModelParticles()->Render(mgr, *this);
      }
    }

    if (renderPrePostParticles) {
      GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnLast();
    }
  }
}

void CActor::RenderInternal(const CStateManager& mgr) const {
  const CModelData::EWhichModel which = CModelData::GetRenderingModel(mgr);
  int value = GetRenderAlphaBufferAlpha(mgr);
  if (value != -1) {
    gpRender->SetDestinationAlpha(value);
  }
  GetModelData()->Render(which, GetTransform(), GetActorLights(), GetModelFlags());

  if (value != -1) {
    gpRender->DisableDestinationAlpha();
  }
}

float CActor::GetYaw() const {
  float sq = CMath::SqrtF(mTransform.Get11() * mTransform.Get11() +
                          mTransform.Get01() * mTransform.Get01());
  if (sq > 0.001f) {
    return -atan2f(mTransform.Get01(), mTransform.Get11());
  }
  return 0.f;
}

CHealthInfo* CActor::HealthInfo() { return nullptr; }

const CDamageVulnerability* CActor::GetDamageVulnerability() const {
  return &CDamageVulnerability::NormalVulnerabilty();
}

const CDamageVulnerability* CActor::GetDamageVulnerability(const CVector3f&, const CVector3f&,
                                                           const CDamageInfo&) const {
  return GetDamageVulnerability();
}

rstl::optional_object< CAABox > CActor::GetTouchBounds() const {
  return rstl::optional_object_null();
}

void CActor::Touch(CActor&, CStateManager&) {}

bool CActor::GetUseInSortedLists() const { return mUseInSortedLists; }

void CActor::SetUseInSortedLists(bool use) { mUseInSortedLists = use; }

bool CActor::GetCallTouch() const { return mCallTouch; }

void CActor::SetCallTouch(bool value) { mCallTouch = value; }

void CActor::AddMaterial(EMaterialTypes mat1, CStateManager& mgr) {
  mMaterial.Add(mat1);
  mgr.UpdateObjectInLists(*this);
}

void CActor::AddMaterial(EMaterialTypes mat1, EMaterialTypes mat2, CStateManager& mgr) {
  mMaterial.Add(mat1);
  mMaterial.Add(mat2);
  mgr.UpdateObjectInLists(*this);
}

void CActor::AddMaterial(EMaterialTypes mat1, EMaterialTypes mat2, EMaterialTypes mat3,
                         CStateManager& mgr) {
  mMaterial.Add(mat1);
  mMaterial.Add(mat2);
  mMaterial.Add(mat3);
  mgr.UpdateObjectInLists(*this);
}

void CActor::AddMaterial(EMaterialTypes mat1, EMaterialTypes mat2, EMaterialTypes mat3,
                         EMaterialTypes mat4, CStateManager& mgr) {
  mMaterial.Add(mat1);
  mMaterial.Add(mat2);
  mMaterial.Add(mat3);
  mMaterial.Add(mat4);
  mgr.UpdateObjectInLists(*this);
}

void CActor::AddMaterial(EMaterialTypes mat1, EMaterialTypes mat2, EMaterialTypes mat3,
                         EMaterialTypes mat4, EMaterialTypes mat5, CStateManager& mgr) {
  mMaterial.Add(mat1);
  mMaterial.Add(mat2);
  mMaterial.Add(mat3);
  mMaterial.Add(mat4);
  mMaterial.Add(mat5);
  mgr.UpdateObjectInLists(*this);
}

void CActor::RemoveMaterial(EMaterialTypes mat1, CStateManager& mgr) {
  mMaterial.Remove(mat1);
  mgr.UpdateObjectInLists(*this);
}

void CActor::RemoveMaterial(EMaterialTypes mat1, EMaterialTypes mat2, CStateManager& mgr) {

  mMaterial.Remove(mat1);
  mMaterial.Remove(mat2);
  mgr.UpdateObjectInLists(*this);
}

void CActor::RemoveMaterial(EMaterialTypes mat1, EMaterialTypes mat2, EMaterialTypes mat3,
                            CStateManager& mgr) {

  mMaterial.Remove(mat1);
  mMaterial.Remove(mat2);
  mMaterial.Remove(mat3);
  mgr.UpdateObjectInLists(*this);
}

void CActor::RemoveMaterial(EMaterialTypes mat1, EMaterialTypes mat2, EMaterialTypes mat3,
                            EMaterialTypes mat4, CStateManager& mgr) {

  mMaterial.Remove(mat1);
  mMaterial.Remove(mat2);
  mMaterial.Remove(mat3);
  mMaterial.Remove(mat4);
  mgr.UpdateObjectInLists(*this);
}

void CActor::RemoveMaterial(EMaterialTypes mat1, EMaterialTypes mat2, EMaterialTypes mat3,
                            EMaterialTypes mat4, EMaterialTypes mat5, CStateManager& mgr) {

  mMaterial.Remove(mat1);
  mMaterial.Remove(mat2);
  mMaterial.Remove(mat3);
  mMaterial.Remove(mat4);
  mMaterial.Remove(mat5);
  mgr.UpdateObjectInLists(*this);
}

void CActor::SetMaterialList(const CMaterialList& l, CStateManager& mgr) {
  mMaterial = l;
  mgr.UpdateObjectInLists(*this);
}

EWeaponCollisionResponseTypes CActor::GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                               const CWeaponMode&, int) const {
  return kWCR_OtherProjectile;
}

CVector3f CActor::GetOrbitPosition(const CStateManager&) const { return mPosition; }

CVector3f CActor::GetAimPosition(const CStateManager&, float) const { return mPosition; }

CVector3f CActor::GetHomingPosition(const CStateManager& mgr, float f) const {
  return GetAimPosition(mgr, f);
}

CVector3f CActor::GetScanObjectIndicatorPosition(const CStateManager& mgr) const {
  return GetOrbitPosition(mgr);
}

bool CActor::IsModelOpaque(const CStateManager& mgr) const {
  if (mPointGeneratorParticles) {
    return false;
  } else if (!HasModelData()) {
    return true;
  } else if (static_cast< char >(mDrawFlags.GetTrans()) > 4) {
    return false;
  } else {
    CModelData::EWhichModel which = CModelData::GetRenderingModel(mgr);
    return mModelData->IsDefinitelyOpaque(which);
  }
}

void CActor::SetCalculateLighting(bool b) {
  if (b && mActorLights.null()) {
    mActorLights = rs_new CActorLights(8, CVector3f::Zero(), 4, 4);
  }
  mCalculateLighting = b;
}

void CActor::SetActorLights(rstl::auto_ptr< CActorLights > lights) {
  mActorLights = lights.release();
  mCalculateLighting = true;
}

const CMaterialFilter& CActor::GetMaterialFilter() const { return mMaterialFilter; }

void CActor::SetMaterialFilter(const CMaterialFilter& filter) { mMaterialFilter = filter; }

void CActor::SetActive(const bool active) {
  if (mDrawEnabled != active) {
    SetTransformDirty();
    mDrawEnabled = active;
  }
  CEntity::SetActive(active);
}

void CActor::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Activate: {
    if (!GetActive()) {
      mTime = CGraphics::GetSecondsMod900();
    }
    break;
  }
  case kSM_Deactivate: {
    StopLoopedSounds();
    break;
  }
  case kSM_Delete: {
    StopLoopedSounds();
    if (HasModelData() && AnimationData() != nullptr) {
      AnimationData()->GetParticleDB().DeleteAllLights(&mgr);
    }
    if (mEchoEmitterEnabled) {
      mEchoEmitter->DestroyEmitter(mgr);
    }
    break;
  }
  case kSM_Create: {
    if (GetScannableObjectInfo() != nullptr) {
      AddMaterial(kMT_Scannable, mgr);
    } else {
      RemoveMaterial(kMT_Scannable, mgr);
    }
    if (HasAnimation()) {
      AnimationData()->InitializeEffects(mgr, GetAreaIdForPersistence(),
                                         GetModelData()->GetScale());
    }
    if (mEchoEmitterEnabled) {
      mEchoEmitter->CreateEmitter(mgr);
    }
    break;
  }
  case kSM_AreaLoaded: {
    rstl::vector< SConnection >::const_iterator iter = GetConnectionList().begin();
    for (; iter != GetConnectionList().end(); ++iter) {
      if (iter->state != kSS_DefaultState) {
        continue;
      }
      CEntity* entity = mgr.ObjectById(mgr.GetIdForScript(iter->objId));
      CActor* act = TCastToPtr< CActor >(entity);
      if (act != nullptr && mNextDrawNode == kInvalidUniqueId) {
        mNextDrawNode = act->GetUniqueId();
      }
    }
    break;
  }
  default:
    break;
  }
  CEntity::AcceptScriptMsg(mgr, msg);
}

CAABox CActor::GetSortingBounds(const CStateManager& mgr) const { return GetRenderBoundsCached(); }

void CActor::FluidFXThink(EFluidState, CScriptWater&, CStateManager&) {}

void CActor::OnScanStateChange(EScanState state, CStateManager& mgr) {
  switch (state) {
  case kSS_Start:
    SendScriptMsgs(kSS_ScanProcessing, mgr);
    break;
  case kSS_Processing:
    SendScriptMsgs(kSS_ScanStart, mgr);
    break;
  case kSS_Done:
    SendScriptMsgs(kSS_ScanDone, mgr);
    break;
  }
}

CScannableObjectInfo* CActor::GetScannableObjectInfo() const {
  return mScanObjectInfo.null() ? nullptr : mScanObjectInfo->GetObject();
}

void CActor::SetMuted(bool b) {
  mMuted = b;
  StopLoopedSounds();
}

void CActor::SetVolume(uchar volume) {
  for (uint i = 0; i < mLoopingSoundCount; ++i) {
    if (const CSfxHandle& handle = mLoopingSounds[i].second.mHandle) {
      CSfxManager::UpdateEmitter(handle, GetTranslation(), CVector3f::Zero(), volume);
    }
  }
  mMaxVol = volume;
}

void CActor::SetSoundEventPitchBend(int v) {
  mEnablePitchBend = true;
  mPitchBend = v;
  for (uint i = 0; i < mLoopingSoundCount; ++i) {
    TLoopingSound& sound = mLoopingSounds[i];
    if (sound.second.mHandle) {
      CSfxManager::PitchBend(sound.second.mHandle, v);
    }
  }
}

void CActor::ProcessSoundEvent(int sfxId, float weight, int flags, float fallOff, float maxDist,
                               const CSegId& locator, ushort pitchStart, ushort pitchEnd,
                               float pitchDuration, uchar minVol, uchar maxVol,
                               float distanceSquared, const CVector3f& position, int aid,
                               CStateManager& mgr, bool translateId) {
  if (distanceSquared < maxDist * maxDist) {
    const ushort id = translateId ? static_cast< ushort >(sfxId) : static_cast< ushort >(sfxId);
    if (sfxId & 0x20000) {
      aid = CSfxManager::kAllAreas;
    }
    const bool useAcoustics = (flags & 0x80) == 0;
    const bool looping = (sfxId & 0x80000000) != 0;
    const bool nonEmitter = (sfxId & 0x40000000) != 0;
    const bool continuousUpdate = (sfxId & 0x20000000) != 0;
    const bool useEchoVolume = (sfxId & 0x40000) != 0;
    uchar volume = maxVol;
    if (useEchoVolume) {
      volume = GetVisorSoundVolume(mgr);
    }
    uint musyxFlags = 0x1; // Continuous parameter update
    if (flags & 0x8) {
      musyxFlags |= 0x8; // Doppler FX
    }

    CAudioSys::C3DEmitterParmData parms(maxDist, fallOff, musyxFlags, volume,
                                        rstl::min_val(minVol, volume));
    if (locator.val() == 0) {
      parms.mPos = position;
    } else {
      parms.mPos = (GetTransform() * GetScaledLocatorTransform(locator)).GetTranslation();
    }
    parms.mDir = CVector3f::Zero();
    parms.mSfxId = id;

    if (mgr.Random()->Float() <= weight) {
      if (looping) {
        PlayLoopedSound(id, flags, fallOff, maxDist, rstl::min_val(minVol, volume), volume,
                        nonEmitter, aid, useAcoustics, locator, pitchStart, pitchEnd, pitchDuration,
                        useEchoVolume);
      } else {
        CSfxHandle handle;
        if (!nonEmitter) {
          handle =
              CSfxManager::AddEmitter(parms, aid, useAcoustics, false, CSfxManager::kMedPriority);
        } else {
          short pan = 64;
          if (flags & 0x10000000) {
            if (const CPlayer* player =
                    TCastToConstPtr< CPlayer >(mgr.GetObjectById(GetUniqueId()))) {
              pan = player->GetSoundPan(CPlayer::kMSP_4);
            }
          }
          handle = CSfxManager::SfxStart(id, 127, pan, aid, useAcoustics, false,
                                         CSfxManager::kMedPriority);
        }
        if (continuousUpdate) {
          mNonLoopingSounds[mNextNonLoopingSfxHandle] = SSound(handle, locator, useEchoVolume);
          mNextNonLoopingSfxHandle = (mNextNonLoopingSfxHandle + 1) % mNonLoopingSounds.size();
        }

        if (handle) {
          if (mEnablePitchBend) {
            CSfxManager::PitchBend(handle, mPitchBend);
          }
          if (pitchDuration > 0.f) {
            CSfxManager::AddPitchBend(CSfxPitchBend(handle, pitchStart, pitchEnd, pitchDuration));
          } else if (!mEnablePitchBend) {
            CSfxManager::PitchBend(handle, pitchStart);
          }
        }
      }
    }
  }
}

CTransform4f CActor::GetLocatorTransform(const rstl::string& segName) const {
  return GetModelData()->GetLocatorTransform(segName);
}

CTransform4f CActor::GetScaledLocatorTransform(const rstl::string& segName) const {
  return GetModelData()->GetScaledLocatorTransform(segName);
}

void CActor::SetTranslation(const CVector3f& vec) {
  mTransform.SetTranslation(vec);
  mPosition = vec;
  SetTransformDirty();
}

CTransform4f CActor::GetScaledLocatorTransform(const CSegId& locator) const {
  return GetModelData()->GetScaledLocatorTransform(locator);
}

void CActor::SetDrawShadow(bool enabled) {
  if (enabled) {
    AllocateShadow();
    if (!mShadowEnabled && HasShadow()) {
      mShadowDirty = true;
    }
  }
  mShadowEnabled = enabled;
}

void CActor::AllocateShadow() {
  if (!HasShadow() && HasModelData()) {
    mSimpleShadow = rs_new CSimpleShadow(1.f, 1.f, 20.f, 0.05f);
  }
}

bool CActor::CanDrawStatic() const {
  if (!GetActive() || !HasModelData() || static_cast< char >(mDrawFlags.GetTrans()) > 4) {
    return false;
  }
  if (NullModel() || GetAnimationData()) {
    return false;
  }
  return true;
}

void CActor::ClearSoundEventPitchBend() { mEnablePitchBend = false; }

TUniqueId CActor::InFluidId() const {
  if (mFluidIds.empty()) {
    return kInvalidUniqueId;
  }
  return mFluidIds.back();
}

void CActor::RemoveInvalidFluidIds(CStateManager& mgr) {
  rstl::reserved_vector< TUniqueId, 4 >::iterator it = mFluidIds.begin();
  while (it != mFluidIds.end()) {
    if (!TCastToConstPtr< CScriptWater >(mgr.GetObjectById(*it))) {
      it = mFluidIds.erase(it);
    } else {
      ++it;
    }
  }
}

void CActor::SetInFluid(CStateManager& mgr, bool inFluid, TUniqueId uid) {
  if (inFluid) {
    bool found = false;
    for (int i = 0; i < mFluidIds.size(); ++i) {
      if (mFluidIds[i] == uid) {
        found = true;
        break;
      }
    }
    if (!found && mFluidIds.size() != mFluidIds.capacity()) {
      if (!mFluidIdsChanged) {
        mFluidIdsChanged = true;
        mPreviousFluidIds = mFluidIds;
      }
      mFluidIds.push_back(uid);
      rstl::sort(mFluidIds.begin(), mFluidIds.end(), CFluidHeightCompare(mgr));
    }
  } else {
    for (rstl::reserved_vector< TUniqueId, 4 >::iterator it = mFluidIds.begin();
         it != mFluidIds.end(); ++it) {
      if (*it == uid) {
        if (!mFluidIdsChanged) {
          mFluidIdsChanged = true;
          mPreviousFluidIds = mFluidIds;
        }
        mFluidIds.erase(it);
        break;
      }
    }
  }

  rstl::reserved_vector< TUniqueId, 4 >::iterator it = mFluidIds.begin();
  while (it != mFluidIds.end()) {
    if (!TCastToConstPtr< CScriptWater >(mgr.GetObjectById(*it))) {
      it = mFluidIds.erase(it);
    } else {
      ++it;
    }
  }
}

const rstl::reserved_vector< TUniqueId, 4 >& CActor::GetFluidList() const { return mFluidIds; }

void CActor::SetFluidList(const rstl::reserved_vector< TUniqueId, 4 >& fluids) {
  mFluidIds = fluids;
  mPreviousFluidIds = mFluidIds;
  mFluidIdsChanged = false;
}

void CActor::ClearFluidList(CStateManager& mgr) {
  mFluidIds.clear();
  mPreviousFluidIds.clear();
  mFluidIdsChanged = false;
}

uint CActor::GetVisorSoundVolume(const CStateManager& mgr) const {
  if (!mgr.IsMultiplayer()) {
    int volume = mNormalVolume;
    if (mgr.GetPlayer(0)->GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Echo) {
      volume = mEchoVolume;
    }
    return static_cast< uchar >(volume);
  }
  return mMaxVol;
}

void CActor::UpdateSfxEmitters(CStateManager& mgr) {
  const CVector3f position = GetTranslation();
  uint i = 0;
  const uint count = mNonLoopingSounds.size();
  for (; i < count; ++i) {
    const CSegId& locator = mNonLoopingSounds[i].mLocator;
    const CVector3f soundPosition =
        locator.val() == 0 ? position
                           : (GetTransform() * GetScaledLocatorTransform(locator)).GetTranslation();
    uint volume = mMaxVol;
    if (mNonLoopingSounds[i].mUseEchoVolume) {
      volume = GetVisorSoundVolume(mgr);
    }
    CSfxManager::UpdateEmitter(mNonLoopingSounds[i].mHandle, soundPosition, CVector3f::Zero(),
                               volume);
  }
  for (i = 0; i < mLoopingSoundCount; ++i) {
    const CSegId& locator = mLoopingSounds[i].second.mLocator;
    const CVector3f soundPosition =
        locator.val() == 0 ? position
                           : (GetTransform() * GetScaledLocatorTransform(locator)).GetTranslation();
    uint volume = mMaxVol;
    if (mLoopingSounds[i].second.mUseEchoVolume) {
      volume = GetVisorSoundVolume(mgr);
    }
    CSfxManager::UpdateEmitter(mLoopingSounds[i].second.mHandle, soundPosition, CVector3f::Zero(),
                               volume);
  }
}

CSfxHandle CActor::PlayCustomSound(const CVector3f& position, const CVector3f& direction,
                                   const SLdrAudioPlaybackParms& parameters, bool looped) const {
  const int areaId = GetCurrentAreaId().Value();
  CAudioSys::C3DEmitterParmData emitter(parameters.maximumDistance, parameters.fallOff, 1,
                                        parameters.maxVolume, parameters.minVolume);
  emitter.mPos = position;
  emitter.mDir = direction;
  emitter.mSfxId = parameters.sound_Id;
  return CSfxManager::AddEmitter(emitter, areaId, parameters.useRoomAcoustics, looped,
                                 CSfxManager::kMedPriority);
}

void CActor::StopLoopedSound(ushort sfxId) {
  for (uint i = 0; i < mLoopingSoundCount; ++i) {
    if (mLoopingSounds[i].first == sfxId) {
      TLoopingSound& sound = mLoopingSounds[i];
      if (const CSfxHandle& handle = sound.second.mHandle) {
        CSfxManager::RemoveEmitter(handle);
      }
      sound.first = InvalidSfxId;
      sound.second = SSound(CSfxHandle(), CSegId::Invalid(), false);
      RemoveLoopedSoundAt(i);
      return;
    }
  }
}

void CActor::PlayLoopedSound(ushort sfxId, int flags, float fallOff, float maxDist, uchar minVol,
                             uchar maxVol, bool nonEmitter, int area, bool useAcoustics,
                             const CSegId& locator, ushort pitchStart, ushort pitchEnd,
                             float pitchDuration, bool useEchoVolume) {
  if (FindLoopedSound(sfxId)) {
    return;
  }

  uint musyxFlags = 1;
  if (flags & 8) {
    musyxFlags |= 8;
  }
  CAudioSys::C3DEmitterParmData emitter(maxDist, fallOff, musyxFlags, maxVol, minVol);
  emitter.mPos = locator.val() == 0
                     ? GetTranslation()
                     : (GetTransform() * GetScaledLocatorTransform(locator)).GetTranslation();
  emitter.mDir = CVector3f::Zero();
  emitter.mSfxId = sfxId;

  if (mLoopingSoundCount < 4) {
    AddLoopedSound(sfxId, nonEmitter, area, useAcoustics, emitter, locator, pitchStart, pitchEnd,
                   pitchDuration, useEchoVolume);
  } else if (flags & 4) {
    const CSfxHandle handle = mLoopingSounds[0].second.mHandle;
    CSfxManager::RemoveEmitter(handle);
    RemoveLoopedSoundAt(0);
    AddLoopedSound(sfxId, nonEmitter, area, useAcoustics, emitter, locator, pitchStart, pitchEnd,
                   pitchDuration, useEchoVolume);
  }
}

void CActor::AddLoopedSound(ushort sfxId, bool nonEmitter, int area, const bool useAcoustics,
                            CAudioSys::C3DEmitterParmData& parameters, const CSegId& locator,
                            ushort pitchStart, ushort pitchEnd, float pitchDuration,
                            bool useEchoVolume) {
  CSfxHandle handle;
  if (nonEmitter) {
    handle = CSfxManager::SfxStart(sfxId, 127, 64, area, true, true, CSfxManager::kMedPriority);
  } else {
    handle =
        CSfxManager::AddEmitter(parameters, area, useAcoustics, true, CSfxManager::kMedPriority);
  }

  if (handle) {
    mLoopingSounds[mLoopingSoundCount].first = sfxId;
    mLoopingSounds[mLoopingSoundCount].second = SSound(handle, locator, useEchoVolume);
    if (mEnablePitchBend) {
      CSfxManager::PitchBend(handle, mPitchBend);
    }
    if (pitchDuration > 0.f) {
      CSfxManager::AddPitchBend(CSfxPitchBend(handle, pitchStart, pitchEnd, pitchDuration));
    } else if (!mEnablePitchBend) {
      CSfxManager::PitchBend(handle, pitchStart);
    }
  }
  ++mLoopingSoundCount;
}

void CActor::RemoveLoopedSoundAt(int index) {
  for (uint i = index + 1; i < mLoopingSoundCount; ++i) {
    mLoopingSounds[i - 1] = mLoopingSounds[i];
  }
  --mLoopingSoundCount;
}

bool CActor::FindLoopedSound(ushort sfxId) {
  for (uint i = 0; i < mLoopingSoundCount; ++i) {
    if (mLoopingSounds[i].first == sfxId) {
      return true;
    }
  }
  return false;
}

void CActor::SetValidTarget(int playerIndex, bool enabled) {
  if (enabled) {
    const uint cur = GetValidTargetPlayers();
    mValidTargetPlayers = cur | (1 << playerIndex);
  } else {
    const uint cur = GetValidTargetPlayers();
    mValidTargetPlayers = cur & ~((1 << playerIndex) & 0xf);
  }
}

void CActor::SetVisorOrbitableFlags(CVisorParameters::EVisorOrbitableFlags flags, bool enabled) {
  if (enabled) {
    const uint cur = GetTargetableVisorFlags();
    mTargetableVisorFlags = cur | flags;
  } else {
    mTargetableVisorFlags = GetTargetableVisorFlags() & ~flags;
  }
}

CEchoEmitter* CActor::AllocateEchoEmitter(bool enabled, const CAABox& bounds,
                                          const SEchoParameters& parameters) {
  if (parameters.mIsEchoEmitter) {
    mEchoEmitterEnabled = enabled;
    if (mEchoEmitter.null()) {
      mEchoEmitter = rs_new CEchoEmitter(bounds, parameters);
    } else {
      mEchoEmitter->SetParameters(parameters);
    }
  }
  return mEchoEmitter.get();
}

void CActor::SetEchoEmitter(bool enabled, CEchoEmitter* emitter) {
  if (emitter != nullptr && (mEchoEmitter.null() || mEchoEmitter->IsPendingDeletion())) {
    mEchoEmitterEnabled = enabled;
    mEchoEmitter = emitter;
  }
}

void CActor::Think(float dt, CStateManager& mgr) {
  if (mEchoEmitterEnabled && GetActive()) {
    const rstl::optional_object< CAABox > bounds = GetTouchBounds();
    if (bounds) {
      mEchoEmitter->SetBounds(*bounds);
    }
    mEchoEmitter->Think(dt, mgr);
  }
  mFluidIdsChanged = false;
  CEntity::Think(dt, mgr);
}

void CActor::SetTransformDirty() {
  mNotInSortedLists = true;
  mTransformDirty = true;
  mActorLightsDirty = true;
  mRenderBoundsDirty = true;
}

void CActor::SetTransform(const CTransform4f& xf) {
  mTransform = xf;
  mPosition = xf.GetTranslation();
  SetTransformDirty();
}

void CActor::UpdatePortalSystemState(CStateManager& mgr) {
  const TAreaId areaId = GetCurrentAreaId();
  if (mgr.GetWorld()->DoesAreaExist(areaId)) {
    CPortalArea* portal = mgr.GetWorld()->GetArea(areaId)->GetPostConstructed()->mPortalArea.get();
    if (portal != nullptr) {
      portal->UpdateActor(mgr, *this);
    }
  }
}

float CActor::GetDistanceToCamera(CStateManager& mgr) const {
  float distanceSquared = 3.4028235e38f;
  const CVector3f position = GetTranslation();
  for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
    const CGameCamera* camera = mgr.GetCameraManager(i)->GetCurrentCamera(mgr, true);
    const CVector3f delta = camera->GetTranslation() - position;
    const float cameraDistanceSquared = delta.MagSquared();
    if (cameraDistanceSquared < distanceSquared) {
      distanceSquared = cameraDistanceSquared;
    }
  }
  return distanceSquared;
}

CActor::SSound::SSound(const CSfxHandle& handle, const CSegId& locator, bool useEchoVolume)
: mHandle(handle), mLocator(locator), mUseEchoVolume(useEchoVolume) {}
