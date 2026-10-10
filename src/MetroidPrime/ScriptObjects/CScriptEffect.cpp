#include "MetroidPrime/ScriptObjects/CScriptEffect.hpp"

#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CEffectWaypointPredicate.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Cameras/CScriptCameraSpline.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "rstl/math.hpp"

uint CScriptEffect::mNumParticlesDrawing = 0;
uint CScriptEffect::mNumParticlesUpdating = 0;

static inline CTransform4f StripTranslation(const CTransform4f& xf) {
  CTransform4f result = xf;
  result.SetTranslation(CVector3f::Zero());
  return result;
}

CScriptEffect::CScriptEffect(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    const CVector3f& scale, CAssetId effectId, bool noTimerUnlessAreaOccluded,
    bool rebuildSystemsOnActivate, bool emitting, bool useRateInverseCamDist,
    float rateInverseCamDist, float rateInverseCamDistRate, float duration,
    float durationResetWhileVisible, bool useRateCamDistRange, float rateCamDistRangeMin,
    float rateCamDistRangeMax, float rateCamDistRangeFarRate, bool combatVisorVisible,
    bool darkVisorVisible, bool echoVisorVisible, const CLightParameters& lightParameters,
    bool dieWhenSystemsDone, const CGameSplineDesc& spline, bool useLocalTranslation,
    bool destroyParticlesOnDeactivate, bool orientToSpline, ERenderOrder renderOrder)
: CActor(uid, name, info, 0, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None().WithAlphaSorting(true), kInvalidUniqueId)
, mLightId(kInvalidUniqueId)
, mEffectId(effectId)
, mRateInverseCamDist(rateInverseCamDist)
, mRateInverseCamDistSq(rateInverseCamDist * rateInverseCamDist)
, mRateInverseCamDistRate(rateInverseCamDistRate)
, mRateCamDistRangeMin(rateCamDistRangeMin)
, mRateCamDistRangeMax(rateCamDistRangeMax)
, mRateCamDistRangeFarRate(rateCamDistRangeFarRate)
, mRemTime(duration)
, mDuration(duration)
, mDurationResetWhileVisible(durationResetWhileVisible)
, mEffectLights(lightParameters.MakeActorLights().release())
, mTriggerId(kInvalidUniqueId)
, mDestroyDelayTimer(0.f)
, mSpline(spline.GetDuration(), spline.IsClosedLoop() ? CSpline::kF_LoopPosition : 0,
          spline.GetSpline(), SLdrSpline(), spline.GetType(), spline.GetType())
, mSplineTime(0.f)
, mEmitting(emitting)
, mEnable(emitting)
, mNoTimerUnlessAreaOccluded(noTimerUnlessAreaOccluded)
, mRebuildSystemsOnActivate(rebuildSystemsOnActivate)
, mUseRateInverseCamDist(useRateInverseCamDist)
, mCombatVisorVisible(combatVisorVisible)
, mDarkVisorVisible(darkVisorVisible)
, mEchoVisorVisible(echoVisorVisible)
, mAnyVisorVisible(echoVisorVisible && darkVisorVisible && combatVisorVisible)
, mUseRateCamDistRange(useRateCamDistRange)
, mDieWhenSystemsDone(dieWhenSystemsDone)
, mCanRender(false)
, mLoopSpline(spline.IsClosedLoop())
, mHasSpline(false)
, mUseLocalTranslation(useLocalTranslation)
, mDestroyParticlesOnDeactivate(destroyParticlesOnDeactivate)
, mOrientToSpline(orientToSpline)
, mRenderOrder(renderOrder) {
  if (effectId != kInvalidAssetId) {
    const FourCC type = gpResourceFactory->GetResourceTypeById(mEffectId);
    mDescription = rs_new CToken(gpSimplePool->GetObj(SObjectTag(type, mEffectId)));
    CreateSystem(scale, lightParameters.GetAmbientColor());
  }
  SetDrawEnabled(true);
}

void CScriptEffect::UpdateModelLighting() {
  if (mParticleSystem->Get4CharId() == 'PART') {
    static_cast< CElementGen* >(mParticleSystem.get())
        ->SetLeaveLightsEnabledForModelRender(!mEffectLights.null());
  }
}

void CScriptEffect::Think(float dt, CStateManager& mgr) {
  if (mgr.GetObjectById(mTriggerId) == nullptr) {
    mTriggerId = kInvalidUniqueId;
  }
  UpdateGeneratorRate(mgr);
  UpdateSpline(dt);

  if (GetTransformDirtySpare()) {
    if (!mParticleSystem.null()) {
      mParticleSystem->SetOrientation(StripTranslation(GetTransform()));
      if (mUseLocalTranslation) {
        mParticleSystem->SetTranslation(GetTranslation());
      } else {
        mParticleSystem->SetGlobalTranslation(GetTranslation());
      }
    }
    if (CActor* light = TCastToPtr< CActor >(mgr.ObjectById(mLightId))) {
      light->SetTransform(GetTransform());
    }
    SetTransformDirtySpare(false);
  }

  if (!mNoTimerUnlessAreaOccluded) {
    if (mRemTime <= 0.f) {
      return;
    }
  } else if (mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetOcclusionState() ==
             CGameArea::kOS_Occluded) {
    if (mRemTime <= 0.f) {
      return;
    }
  }
  mRemTime -= dt;

  if (mEnable) {
    if (!mParticleSystem.null()) {
      mParticleSystem->Update(dt);
      mNumParticlesUpdating +=
          mParticleSystem->Get4CharId() == 'PART'
              ? static_cast< CElementGen* >(mParticleSystem.get())->GetParticleCountAll()
              : mParticleSystem->GetParticleCount();
    }
    if (mLightId != kInvalidUniqueId) {
      if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mLightId))) {
        if (mEmitting) {
          light->SetLight(mParticleSystem->GetLight());
        }
      }
    }
    if (mDieWhenSystemsDone) {
      mDestroyDelayTimer += dt;
      if (mDestroyDelayTimer > 15.f) {
        mgr.DeleteObjectRequest(GetUniqueId());
        return;
      }
      if (IsSystemDeletable()) {
        mgr.DeleteObjectRequest(GetUniqueId());
        return;
      }
    }
  }
  if (!mParticleSystem.null()) {
    if (static_cast< char >(GetModelFlags().GetTrans()) != 0) {
      mParticleSystem->SetModulationColor(GetModelFlags().GetColorRef());
    } else {
      mParticleSystem->SetModulationColor(CColor(0xFFFFFFFF));
    }
  }
}

void CScriptEffect::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const bool oldEmitting = mEmitting;
  bool handled = false;
  switch (msg.GetMessage()) {
  case kSM_Activate:
    handled = true;
    if (!mEmitting) {
      SendScriptMsgs(kSS_Active, mgr);
    }
    mEmitting = true;
    if (mRebuildSystemsOnActivate && !mParticleSystem.null()) {
      const CColor color = mParticleSystem->GetModulationColor();
      const CVector3f scale = mParticleSystem->GetGlobalScale();
      CreateSystem(scale, color);
    }
    break;
  case kSM_Deactivate:
    handled = true;
    if (mEmitting) {
      SendScriptMsgs(kSS_Inactive, mgr);
    }
    if (mDestroyParticlesOnDeactivate && !mParticleSystem.null()) {
      mParticleSystem->DestroyParticles();
    }
    mEmitting = false;
    break;
  case kSM_ToggleActive: {
    handled = true;
    EScriptObjectMessage next = mEmitting ? kSM_Deactivate : kSM_Activate;
    AcceptScriptMsg(
        mgr, CScriptMsg(msg.GetSenderId(), msg.GetId(), next, msg.GetOriginator(), msg.GetState()));
    break;
  }
  case kSM_AreaLoaded: {
    for (int i = 0; i < GetConnectionList().size(); ++i) {
      const SConnection& conn = GetConnectionList()[i];
      if (conn.state == kSS_InheritBounds && conn.msg == kSM_Activate) {
        const CStateManager::TIdListResult ids = mgr.GetIdListForScript(conn.objId);
        for (CStateManager::TIdList::const_iterator it = ids.first; it != ids.second; ++it) {
          if (TCastToPtr< CScriptTrigger >(mgr.ObjectById(it->second))) {
            mTriggerId = it->second;
          }
        }
      }
    }
    if (CheckConnectedObject_if(mgr, kSS_Connect, kSM_Attach, CEffectWaypointPredicate()) !=
        kInvalidUniqueId) {
      mHasSpline = true;
      ScriptCameraSpline::Initialise(*this, kSS_Connect, kSM_Attach, kSS_CameraTarget, kSM_Follow,
                                     mgr, mSpline);
    }
    break;
  }
  case kSM_Create:
    if (!mParticleSystem.null() && mParticleSystem->SystemHasLight()) {
      mLightId = mgr.AllocateUniqueId();
      mgr.AddObject(rs_new CGameLight(mLightId, GetCurrentAreaId(), mEmitting, rstl::string_l(""),
                                      GetTransform(), GetUniqueId(), mParticleSystem->GetLight(),
                                      mEffectId, 1, 0.f));
    }
    break;
  case kSM_Clear:
    if (!mParticleSystem.null()) {
      mParticleSystem->DestroyParticles();
    }
    break;
  case kSM_Delete:
    if (mLightId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mLightId);
      mLightId = kInvalidUniqueId;
    }
    break;
  default:
    break;
  }
  if (!handled) {
    CActor::AcceptScriptMsg(mgr, msg);
  }
  CActor* light = TCastToPtr< CActor >(mgr.ObjectById(mLightId));
  mgr.SendScriptMsg(light, msg.GetSenderId(), msg.GetMessage());
  if (oldEmitting == mEmitting) {
    return;
  }
  if (mEmitting) {
    rstl::vector< TUniqueId > playIds;
    playIds.reserve(GetConnectionList().size());
    for (int i = 0; i < GetConnectionList().size(); ++i) {
      const SConnection& conn = GetConnectionList()[i];
      if (conn.state == kSS_Play && conn.msg == kSM_Activate) {
        const TUniqueId id = mgr.GetIdForScript(conn.objId);
        if (id != kInvalidUniqueId) {
          playIds.push_back(id);
        }
      }
    }
    if (!playIds.empty()) {
      const int index = static_cast< int >(0.99f * (mgr.Random()->Float() * playIds.size()));
      if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(playIds[index]))) {
        SetTransform(actor->GetTransform());
        if (light != nullptr) {
          light->SetTransform(actor->GetTransform());
        }
      }
    }
  }
  mEnable = true;
  if (!mParticleSystem.null()) {
    mParticleSystem->SetParticleEmission(mEmitting);
  }
  if (mEmitting) {
    mRemTime = rstl::max_val(mDuration, mRemTime);
  }
}

void CScriptEffect::AddToRenderer(const CStateManager& mgr) const {
  if (!GetPreRenderClipped() && mRenderOrder == kRO_Normal) {
    EnsureRendered(mgr);
  }
}

void CScriptEffect::Render(const CStateManager&) const {
  if (!mEffectLights.null()) {
    mEffectLights->ActivateLights();
  }
  if (!mParticleSystem.null()) {
    const int count =
        mParticleSystem->Get4CharId() == 'PART'
            ? static_cast< CElementGen* >(mParticleSystem.get())->GetParticleCountAll()
            : mParticleSystem->GetParticleCount();
    if (count > 0) {
      mNumParticlesDrawing += count;
      mParticleSystem->Render();
    }
  }
}

void CScriptEffect::UpdateGeneratorRate(CStateManager& mgr) {
  if (!mUseRateInverseCamDist && !mUseRateCamDistRange) {
    return;
  }
  float rate = 1.f;
  float distanceSq =
      (mgr.GetCameraManager(0)->GetCurrentCamera(mgr, true)->GetTranslation() - GetTranslation())
          .MagSquared();
  for (int i = 1; i < uint(mgr.GetNumPlayers()); ++i) {
    const float nextDistanceSq =
        (mgr.GetCameraManager(i)->GetCurrentCamera(mgr, true)->GetTranslation() - GetTranslation())
            .MagSquared();
    if (nextDistanceSq > distanceSq) {
      distanceSq = nextDistanceSq;
    }
  }
  const float distance = distanceSq > 0.001f ? CMath::FastSqrtF(distanceSq) : 0.f;
  if (mUseRateInverseCamDist && distanceSq < mRateInverseCamDistSq) {
    rate = (1.f - mRateInverseCamDistRate) * (distance / mRateInverseCamDist) +
           mRateInverseCamDistRate;
  }
  if (mUseRateCamDistRange) {
    const float range = mRateCamDistRangeMax - mRateCamDistRangeMin;
    const float t = rstl::min_val(1.f, rstl::max_val(0.f, distance - mRateCamDistRangeMin) / range);
    rate = (1.f - t) * rate + t * mRateCamDistRangeFarRate;
  }
  mParticleSystem->SetGeneratorRate(rate);
}

void CScriptEffect::PreRender(CStateManager& mgr) {
  bool visible = false;
  if (!mCanRender) {
    mRemTime = rstl::max_val(mDurationResetWhileVisible, mRemTime);
  } else {
    const CAABox& bounds = GetOtherBounds();
    if (mgr.IsActorVisible(*this)) {
      mRemTime = rstl::max_val(mDurationResetWhileVisible, mRemTime);
      visible = true;
      if (!mAnyVisorVisible) {
        switch (mgr.GetPlayerState()->GetActiveVisor(mgr)) {
        case CPlayerState::kPV_Combat:
        case CPlayerState::kPV_Scan:
          visible = mCombatVisorVisible;
          break;
        case CPlayerState::kPV_Echo:
          visible = mEchoVisorVisible;
          break;
        case CPlayerState::kPV_Dark:
          visible = mDarkVisorVisible;
          break;
        }
      }
      if (visible && !mEffectLights.null()) {
        const CVector3f center = bounds.GetCenterPoint();
        mEffectLights->BuildAreaLightList(mgr, mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()),
                                          CAABox(center, center));
        mEffectLights->BuildDynamicLightList(mgr, bounds);
      }
    }
  }
  SetPreRenderClipped(!visible);
  if (visible) {
    switch (mRenderOrder) {
    case kRO_Normal:
      break;
    case kRO_Queue2:
      mgr.RenderLast(GetUniqueId());
      break;
    case kRO_Queue1:
      mgr.RenderFirstSorted(GetUniqueId());
      break;
    }
  }
}

void CScriptEffect::ResetParticleCounts() {
  mNumParticlesDrawing = 0;
  mNumParticlesUpdating = 0;
}

bool CScriptEffect::IsSystemDeletable() const {
  return mParticleSystem.null() || mParticleSystem->IsSystemDeletable();
}

bool CScriptEffect::CanRenderUnsorted(const CStateManager&) const { return false; }

void CScriptEffect::PreRenderAllViewports(CStateManager& mgr) {
  const rstl::optional_object< CAABox > bounds =
      !mParticleSystem.null() ? mParticleSystem->GetBounds() : rstl::optional_object< CAABox >();
  if (bounds.valid()) {
    SetOtherBounds(*bounds);
    SetRenderBounds(*bounds);
    mCanRender = true;
  } else {
    const CVector3f translation = GetTranslation();
    const CAABox emptyBounds(translation, translation);
    SetOtherBounds(emptyBounds);
    SetRenderBounds(emptyBounds);
    mCanRender = false;
  }
  UpdatePortalSystemState(mgr);
}

CAABox CScriptEffect::GetSortingBounds(const CStateManager& mgr) const {
  if (mTriggerId != kInvalidUniqueId) {
    if (const CScriptTrigger* trigger =
            static_cast< const CScriptTrigger* >(mgr.GetObjectById(mTriggerId))) {
      return trigger->GetTriggerBoundsWR();
    }
  }
  return CActor::GetSortingBounds(mgr);
}

void CScriptEffect::SetActive(const bool active) {
  CActor::SetActive(active);
  SetDrawEnabled(true);
}

void CScriptEffect::CreateSystem(const CVector3f& scale, const CColor& color) {
  const FourCC type = gpResourceFactory->GetResourceTypeById(mEffectId);
  const CVector3f position = GetTransform().GetTranslation();
  mParticleSystem = CElementGen::ConstructChildParticleSystem(
      *mDescription, type, 0, CElementGen::kOSF_One, mEffectLights.get() != nullptr, mEmitting,
      mUseLocalTranslation ? position : CVector3f::Zero(), StripTranslation(GetTransform()),
      mUseLocalTranslation ? CVector3f::Zero() : position, CTransform4f::Identity(), scale, color,
      CVector3f(1.f, 1.f, 1.f));
  UpdateModelLighting();
}

void CScriptEffect::UpdateSpline(float dt) {
  if (mHasSpline && mEmitting) {
    mSplineTime += dt;
    if (mSplineTime >= mSpline.GetPositionSpline().GetDuration()) {
      mSplineTime = mLoopSpline ? 0.f : mSpline.GetPositionSpline().GetDuration();
    }
    const CVector3f position = mSpline.GetPositionByTime(mSplineTime);
    SetTranslation(position);
    if (mOrientToSpline) {
      const float time =
          mSpline.GetDuration() * mSpline.PositionTimeSpline().EvaluateAt(mSplineTime);
      const CVector3f forward = mSpline.GetPositionSpline().GetTangentByTime(time);
      if (forward.CanBeNormalized()) {
        CTransform4f xf = CTransform4f::Identity();
        xf.SetTranslation(GetTranslation());
        xf.SetColumn(kDY, forward);
        CVector3f planar = forward.DropZ();
        planar.Normalize();
        if (CVector3f::Dot(forward, planar) < 0.99999f) {
          const CQuaternion rotation = CQuaternion::LookAt(
              CUnitVector3f(planar), CUnitVector3f(forward), CRelAngle::FromRadians(M_2PIF));
          xf.SetColumn(kDZ, rotation.Transform(CVector3f::Up()));
        }
        xf.SetColumn(kDX, CVector3f::Cross(forward, xf.GetUp()));
        SetTransform(xf);
      }
    }
  }
}

void CScriptEffect::SetGlobalScale(const CVector3f& scale) {
  if (!mParticleSystem.null()) {
    mParticleSystem->SetGlobalScale(scale);
  }
}

CVector3f CScriptEffect::GetGlobalScale() const {
  if (!mParticleSystem.null()) {
    return mParticleSystem->GetGlobalScale();
  }
  return CVector3f(1.f, 1.f, 1.f);
}

void CScriptEffect::SetGlobalTranslation(const CVector3f& translation) {
  if (!mParticleSystem.null()) {
    mParticleSystem->SetGlobalTranslation(translation);
  }
}

CScriptEffect::~CScriptEffect() {}

CEntity* LoadEffect(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrEffect sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrEffect.inc"

  if (sldrThis.particleEffect == kInvalidAssetId) {
    return nullptr;
  }
  if (gpResourceFactory->GetResourceTypeById(sldrThis.particleEffect) == 0) {
    return nullptr;
  }
  const CGameSplineDesc spline(sldrThis.motionControlSpline,
                               CMotionSpline::ESplineType(sldrThis.motionSplineType.type),
                               sldrThis.motionSplineDuration, sldrThis.motionSplinePathLoops);
  LdrToEntityInfo(info, sldrThis.editorProperties);
  info.SetActive(true);
  return rs_new CScriptEffect(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name, info,
      LdrToTransform4f(sldrThis.editorProperties), sldrThis.editorProperties.transform.scale,
      sldrThis.particleEffect, sldrThis.unknown_0x3df5a489, sldrThis.restartOnActivate,
      sldrThis.editorProperties.active, sldrThis.unknown_0xee538174, sldrThis.unknown_0xa94b0efd,
      sldrThis.unknown_0x93756968, sldrThis.unknown_0x0b94597d, sldrThis.unknown_0xd0e8a496,
      sldrThis.unknown_0xa8bb6c61, sldrThis.unknown_0x7589d549, sldrThis.unknown_0xa7d7d767,
      sldrThis.unknown_0xfe69615c, sldrThis.visibleInScanOrNormal, sldrThis.visibleInDark,
      sldrThis.visibleInEcho, LdrToLightParameters(sldrThis.lighting), sldrThis.deleteWhenDone,
      spline, sldrThis.splineMovesParticleEmitterOnly, sldrThis.destroyParticlesOnDeactivate,
      sldrThis.adoptSplineOrientation, CScriptEffect::ERenderOrder(sldrThis.renderOrder));
}

typedef struct { float f0; float f4; float f8; unsigned char _pc[4]; } __mwdec_g_lbl_803DA900;
extern __mwdec_g_lbl_803DA900 lbl_803DA900;
extern "C" void fn_80082ED0() {
    lbl_803DA900.f0 = 1.0f;
    lbl_803DA900.f4 = 1.0f;
    lbl_803DA900.f8 = 1.0f;
}
