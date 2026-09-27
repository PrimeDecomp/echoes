#include "MetroidPrime/CActor.hpp"

#include "Kyoto/Math/CAABox.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CActorModelParticles.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CSimpleShadow.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/TCastTo.hpp"

// #include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CTimeProvider.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CMath.hpp"

#include "MetroidPrime/TGameTypes.hpp"
#include "dolphin/types.h"
#include "rstl/math.hpp"

void fn_80049ED8(CActor*, CStateManager&);
extern "C" void fn_801ECD8C(CActor*, CStateManager&);

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
, mModelData(mData.IsNull() ? nullptr : new CModelData(mData))
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
, x134_(-1)
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
, x151_5_(true)
, mCallTouch(true)
, mGlobalTimeProvider(params.UseGlobalRenderTime())
, mRenderUnsorted(params.IsHotInThermal())
, mPointGeneratorParticles(false)
, mRenderParticleDBInside(true)
, mEnablePitchBend(false)
, mTargetableVisorFlags(params.GetVisorParameters().GetMask())
, mEnableRender(true)
, mWorldLightingDirty(false)
, mDrawEnabled(true)
, mDoTargetDistanceTest(true)
, x153_4_(true)
, x153_5_(true)
, mTargetable(true)
, x153_7_(true)
, x154_0_(false)
, x154_1_(params.ForceRenderUnsorted())
, x154_2_(false)
, x154_3_(params.NoSortThermal())
, mLoopingSoundCount(0) {
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
    mScanObjectInfo =
        new TCachedToken< CScannableObjectInfo >(
          gpSimplePool->GetObj(SObjectTag('SCAN', scanId)),
          true
        );
  }
}

CActor::~CActor() { RemoveEmitter(); }

CAdvancementDeltas CActor::UpdateAnimation(float dt, CStateManager& mgr, bool advTree) {
  CAdvancementDeltas result = ModelData()->AdvanceAnimation(dt, mgr, GetCurrentAreaId(), advTree);
  ModelData()->AdvanceParticles(GetTransform(), dt, mgr);
  UpdateSfxEmitters();
  if (HasAnimation()) {
    // ushort maxVol = xd4_maxVol;
    // int aid = GetCurrentAreaId().Value();

    // const CGameCamera& camera = mgr.GetCameraManager()->GetCurrentCamera(mgr);
    // const CVector3f origin = GetTranslation();
    // const CVector3f toCamera = camera.GetTranslation() - origin;

    // const CInt32POINode* intNode;
    // const CSoundPOINode* soundNode;
    // const CParticlePOINode* particleNode;

    // int soundNodeCount = 0;
    // if (HasAnimation()) {
    //   soundNode = GetAnimationData()->GetSoundPOIList(soundNodeCount);
    // } else {
    //   soundNode = nullptr;
    // }
    // if (soundNodeCount > 0 && soundNode != nullptr) {
    //   for (int i = 0; i < soundNodeCount; ++soundNode, ++i) {
    //     int charIdx = soundNode->GetCharacterIndex();
    //     if (soundNode->GetPoiType() != kPT_Sound || GetMuted())
    //       continue;
    //     if (charIdx != -1 && GetAnimationData()->GetCharacterIndex() != charIdx)
    //       continue;
    //     ProcessSoundEvent(soundNode->GetSoundId(), soundNode->GetWeight(), soundNode->GetFlags(),
    //                       soundNode->GetFallOff(), soundNode->GetMaxDistance(), 20, maxVol,
    //                       toCamera, origin, aid, mgr, true);
    //   }
    // }

    // int intNodeCount = 0;
    // if (HasAnimation()) {
    //   intNode = GetAnimationData()->GetInt32POIList(intNodeCount);
    // } else {
    //   intNode = nullptr;
    // }
    // if (intNodeCount > 0 && intNode != nullptr) {
    //   for (int i = 0; i < intNodeCount; ++intNode, ++i) {
    //     int charIdx = intNode->GetCharacterIndex();
    //     if (intNode->GetPoiType() == kPT_SoundInt32 && !GetMuted() &&
    //         (charIdx == -1 || GetAnimationData()->GetCharacterIndex() == charIdx)) {
    //       ProcessSoundEvent(intNode->GetValue(), intNode->GetWeight(), intNode->GetFlags(), 0.1f,
    //                         150.f, 20, maxVol, toCamera, origin, aid, mgr, true);
    //     } else if (intNode->GetPoiType() == kPT_UserEvent) {
    //       DoUserAnimEvent(mgr, *intNode, static_cast< EUserEventType >(intNode->GetValue()), dt);
    //     }
    //   }
    // }

    // int particleNodeCount = 0;
    // if (HasAnimation()) {
    //   particleNode = GetAnimationData()->GetParticlePOIList(particleNodeCount);
    // } else {
    //   particleNode = nullptr;
    // }
    // if (particleNodeCount > 0 && particleNode != nullptr) {
    //   for (int i = 0; i < particleNodeCount; ++particleNode, ++i) {
    //     int charIdx = particleNode->GetCharacterIndex();
    //     if (charIdx != -1 && GetAnimationData()->GetCharacterIndex() != charIdx)
    //       continue;
    //     AnimationData()->SetParticleEffectState(particleNode->GetString(), true, mgr);
    //   }
    // }
  }
  return result;
}

void CActor::RemoveEmitter() {
  for (uint i = 0; i < mLoopingSoundCount; ++i) {
    TLoopingSound& sound = mLoopingSounds[i];
    if (const CSfxHandle& handle = sound.second.mHandle) {
      CSfxManager::RemoveEmitter(handle);
      sound.first = InvalidSfxId;
      sound.second = SSound(CSfxHandle(), CSegId::Invalid(), false);
    }
  }
  mLoopingSoundCount = 0;
}

void CActor::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                             float dt) {
  if (type == kUE_LoopedSoundStop) {
    RemoveEmitter();
  }
}

float CActor::GetAverageAnimVelocity(int anim) {
  return HasAnimation() ? GetAnimationData()->GetAverageVelocity(anim) : 0.f;
}

void CActor::PreRenderAllViewports(CStateManager& mgr) {
  if (HasModelData()) {
    CAABox bounds = GetModelData()->GetBounds(GetTransform());
    SetRenderBounds(bounds);
    if (GetModelData()->HasAnimation()) {
      rstl::optional_object< CAABox > new_bounds =
          GetModelData()->GetAnimationData()->GetParticleDB().GetTotalBounds();
      if (new_bounds) {
        bounds.AccumulateBounds(new_bounds->GetMinPoint());
        bounds.AccumulateBounds(new_bounds->GetMaxPoint());
      }
    }
    mOtherBounds = bounds;
  } else {
    const CVector3f origin = GetTranslation();
    SetRenderBounds(CAABox(origin, origin));
    mOtherBounds = CAABox(origin, origin);
  }
  if (mRenderBoundsDirty) {
    fn_80049ED8(this, mgr);
    mRenderBoundsDirty = 0;
  }
}

void CActor::SetModelData(const CModelData& data, CStateManager& mgr) {
  if (data.IsNull()) {
    if (GetModelData() && GetModelData()->HasAnimation()) {
      AnimationData()->GetParticleDB().DeleteAllLights(&mgr);
    }
    mModelData = nullptr;
  } else {
    mModelData = new CModelData(data);
  }
}

// TODO nonmatching
void CActor::PreRender(CStateManager& mgr) {
  const CFrustumPlanes& planes = mgr.GetFrustumPlanes();
  int x = mgr.fn_800366e4(this);

  if (HasModelData()) {
    SetPreRenderClipped(!planes.BoxInFrustumPlanes(mRenderBounds));
    if (!GetPreRenderClipped()) {
      bool lightsDirty = false;
      if (GetPreRenderHasMoved()) {
        SetPreRenderHasMoved(false);
        SetShadowDirty(true);
        lightsDirty = true;
      } else if (mWorldLightingDirty) {
        lightsDirty = true;
      } else if (HasActorLights() && GetActorLights()->GetNeedsRelight()) {
        lightsDirty = true;
      }

      // TODO why doesn't GetDrawShadow() work?
      if (GetShadowDirty() && mShadowEnabled && HasShadow()) {
        // Shadow()->Calculate(GetModelData()->GetBounds(), GetTransform(), mgr);
        SetShadowDirty(false);
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
    } else {
      if (GetPreRenderHasMoved()) {
        SetPreRenderHasMoved(false);
        SetShadowDirty(true);
      }
      // TODO why doesn't GetDrawShadow() work?
      if (GetShadowDirty() && mShadowEnabled && HasShadow()) {
        // if (planes.BoxInFrustumPlanes(
        //         GetShadow()->GetMaxShadowBox(GetModelData()->GetBounds(GetTransform()))) == true)
        //         {
        //   Shadow()->Calculate(GetModelData()->GetBounds(), GetTransform(), mgr);
        //   SetShadowDirty(false);
        // }
      }
    }
  }
}

bool CActor::fn_8004CD00(const CStateManager& mgr) const {
  return GetDrawShadow() && mgr.Get0x244c() == 0;
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
      if (fn_8004CD00(mgr)) {
        if (GetShadow()->Valid() &&
            mgr.GetFrustumPlanes().BoxInFrustumPlanes(GetShadow()->GetBounds())) {
          gpRender->AddDrawable(GetShadow(), GetShadow()->GetTransform().GetTranslation(),
                                GetShadow()->GetBounds(), 1, IRenderer::kDS_SortedCallback);
        }
      }
    }
  }
}

int CActor::fn_8004CAA0(const CStateManager& mgr) const {
  int result;
  switch (mgr.GetPlayerState(0)->GetActiveVisor(mgr)) {
  case CPlayerState::kPV_Dark:
    // TODO: flag magic
    result = -1;
    break;
  case CPlayerState::kPV_Scan:
    if (mMaterial.HasMaterial(kMT_ScanPassthrough) == false) {
      // TODO this call is weird
      result = mgr.fn_801EDD8C(GetUniqueId()) << 2;
    } else {
      result = -1;
    }
    break;
  default:
    result = -1;
  }
  return result;
}

void CActor::EnsureRendered(const CStateManager& mgr, const CVector3f& pos,
                            const CAABox& bounds) const {
  if (GetModelData()) {
    const CModelData::EWhichModel which = CModelData::GetRenderingModel(mgr);
    int value = fn_8004CAA0(mgr);
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

void CActor::DrawTouchBounds() const {}

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
  int value = fn_8004CAA0(mgr);
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
    double ret = -atan2(mTransform.Get01(), mTransform.Get11());
    return ret;
  }
  return 0.f;
}

CHealthInfo* CActor::HealthInfo() { return nullptr; }

float CActor::GetPitch() const {
  float sq = CMath::SqrtF(mTransform.Get11() * mTransform.Get11() +
                          mTransform.Get01() * mTransform.Get01());
  double ret = -atan2(-mTransform.Get21(), sq);
  return ret;
}

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
    mActorLights = new CActorLights(8, CVector3f::Zero(), 4, 4);
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
    SetDirtyFlags();
    mDrawEnabled = active; // no setter?
  }
  CEntity::SetActive(active);
}

// void CActor::SetDirtyFlags() {
//   SetTransformDirty(true);
//   SetTransformDirtySpare(true);
//   SetPreRenderHasMoved(true);
// }

void CActor::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Activate: {
    if (!GetActive()) {
      mTime = CGraphics::GetSecondsMod900();
    }
    break;
  }
  case kSM_Deactivate: {
    RemoveEmitter();
    break;
  }
  case kSM_XDelete: {
    RemoveEmitter();
    if (HasModelData() && AnimationData() != nullptr) {
      AnimationData()->GetParticleDB().DeleteAllLights(&mgr);
    }
    // if (field_0x130) {
    //   (field25_0xa4->vtable[3])(mgr);
    // }
    break;
  }
  case kSM_XCRT: {
    if (!mScanObjectInfo.null()) {
      AddMaterial(kMT_Scannable, mgr);
    } else {
      RemoveMaterial(kMT_Scannable, mgr);
    }
    if (HasAnimation()) {
      AnimationData()->InitializeEffects(mgr, GetCurrentAreaId(), GetModelData()->GetScale());
    }
    // if field_0x130
    fn_801ECD8C(this, mgr); // method of field25_0xa4
    break;
  }
  case kSM_XALD: {
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
    SendScriptMsgs(kSS_ScanProcessing, mgr, kInvalidUniqueId, kSM_None);
    break;
  case kSS_Processing:
    SendScriptMsgs(kSS_ScanStart, mgr, kInvalidUniqueId, kSM_None);
    break;
  case kSS_Done:
    SendScriptMsgs(kSS_ScanDone, mgr, kInvalidUniqueId, kSM_None);
    break;
  }
}

CScannableObjectInfo* CActor::GetScannableObjectInfo() const {
  if (mScanObjectInfo.null()) {
    return nullptr;
  }

  // if (**x98_scanObjectInfo->IsLoaded()) {
  //   return x98_scanObjectInfo->GetObject();
  // }

  return nullptr;
}

void CActor::MoveScannableObjectInfoToActor(CActor* actor, CStateManager& mgr) {
  if (actor == nullptr) {
    return;
  }

  actor->mScanObjectInfo = mScanObjectInfo;
  actor->AddMaterial(kMT_Scannable, mgr);
  RemoveMaterial(kMT_Scannable, mgr);
}

void CActor::SetMuted(bool b) {
  mMuted = b;
  RemoveEmitter();
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

CSfxHandle CActor::GetSfxHandle() const { return mLoopingSounds[0].second.mHandle; }

// void CActor::SetInFluid(bool in, TUniqueId uid) {
//   if (in) {
//     mFluidCounter += 1;
//     xc4_fluidId = uid;
//   } else if (mFluidCounter != 0) {
//     mFluidCounter--;
//     if (mFluidCounter == 0) {
//       xc4_fluidId = kInvalidUniqueId;
//     }
//   }
// }

// TODO nonmatching
void CActor::ProcessSoundEvent(int sfxId, float weight, int flags, float fallOff, float maxDist,
                               uchar minVol, uchar maxVol, const CVector3f& toListener,
                               const CVector3f& position, int aid, CStateManager& mgr,
                               bool translateId) {
  if (toListener.MagSquared() >= maxDist * maxDist) {
    return;
  }
  ushort id = translateId ? CSfxManager::TranslateSFXID(static_cast< ushort >(sfxId))
                          : static_cast< ushort >(sfxId);

  uint musyxFlags = 0x1; // Continuous parameter update
  if (flags & 0x8) {
    musyxFlags |= 0x8; // Doppler FX
  }

  // TODO ctor?
  CAudioSys::C3DEmitterParmData parms(maxDist, fallOff, musyxFlags, maxVol, minVol);
  parms.mPos = position;
  parms.mDir = CVector3f::Zero();
  parms.mSfxId = id;

  bool useAcoustics = (flags & 0x80) == 0;
  bool looping = (sfxId & 0x80000000) != 0;
  bool nonEmitter = (sfxId & 0x40000000) != 0;

  // if (mgr.Random()->Float() > weight) {
  //   return;
  // }

  if (looping) {
    // TODO: Recover the Echoes ProcessSoundEvent signature and looping-sound helper.
    // This inherited implementation still handles only the first looping sound.
    TLoopingSound& sound = mLoopingSounds[0];
    ushort curId = sound.first;
    if (!sound.second.mHandle) {
      CSfxHandle handle;
      if (nonEmitter) {
        handle = CSfxManager::SfxStart(id, 127, 64, aid, true, true, CSfxManager::kMedPriority);
      } else {
        handle = CSfxManager::AddEmitter(parms, aid, useAcoustics, true, CSfxManager::kMedPriority);
      }
      if (handle) {
        sound.first = id;
        sound.second.mHandle = handle;
        mLoopingSoundCount = 1;
        if (mEnablePitchBend) {
          CSfxManager::PitchBend(handle, mPitchBend);
        }
      }
    } else if (curId == id) {
      CSfxManager::UpdateEmitter(sound.second.mHandle, parms.mPos, parms.mDir, maxVol);
    } else if (flags & 0x4) {
      CSfxManager::RemoveEmitter(sound.second.mHandle);
      CSfxHandle handle =
          CSfxManager::AddEmitter(parms, aid, useAcoustics, true, CSfxManager::kMedPriority);
      if (handle) {
        sound.first = id;
        sound.second.mHandle = handle;
        if (mEnablePitchBend) {
          CSfxManager::PitchBend(handle, mPitchBend);
        }
      }
    }
  } else {
    CSfxHandle handle;
    if (nonEmitter) {
      handle =
          CSfxManager::SfxStart(id, 127, 64, aid, useAcoustics, false, CSfxManager::kMedPriority);
    } else {
      handle = CSfxManager::AddEmitter(parms, aid, useAcoustics, false, CSfxManager::kMedPriority);
    }
    if ((sfxId & 0x20000000) != 0 /* continuous update */) {
      mNonLoopingSounds[mNextNonLoopingSfxHandle] = SSound(handle, CSegId::Invalid(), false);
      mNextNonLoopingSfxHandle = (mNextNonLoopingSfxHandle + 1) % mNonLoopingSounds.size();
    }

    if (mEnablePitchBend) {
      CSfxManager::PitchBend(handle, mPitchBend);
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
  SetTransformDirty(true);
  SetTransformDirtySpare(true);
  SetPreRenderHasMoved(true);
}

CActor::SSound::SSound(const CSfxHandle& handle, const CSegId& locator, bool useEchoVolume)
: mHandle(handle), mLocator(locator), mUseEchoVolume(useEchoVolume) {}
