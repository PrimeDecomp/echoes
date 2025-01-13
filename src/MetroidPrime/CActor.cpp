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

CActor::CActor(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, int inGrave,
               const CTransform4f& xf, const CModelData& mData, const CMaterialList& list,
               const CActorParameters& params, TUniqueId nextDrawNode)
: CEntity(uid, info, name, inGrave | 2)
, m_transform(xf)
, m_position(xf.GetTranslation())
, m_modelData(mData.IsNull() ? nullptr : new CModelData(mData))
, m_material(MakeActorMaterialList(list, params))
, x70_materialFilter(
      CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()))
, x88_sfxId(InvalidSfxId)
, x90_actorLights(mData.IsNull() ? nullptr : params.GetLighting().MakeActorLights().release())
, otherBounds(CAABox::MakeMaxInvertedBox())
, m_renderBounds(CAABox::MakeMaxInvertedBox())
, xb4_drawFlags(CModelFlags::Normal())
, xbc_time(0.f)
, xc0_pitchBend(8192)
, xc4_fluidId(kInvalidUniqueId)
, xc6_nextDrawNode(nextDrawNode)
, xc8_drawnToken(-1)
, xcc_addedToken(-1)
// , xd0_damageMag(params.GetThermalMag())
, xd4_maxVol(CAudioSys::kMaxVolume)
, xd8_nonLoopingSfxHandles(CSfxHandle())
, m_nextNonLoopingSfxHandle(0)
, m_notInSortedLists(true)
, m_transformDirty(true)
, m_actorLightsDirty(true)
, m_outOfFrustum(false)
, m_calculateLighting(true)
, m_shadowEnabled(false)
, m_shadowDirty(false)
, m_muted(false)
, m_useInSortedLists(true)
, m_callTouch(true)
, m_globalTimeProvider(params.UseGlobalRenderTime())
, m_renderUnsorted(params.ForceRenderUnsorted())
, m_pointGeneratorParticles(false)
, m_fluidCounter(0)
// , m_thermalVisorFlags(params.IsHotInThermal() ? kTF_Hot : kTF_Cold)
, m_renderParticleDBInside(true)
, m_enablePitchBend(false)
, m_targetableVisorFlags(params.GetVisorParameters().GetMask())
, m_enableRender(true)
, m_worldLightingDirty(false)
, m_drawEnabled(true)
, m_doTargetDistanceTest(true)
, m_targetable(true) {
  if (!m_modelData.null()) {
    if (params.GetXRay().first != 0) {
      m_modelData->SetXRayModel(params.GetXRay());
    }
    if (params.GetInfra().first != 0) {
      m_modelData->SetInfraModel(params.GetInfra());
    }
    const CLightParameters& lighting = params.GetLighting();
    if (!lighting.ShouldMakeLights() || lighting.GetMaxAreaLights() == 0) {
      m_modelData->SetAmbientColor(lighting.GetAmbientColor());
    }
    m_modelData->SetSortThermal(!params.NoSortThermal());
  }
  const CAssetId scanId = params.GetScannable().GetScannableObject0();
  if (scanId != kInvalidAssetId) {
    x98_scanObjectInfo =
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
  if (CSfxHandle handle = x8c_loopingSfxHandle) {
    CSfxManager::RemoveEmitter(handle);
    x88_sfxId = -1;
    x8c_loopingSfxHandle = CSfxHandle();
  }
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

void CActor::CalculateRenderBounds(CStateManager& mgr) {
  if (HasModelData()) {
    CAABox bounds = GetModelData()->GetBounds(GetTransform());
    SetRenderBounds(bounds);
    if (GetModelData()->HasAnimation()) {
      rstl::optional_object< CAABox > new_bounds =
          GetModelData()->GetAnimationData()->GetParticleDB().GetBounds();
      if (new_bounds) {
        bounds.AccumulateBounds(new_bounds->GetMinPoint());
        bounds.AccumulateBounds(new_bounds->GetMaxPoint());
      }
    }
    otherBounds = bounds;
  } else {
    const CVector3f origin = GetTranslation();
    SetRenderBounds(CAABox(origin, origin));
    otherBounds = CAABox(origin, origin);
  }
  if (m_renderBoundsDirty) {
    fn_80049ED8(this, mgr);
    m_renderBoundsDirty = 0;
  }
}

void CActor::SetModelData(const CModelData& data, CStateManager& mgr) {
  if (data.IsNull()) {
    if (GetModelData() && GetModelData()->HasAnimation()) {
      AnimationData()->GetParticleDB().DeleteAllLights(mgr);
    }
    m_modelData = nullptr;
  } else {
    m_modelData = new CModelData(data);
  }
}

// TODO nonmatching
void CActor::PreRender(CStateManager& mgr, const CFrustumPlanes& planes) {
  int x = mgr.fn_800366e4(this);

  if (HasModelData()) {
    SetPreRenderClipped(!planes.BoxInFrustumPlanes(m_renderBounds));
    if (!GetPreRenderClipped()) {
      bool lightsDirty = false;
      if (GetPreRenderHasMoved()) {
        SetPreRenderHasMoved(false);
        SetShadowDirty(true);
        lightsDirty = true;
      } else if (m_worldLightingDirty) {
        lightsDirty = true;
      } else if (HasActorLights() && GetActorLights()->GetNeedsRelight()) {
        lightsDirty = true;
      }

      // TODO why doesn't GetDrawShadow() work?
      if (GetShadowDirty() && m_shadowEnabled && HasShadow()) {
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
                m_worldLightingDirty = false;
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
      if (GetShadowDirty() && m_shadowEnabled && HasShadow()) {
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
    if (m_material.HasMaterial(kMT_ScanPassthrough) == false) {
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
      gpRender->UnkH(value);
    }
    GetModelData()->RenderUnsortedParts(which, GetTransform(), GetActorLights(), GetModelFlags());

    if (value != -1) {
      gpRender->UnkI();
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
    result = m_renderUnsorted || IsModelOpaque(mgr);
  }
  return result;
}

void CActor::Render(const CStateManager& mgr) const {
  if (GetModelData() && !NullModel()) {
    bool renderPrePostParticles = GetRenderParticleDatabaseInside() && HasAnimation();
    if (renderPrePostParticles) {
      GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnFirst();
    }

    if (m_enableRender) {
      if (m_pointGeneratorParticles) {
        mgr.SetupParticleHook(*this);
      }
      if (m_globalTimeProvider) {
        RenderInternal(mgr);
      } else {
        const float timeSince = CGraphics::GetSecondsMod900() - xbc_time;
        CTimeProvider tp(CMath::FastFmod(timeSince, 900.f));
        RenderInternal(mgr);
      }
      if (m_pointGeneratorParticles) {
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
    gpRender->UnkH(value);
  }
  GetModelData()->Render(which, GetTransform(), GetActorLights(), GetModelFlags());

  if (value != -1) {
    gpRender->UnkI();
  }
}

float CActor::GetYaw() const {
  float sq = CMath::SqrtF(m_transform.Get11() * m_transform.Get11() +
                          m_transform.Get01() * m_transform.Get01());
  if (sq > 0.001f) {
    double ret = -atan2(m_transform.Get01(), m_transform.Get11());
    return ret;
  }
  return 0.f;
}

CHealthInfo* CActor::HealthInfo(CStateManager& mgr) { return nullptr; }

float CActor::GetPitch() const {
  float sq = CMath::SqrtF(m_transform.Get11() * m_transform.Get11() +
                          m_transform.Get01() * m_transform.Get01());
  double ret = -atan2(-m_transform.Get21(), sq);
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

bool CActor::GetUseInSortedLists() const { return m_useInSortedLists; }

void CActor::SetUseInSortedLists(bool use) { m_useInSortedLists = use; }

bool CActor::GetCallTouch() const { return m_callTouch; }

void CActor::SetCallTouch(bool value) { m_callTouch = value; }

void CActor::AddMaterial(EMaterialTypes mat1, CStateManager& mgr) {
  m_material.Add(mat1);
  mgr.UpdateObjectInLists(*this);
}

void CActor::AddMaterial(EMaterialTypes mat1, EMaterialTypes mat2, CStateManager& mgr) {
  m_material.Add(mat1);
  m_material.Add(mat2);
  mgr.UpdateObjectInLists(*this);
}

void CActor::AddMaterial(EMaterialTypes mat1, EMaterialTypes mat2, EMaterialTypes mat3,
                         CStateManager& mgr) {
  m_material.Add(mat1);
  m_material.Add(mat2);
  m_material.Add(mat3);
  mgr.UpdateObjectInLists(*this);
}

void CActor::AddMaterial(EMaterialTypes mat1, EMaterialTypes mat2, EMaterialTypes mat3,
                         EMaterialTypes mat4, CStateManager& mgr) {
  m_material.Add(mat1);
  m_material.Add(mat2);
  m_material.Add(mat3);
  m_material.Add(mat4);
  mgr.UpdateObjectInLists(*this);
}

void CActor::AddMaterial(EMaterialTypes mat1, EMaterialTypes mat2, EMaterialTypes mat3,
                         EMaterialTypes mat4, EMaterialTypes mat5, CStateManager& mgr) {
  m_material.Add(mat1);
  m_material.Add(mat2);
  m_material.Add(mat3);
  m_material.Add(mat4);
  m_material.Add(mat5);
  mgr.UpdateObjectInLists(*this);
}

void CActor::RemoveMaterial(EMaterialTypes mat1, CStateManager& mgr) {
  m_material.Remove(mat1);
  mgr.UpdateObjectInLists(*this);
}

void CActor::RemoveMaterial(EMaterialTypes mat1, EMaterialTypes mat2, CStateManager& mgr) {

  m_material.Remove(mat1);
  m_material.Remove(mat2);
  mgr.UpdateObjectInLists(*this);
}

void CActor::RemoveMaterial(EMaterialTypes mat1, EMaterialTypes mat2, EMaterialTypes mat3,
                            CStateManager& mgr) {

  m_material.Remove(mat1);
  m_material.Remove(mat2);
  m_material.Remove(mat3);
  mgr.UpdateObjectInLists(*this);
}

void CActor::RemoveMaterial(EMaterialTypes mat1, EMaterialTypes mat2, EMaterialTypes mat3,
                            EMaterialTypes mat4, CStateManager& mgr) {

  m_material.Remove(mat1);
  m_material.Remove(mat2);
  m_material.Remove(mat3);
  m_material.Remove(mat4);
  mgr.UpdateObjectInLists(*this);
}

void CActor::RemoveMaterial(EMaterialTypes mat1, EMaterialTypes mat2, EMaterialTypes mat3,
                            EMaterialTypes mat4, EMaterialTypes mat5, CStateManager& mgr) {

  m_material.Remove(mat1);
  m_material.Remove(mat2);
  m_material.Remove(mat3);
  m_material.Remove(mat4);
  m_material.Remove(mat5);
  mgr.UpdateObjectInLists(*this);
}

void CActor::SetMaterialList(const CMaterialList& l, CStateManager& mgr) {
  m_material = l;
  mgr.UpdateObjectInLists(*this);
}

EWeaponCollisionResponseTypes CActor::GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                               const CWeaponMode&, int) const {
  return kWCR_OtherProjectile;
}

CVector3f CActor::GetOrbitPosition(const CStateManager&) const { return m_position; }

CVector3f CActor::GetAimPosition(const CStateManager&, float) const { return m_position; }

CVector3f CActor::GetHomingPosition(const CStateManager& mgr, float f) const {
  return GetAimPosition(mgr, f);
}

CVector3f CActor::GetScanObjectIndicatorPosition(const CStateManager& mgr) const {
  return GetOrbitPosition(mgr);
}

bool CActor::IsModelOpaque(const CStateManager& mgr) const {
  if (m_pointGeneratorParticles) {
    return false;
  } else if (!HasModelData()) {
    return true;
  } else if (static_cast< char >(xb4_drawFlags.GetTrans()) > 4) {
    return false;
  } else {
    CModelData::EWhichModel which = CModelData::GetRenderingModel(mgr);
    return m_modelData->IsDefinitelyOpaque(which);
  }
}

void CActor::SetCalculateLighting(bool b) {
  if (b && x90_actorLights.null()) {
    x90_actorLights = new CActorLights(8, CVector3f::Zero(), 4, 4);
  }
  m_calculateLighting = b;
}

void CActor::SetActorLights(rstl::auto_ptr< CActorLights > lights) {
  x90_actorLights = lights.release();
  m_calculateLighting = true;
}

const CMaterialFilter& CActor::GetMaterialFilter() const { return x70_materialFilter; }

void CActor::SetMaterialFilter(const CMaterialFilter& filter) { x70_materialFilter = filter; }

void CActor::SetActive(const bool active) {
  if (m_drawEnabled != active) {
    SetDirtyFlags();
    m_drawEnabled = active; // no setter?
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
      xbc_time = CGraphics::GetSecondsMod900();
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
      AnimationData()->GetParticleDB().DeleteAllLights(mgr);
    }
    // if (field_0x130) {
    //   (field25_0xa4->vtable[3])(mgr);
    // }
    break;
  }
  case kSM_XCRT: {
    if (!x98_scanObjectInfo.null()) {
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
      if (act != nullptr && xc6_nextDrawNode == kInvalidUniqueId) {
        xc6_nextDrawNode = act->GetUniqueId();
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
  if (x98_scanObjectInfo.null()) {
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

  actor->x98_scanObjectInfo = x98_scanObjectInfo;
  actor->AddMaterial(kMT_Scannable, mgr);
  RemoveMaterial(kMT_Scannable, mgr);
}

void CActor::SetMuted(bool b) {
  m_muted = b;
  RemoveEmitter();
}

void CActor::SetVolume(uchar volume) {
  if (CSfxHandle handle = x8c_loopingSfxHandle) {
    CSfxManager::UpdateEmitter(handle, GetTranslation(), CVector3f::Zero(), volume);
  }
  xd4_maxVol = volume;
}

void CActor::SetSoundEventPitchBend(int v) {
  m_enablePitchBend = true;
  xc0_pitchBend = v;
  if (x8c_loopingSfxHandle) {
    CSfxManager::PitchBend(x8c_loopingSfxHandle, v);
  }
}

CSfxHandle CActor::GetSfxHandle() const { return x8c_loopingSfxHandle; }

// void CActor::SetInFluid(bool in, TUniqueId uid) {
//   if (in) {
//     m_fluidCounter += 1;
//     xc4_fluidId = uid;
//   } else if (m_fluidCounter != 0) {
//     m_fluidCounter--;
//     if (m_fluidCounter == 0) {
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
  parms.x0_pos = position;
  parms.xc_dir = CVector3f::Zero();
  parms.x24_sfxId = id;

  bool useAcoustics = (flags & 0x80) == 0;
  bool looping = (sfxId & 0x80000000) != 0;
  bool nonEmitter = (sfxId & 0x40000000) != 0;

  // if (mgr.Random()->Float() > weight) {
  //   return;
  // }

  if (looping) {
    ushort curId = x88_sfxId;
    if (!x8c_loopingSfxHandle) {
      CSfxHandle handle;
      if (nonEmitter) {
        handle = CSfxManager::SfxStart(id, 1.f, 0.f, true, CSfxManager::kMedPriority, true, aid);
      } else {
        handle = CSfxManager::AddEmitter(parms, useAcoustics, CSfxManager::kMedPriority, true, aid);
      }
      if (handle) {
        x88_sfxId = id;
        x8c_loopingSfxHandle = handle;
        if (m_enablePitchBend) {
          CSfxManager::PitchBend(handle, xc0_pitchBend);
        }
      }
    } else if (curId == id) {
      CSfxManager::UpdateEmitter(x8c_loopingSfxHandle, parms.x0_pos, parms.xc_dir, maxVol);
    } else if (flags & 0x4) {
      CSfxManager::RemoveEmitter(x8c_loopingSfxHandle);
      CSfxHandle handle =
          CSfxManager::AddEmitter(parms, useAcoustics, CSfxManager::kMedPriority, true, aid);
      if (handle) {
        x88_sfxId = id;
        x8c_loopingSfxHandle = handle;
        if (m_enablePitchBend) {
          CSfxManager::PitchBend(handle, xc0_pitchBend);
        }
      }
    }
  } else {
    CSfxHandle handle;
    if (nonEmitter) {
      handle =
          CSfxManager::SfxStart(id, 1.f, 0.f, useAcoustics, CSfxManager::kMedPriority, false, aid);
    } else {
      handle = CSfxManager::AddEmitter(parms, useAcoustics, CSfxManager::kMedPriority, false, aid);
    }
    if ((sfxId & 0x20000000) != 0 /* continuous update */) {
      xd8_nonLoopingSfxHandles[m_nextNonLoopingSfxHandle] = handle;
      m_nextNonLoopingSfxHandle = (m_nextNonLoopingSfxHandle + 1) % xd8_nonLoopingSfxHandles.size();
    }

    if (m_enablePitchBend) {
      CSfxManager::PitchBend(handle, xc0_pitchBend);
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
  m_transform.SetTranslation(vec);
  SetTransformDirty(true);
  SetTransformDirtySpare(true);
  SetPreRenderHasMoved(true);
}
