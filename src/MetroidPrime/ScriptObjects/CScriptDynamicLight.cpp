#include "MetroidPrime/ScriptObjects/CScriptDynamicLight.hpp"

#include "MetroidPrime/CEffectWaypointPredicate.hpp"

#include "Kyoto/Math/CGameSplineDesc.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CScriptCameraSpline.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDynamicLight.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "rstl/optional_object.hpp"

bool CScriptDynamicLight::SDescription::UsesWorld() const {
  return mKind != kLK_LocalAmbient &&
         (mLightSet == kLS_World || mLightSet == kLS_LayerOneAndWorld ||
          mLightSet == kLS_LayerTwoAndWorld || mLightSet == kLS_All);
}

bool CScriptDynamicLight::SDescription::UsesLayerOne() const {
  return mLightSet == kLS_LayerOne || mLightSet == kLS_BothLayers ||
         mLightSet == kLS_LayerOneAndWorld || mLightSet == kLS_All;
}

bool CScriptDynamicLight::SDescription::UsesLayerTwo() const {
  return mLightSet == kLS_LayerTwo || mLightSet == kLS_BothLayers ||
         mLightSet == kLS_LayerTwoAndWorld || mLightSet == kLS_All;
}

CScriptDynamicLight::CScriptDynamicLight(TUniqueId uid, TAreaId areaId, const CLight& light,
                                         const SDescription& description, const rstl::string& name,
                                         const CEntityInfo& info, const CTransform4f& xf,
                                         const CGameSplineDesc& spline)
: CGameLight(uid, areaId, info.GetActive(), name, xf, kInvalidUniqueId, light, 0, 0, 0.f, &info)
, mDescription(description)
, mIntensity(0.f)
, mIntensityTime(0.f)
, mFalloffTime(0.f)
, mSpotlightTime(0.f)
, mSplineTime(0.f)
, mSpline(spline.GetDuration(), spline.IsClosedLoop() ? CSpline::kF_LoopPosition : 0,
          spline.GetSpline(), CMayaSpline(), spline.GetType(), spline.GetType())
, mParentId(kInvalidUniqueId)
, mParentLocator(CSegId::Invalid())
, mParentTransform(CTransform4f::Identity())
, mTargetId(kInvalidUniqueId)
, mHasSpline(false)
, mSplineLoops(spline.IsClosedLoop())
, mWorld(description.UsesWorld())
, mLayerOne(description.UsesLayerOne())
, mLayerTwo(description.UsesLayerTwo())
, mHasParent(false)
, mUseParentLocator(false) {}

void CScriptDynamicLight::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    CGameLight::Think(dt, mgr);
    UpdateSpline(dt);
    UpdateLight(dt);
    UpdateParent(mgr);
    UpdateTarget(mgr);
  }
}

void CScriptDynamicLight::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_AreaLoaded:
    if (CheckConnectedObject_if(mgr, kSS_CameraPath, kSM_Attach, CEffectWaypointPredicate()) !=
        kInvalidUniqueId) {
      mHasSpline = true;
      ScriptCameraSpline::Initialise(*this, kSS_CameraPath, kSM_Attach, kSS_CameraTarget,
                                     kSM_Follow, mgr, mSpline);
    }
    FindLightReceivers(mgr);
    FindParent(mgr);
    FindTarget(mgr);
    UpdateLight(0.f);
    break;
  case kSM_Activate:
    break;
  case kSM_Create:
    break;
  case kSM_Delete:
    break;
  default:
    break;
  }
  CActor::AcceptScriptMsg(mgr, msg);
}

void CScriptDynamicLight::FindLightReceivers(CStateManager& mgr) {
  bool found = false;
  const rstl::vector< TUniqueId > receivers = FindConnectedObjects(mgr, kSS_Play, kSM_Activate);
  for (int i = 0; i < receivers.size(); ++i) {
    CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(receivers[i]));
    if (actor && actor->HasActorLights()) {
      actor->ActorLights()->AddExplicitLightId(GetUniqueId());
      found = true;
    }
  }
  if (found) {
    mLayerOne = false;
    mLayerTwo = false;
  }
}

void CScriptDynamicLight::FindParent(CStateManager& mgr) {
  const rstl::vector< TUniqueId > parents = FindConnectedObjects(mgr, kSS_Connect, kSM_Attach);
  for (int i = 0; i < parents.size(); ++i) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(parents[i]))) {
      mParentId = actor->GetUniqueId();
      mParentTransform = ConvertEditorEulerToTransform4f(mDescription.mParentRotation,
                                                         mDescription.mParentTranslation);
      mHasParent = true;
      mUseParentLocator = mDescription.mLocatorName.size() != 0;
      if (!mDescription.mUseParentRotation) {
        mParentTransform = actor->GetTransform() * mParentTransform;
        SetTransform(mParentTransform);
        mParentTransform.AddTranslation(actor->GetTranslation() * -1.f);
      }
      break;
    }
  }
}

void CScriptDynamicLight::FindTarget(CStateManager& mgr) {
  const rstl::vector< TUniqueId > targets = FindConnectedObjects(mgr, kSS_CameraTarget, kSM_Attach);
  for (int i = 0; i < targets.size(); ++i) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(targets[i]))) {
      mTargetId = actor->GetUniqueId();
      break;
    }
  }
}

// Guessed helpers for the looping spline timers and the color clamp.
static inline float UpdateSplineTimer(const CMayaSpline& spline, float& time, float dt,
                                      float duration, bool loops) {
  time += dt;
  if (time >= duration) {
    time = loops ? 0.f : duration;
  }
  return spline.EvaluateAt(time);
}

static inline float ClampToOne(float value) { return value < 1.f ? value : 1.f; }

void CScriptDynamicLight::UpdateLight(float dt) {
  if (!GetActive()) {
    return;
  }
  CLight& light = Light();
  const ELightKind kind = mDescription.mKind;
  const CMayaSpline& intensitySpline = mDescription.mIntensitySpline;
  mIntensity = UpdateSplineTimer(intensitySpline, mIntensityTime, dt,
                                 mDescription.mIntensityDuration, mDescription.mIntensityLoops);
  if (kind == kLK_LocalAmbient || kind == kLK_Directional || kind == kLK_Spot) {
    const float red = ClampToOne(mIntensity * mDescription.mColor.GetRed());
    const float green = ClampToOne(mIntensity * mDescription.mColor.GetGreen());
    const float blue = ClampToOne(mIntensity * mDescription.mColor.GetBlue());
    const float alpha = ClampToOne(mIntensity * mDescription.mColor.GetAlpha());
    const CColor color(red, green, blue, alpha);
    light.SetColor(color);
  }
  if (kind == kLK_Point || kind == kLK_Spot) {
    const CMayaSpline& falloffSpline = mDescription.mFalloffSpline;
    const float falloff = UpdateSplineTimer(
        falloffSpline, mFalloffTime, dt, mDescription.mFalloffDuration, mDescription.mFalloffLoops);
    const EFalloffType falloffType = mDescription.mFalloffType;
    switch (kind) {
    case kLK_Point:
      light.SetAngleAttenuation(mIntensity, 0.f, 0.f);
    case kLK_Spot:
      light.SetAttenuation(falloffType == kFT_Constant ? 1.f : 0.f,
                           falloffType == kFT_Linear ? falloff : 0.f,
                           falloffType == kFT_Quadratic ? falloff : 0.f);
      break;
    }
  }
  if (kind == kLK_Spot) {
    const CMayaSpline& spotlightSpline = mDescription.mSpotlightSpline;
    light.SetSpotCutoff(UpdateSplineTimer(spotlightSpline, mSpotlightTime, dt,
                                          mDescription.mSpotlightDuration,
                                          mDescription.mSpotlightLoops));
  }
}

void CScriptDynamicLight::UpdateSpline(float dt) {
  if (mHasSpline && GetActive()) {
    mSplineTime += dt;
    if (mSplineTime >= mSpline.GetPositionSpline().GetDuration()) {
      mSplineTime = mSplineLoops ? 0.f : mSpline.GetPositionSpline().GetDuration();
    }
    const CVector3f position = mSpline.GetPositionByTime(mSplineTime);
    SetTranslation(position);
  }
}

void CScriptDynamicLight::UpdateParent(CStateManager& mgr) {
  if (GetActive() && mParentId != kInvalidUniqueId) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mParentId))) {
      if (mUseParentLocator && mParentLocator == CSegId::Invalid() && actor->HasModelData()) {
        if (const CAnimData* animData = actor->GetAnimationData()) {
          mParentLocator = animData->GetLocatorSegId(mDescription.mLocatorName);
          if (mParentLocator == CSegId::Invalid()) {
            mUseParentLocator = false;
          }
        }
      }
      if (mDescription.mUseParentRotation) {
        const CTransform4f parent =
            mParentLocator != CSegId::Invalid()
                ? actor->GetTransform() * actor->GetScaledLocatorTransform(mParentLocator)
                : actor->GetTransform();
        SetTransform(parent * mParentTransform);
      } else {
        const CVector3f position =
            mParentLocator != CSegId::Invalid()
                ? (actor->GetTransform() * actor->GetScaledLocatorTransform(mParentLocator))
                      .GetTranslation()
                : actor->GetTranslation();
        SetTranslation(position + mParentTransform.GetTranslation());
      }
    } else {
      mParentId = kInvalidUniqueId;
    }
  }
}

void CScriptDynamicLight::UpdateTarget(CStateManager& mgr) {
  if (GetActive() && mTargetId != kInvalidUniqueId) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mTargetId))) {
      CTransform4f xf = CTransform4f::LookAt(GetTranslation(), actor->GetTranslation());
      xf.SetTranslation(GetTranslation());
      SetTransform(xf);
    } else {
      mTargetId = kInvalidUniqueId;
    }
  }
}

CScriptDynamicLight::SDescription::SDescription(
    ELightKind kind, const CColor& color, const CMayaSpline& intensity, float intensityDuration,
    bool intensityLoops, EFalloffType falloffType, const CMayaSpline& falloff,
    float falloffDuration, bool falloffLoops, const CMayaSpline& spotlight, float spotlightDuration,
    bool spotlightLoops, ELightSet lightSet, const CVector3f& parentTranslation,
    const CVector3f& parentRotation, const rstl::string& locator, bool useParentRotation)
: mKind(kind)
, mColor(color)
, mIntensitySpline(intensity)
, mIntensityDuration(intensityDuration)
, mFalloffType(falloffType)
, mFalloffSpline(falloff)
, mFalloffDuration(falloffDuration)
, mSpotlightSpline(spotlight)
, mSpotlightDuration(spotlightDuration)
, mParentTranslation(parentTranslation)
, mParentRotation(parentRotation)
, mLocatorName(locator)
, mLightSet(lightSet)
, mIntensityLoops(intensityLoops)
, mFalloffLoops(falloffLoops)
, mSpotlightLoops(spotlightLoops)
, mUseParentRotation(useParentRotation) {}

CEntity* LoadDynamicLight(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrDynamicLight sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrDynamicLight.inc"
  const CTransform4f xf = LdrToTransform4f(sldrThis.editorProperties);
  const CGameSplineDesc spline(
      sldrThis.motionSpline.motionControlSpline,
      static_cast< CMotionSpline::ESplineType >(sldrThis.motionSpline.motionSplineType.type),
      sldrThis.motionSpline.motionSplineDuration, sldrThis.motionSpline.motionSplinePathLoops);
  const CScriptDynamicLight::SDescription description(
      static_cast< CScriptDynamicLight::ELightKind >(sldrThis.lightType), sldrThis.color,
      sldrThis.intensity.intensity, sldrThis.intensity.intensityDuration,
      sldrThis.intensity.intensityLoops, static_cast< EFalloffType >(sldrThis.falloff.falloffType),
      sldrThis.falloff.falloffRate, sldrThis.falloff.falloffRateDuration,
      sldrThis.falloff.falloffRateLoops, sldrThis.spotlight.spotlightAngle,
      sldrThis.spotlight.spotlightAngleDuration, sldrThis.spotlight.spotlightAngleLoops,
      static_cast< CScriptDynamicLight::ELightSet >(sldrThis.lightSet),
      sldrThis.parent.translationFromParent, sldrThis.parent.rotationFromParent,
      sldrThis.parent.locatorName, sldrThis.parent.useParentRotation);
  rstl::optional_object< CLight > light;
  switch (description.mKind) {
  case CScriptDynamicLight::kLK_LocalAmbient:
    light = CLight::BuildLocalAmbient(CVector3f::Zero(), description.mColor);
    light->SetAttenuation(0.f, 0.f, 0.f);
    break;
  case CScriptDynamicLight::kLK_Directional:
    light = CLight::BuildDirectional(CVector3f::Forward(), description.mColor);
    break;
  case CScriptDynamicLight::kLK_Spot:
    light = CLight::BuildSpot(CVector3f::Zero(), CVector3f::Forward(), description.mColor, 45.f);
    break;
  default:
    light = CLight::BuildCustom(CVector3f::Zero(), CVector3f::Forward(), description.mColor, 0.f,
                                0.f, 0.f, 0.f, 0.f, 0.f);
    break;
  }
  if (!light) {
    return nullptr;
  }
  if (description.mFalloffType < kFT_Constant || description.mFalloffType > kFT_Quadratic) {
    return nullptr;
  }
  if (description.mLightSet < CScriptDynamicLight::kLS_LayerOne ||
      description.mLightSet > CScriptDynamicLight::kLS_All) {
    return nullptr;
  }
  return rs_new CScriptDynamicLight(mgr.AllocateUniqueId(), mgr.GetNextAreaId(), *light,
                                    description, sldrThis.editorProperties.name,
                                    LdrToEntityInfo(info, sldrThis.editorProperties), xf, spline);
}

CScriptDynamicLight::SDescription::~SDescription() {}

CScriptDynamicLight::~CScriptDynamicLight() {}
