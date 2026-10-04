#include "MetroidPrime/ScriptObjects/CScriptDynamicLight.hpp"

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
, mSpline(spline.GetDuration(), spline.IsClosedLoop() ? CGameSpline::kF_LoopPosition : 0,
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
  if (msg.GetMessage() == kSM_XALD) {
    if (CheckConnectedObject_if(mgr, kSS_CameraPath, kSM_Attach, CValidEntityPredicate()) !=
        kInvalidUniqueId) {
      mHasSpline = true;
      ScriptCameraSpline::Initialise(*this, kSS_CameraPath, kSM_Attach, kSS_CameraTarget,
                                     kSM_Follow, mgr, mSpline);
    }
    FindLightReceivers(mgr);
    FindParent(mgr);
    FindTarget(mgr);
    UpdateLight(0.f);
  }
  CActor::AcceptScriptMsg(mgr, msg);
}

void CScriptDynamicLight::FindLightReceivers(CStateManager& mgr) {
  const rstl::vector< TUniqueId > receivers = FindConnectedObjects(mgr, kSS_Play, kSM_Activate);
  bool found = false;
  for (int i = 0; i < receivers.size(); ++i) {
    CActor* actor = TCastToPtr< CActor >(mgr.GetObjectByIdFromListAll(receivers[i]));
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
    if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(parents[i]))) {
      mParentId = actor->GetUniqueId();
      mParentTransform = ConvertEditorEulerToTransform4f(mDescription.mParentRotation,
                                                         mDescription.mParentTranslation);
      mHasParent = true;
      mUseParentLocator = mDescription.mLocatorName.size() != 0;
      if (!mDescription.mUseParentRotation) {
        mParentTransform = actor->GetTransform() * mParentTransform;
        SetTransform(mParentTransform);
        mParentTransform.SetTranslation(mParentTransform.GetTranslation() +
                                        actor->GetTranslation() * -1.f);
      }
      break;
    }
  }
}

void CScriptDynamicLight::FindTarget(CStateManager& mgr) {
  const rstl::vector< TUniqueId > targets = FindConnectedObjects(mgr, kSS_CameraTarget, kSM_Attach);
  for (int i = 0; i < targets.size(); ++i) {
    if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(targets[i]))) {
      mTargetId = actor->GetUniqueId();
      break;
    }
  }
}

void CScriptDynamicLight::UpdateLight(float dt) {
  if (!GetActive()) {
    return;
  }
  mIntensityTime += dt;
  if (mIntensityTime >= mDescription.mIntensityDuration) {
    mIntensityTime = mDescription.mIntensityLoops ? 0.f : mDescription.mIntensityDuration;
  }
  mIntensity = mDescription.mIntensitySpline.EvaluateAt(mIntensityTime);
  if (mDescription.mKind == kLK_LocalAmbient || mDescription.mKind == kLK_Directional ||
      mDescription.mKind == kLK_Spot) {
    const float red = mIntensity * mDescription.mColor.GetRed();
    const float green = mIntensity * mDescription.mColor.GetGreen();
    const float blue = mIntensity * mDescription.mColor.GetBlue();
    const float alpha = mIntensity * mDescription.mColor.GetAlpha();
    // The target's upper-only limit returns one for unordered input as well.
    Light().SetColor(CColor(CMath::Min(red, 1.f), CMath::Min(green, 1.f), CMath::Min(blue, 1.f),
                            CMath::Min(alpha, 1.f)));
  }
  if (mDescription.mKind == kLK_Point || mDescription.mKind == kLK_Spot) {
    mFalloffTime += dt;
    if (mFalloffTime >= mDescription.mFalloffDuration) {
      mFalloffTime = mDescription.mFalloffLoops ? 0.f : mDescription.mFalloffDuration;
    }
    const float falloff = mDescription.mFalloffSpline.EvaluateAt(mFalloffTime);
    if (mDescription.mKind == kLK_Point) {
      Light().SetAngleAttenuation(mIntensity, 0.f, 0.f);
    }
    Light().SetAttenuation(mDescription.mFalloffType == kFT_Constant ? 1.f : 0.f,
                           mDescription.mFalloffType == kFT_Linear ? falloff : 0.f,
                           mDescription.mFalloffType == kFT_Quadratic ? falloff : 0.f);
  }
  if (mDescription.mKind == kLK_Spot) {
    mSpotlightTime += dt;
    if (mSpotlightTime >= mDescription.mSpotlightDuration) {
      mSpotlightTime = mDescription.mSpotlightLoops ? 0.f : mDescription.mSpotlightDuration;
    }
    Light().SetSpotCutoff(mDescription.mSpotlightSpline.EvaluateAt(mSpotlightTime));
  }
}

void CScriptDynamicLight::UpdateSpline(float dt) {
  if (mHasSpline && GetActive()) {
    mSplineTime += dt;
    if (mSplineTime >= mSpline.GetPositionSpline().GetDuration()) {
      mSplineTime = mSplineLoops ? 0.f : mSpline.GetPositionSpline().GetDuration();
    }
    SetTranslation(mSpline.GetPositionByTime(mSplineTime));
  }
}

void CScriptDynamicLight::UpdateParent(CStateManager& mgr) {
  if (!GetActive() || mParentId == kInvalidUniqueId) {
    return;
  }
  CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(mParentId));
  if (!actor) {
    mParentId = kInvalidUniqueId;
    return;
  }
  if (mUseParentLocator && mParentLocator == CSegId::Invalid() && actor->HasAnimation()) {
    mParentLocator = actor->GetAnimationData()->GetLocatorSegId(mDescription.mLocatorName);
    if (mParentLocator == CSegId::Invalid()) {
      mUseParentLocator = false;
    }
  }
  if (mDescription.mUseParentRotation) {
    const CTransform4f parent =
        mParentLocator == CSegId::Invalid()
            ? actor->GetTransform()
            : actor->GetTransform() * actor->GetScaledLocatorTransform(mParentLocator);
    SetTransform(parent * mParentTransform);
  } else {
    const CVector3f position =
        mParentLocator == CSegId::Invalid()
            ? actor->GetTranslation()
            : (actor->GetTransform() * actor->GetScaledLocatorTransform(mParentLocator))
                  .GetTranslation();
    SetTranslation(position + mParentTransform.GetTranslation());
  }
}

void CScriptDynamicLight::UpdateTarget(CStateManager& mgr) {
  if (!GetActive() || mTargetId == kInvalidUniqueId) {
    return;
  }
  CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(mTargetId));
  if (!actor) {
    mTargetId = kInvalidUniqueId;
  } else {
    CTransform4f xf = CTransform4f::LookAt(GetTranslation(), actor->GetTranslation());
    xf.SetTranslation(GetTranslation());
    SetTransform(xf);
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
  if (!light || description.mFalloffType < kFT_Constant ||
      description.mFalloffType > kFT_Quadratic ||
      description.mLightSet < CScriptDynamicLight::kLS_LayerOne ||
      description.mLightSet > CScriptDynamicLight::kLS_All) {
    return nullptr;
  }
  return rs_new CScriptDynamicLight(mgr.AllocateUniqueId(), mgr.GetNextAreaId(), *light,
                                    description, sldrThis.editorProperties.name,
                                    LdrToEntityInfo(info, sldrThis.editorProperties), xf, spline);
}

CScriptDynamicLight::SDescription::~SDescription() {}

CScriptDynamicLight::~CScriptDynamicLight() {}
