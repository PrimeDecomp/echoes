#include "MetroidPrime/ScriptObjects/CScriptFogOverlay.hpp"

#include <float.h>
#include <math.h>

#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrFogOverlay.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"

CScriptFogOverlay::CScriptFogOverlay(TUniqueId uid, const CEntityInfo& info,
                                     const rstl::string& name, float fullAlpha, float fadeDownTime,
                                     float fadeUpTime, bool startFadedOut, float ambientSpeed,
                                     float ambientSpeedTarget, float speedFadeUpTime,
                                     float speedFadeDownTime, float scaleTarget,
                                     float scaleFadeUpTime, float scaleFadeDownTime,
                                     const CColor& color, const CVector2f& ambientRadius,
                                     const CVector3f& scrollVelocity)
: CActor(uid, name, info, 0, CTransform4f::Identity(), CModelData::None(), CMaterialList(),
         CActorParameters::None(), kInvalidUniqueId)
, mFullAlpha(fullAlpha)
, mFadeDownTime(fadeDownTime)
, mFadeUpTime(fadeUpTime)
, mAmbientRadius(ambientRadius)
, mAmbientSpeed(ambientSpeed / 100.f)
, mAmbientSpeedBase(mAmbientSpeed)
, mAmbientSpeedTarget(ambientSpeedTarget / 100.f)
, mSpeedFadeDownTime(speedFadeDownTime)
, mSpeedFadeUpTime(speedFadeUpTime)
, mColor(color)
, mAmbientAngle(0.f)
, mScrollVelocity(scrollVelocity)
, mScrollScale(1.f)
, mScrollScaleTarget(scaleTarget)
, mScaleFadeDownTime(scaleFadeDownTime)
, mScaleFadeUpTime(scaleFadeUpTime)
, mAlphaCommand(startFadedOut ? kFC_SnapLow : kFC_None)
, mSpeedCommand(kFC_None)
, mScaleCommand(kFC_None) {}

void CScriptFogOverlay::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CActor::AcceptScriptMsg(mgr, msg);
  if (!GetActive()) {
    return;
  }
  switch (msg.GetMessage()) {
  case kSM_Increment:
    mAlphaCommand = kFC_FadeUp;
    break;
  case kSM_Decrement:
    mAlphaCommand = kFC_FadeDown;
    break;
  case kSM_SetToZero:
    mAlphaCommand = kFC_SnapLow;
    break;
  case kSM_SetToMax:
    mAlphaCommand = kFC_SnapHigh;
    break;
  case kSM_InternalMessage00:
    mSpeedCommand = kFC_FadeDown;
    break;
  case kSM_InternalMessage01:
    mSpeedCommand = kFC_FadeUp;
    break;
  case kSM_InternalMessage02:
    mScaleCommand = kFC_FadeDown;
    break;
  case kSM_InternalMessage03:
    mScaleCommand = kFC_FadeUp;
    break;
  default:
    break;
  }
}

static float Fraction(float value) { return value - CMath::FloorF(value); }

// Steps a value toward `high` (mode 1) or `low` (mode 2) over the given time, or snaps it to
// `low` (mode 3) or `high` (mode 4). A step that reaches either bound ends the command.
static void UpdateFade(int* outMode, float* outValue, int mode, float value, float dt, float upTime,
                       float downTime, float low, float high) {
  switch (mode) {
  case 1:
    value += dt * (high - low) / upTime;
    break;
  case 2:
    value += dt * (low - high) / downTime;
    break;
  case 3:
    value = low;
    mode = 0;
    break;
  case 4:
    value = high;
    mode = 0;
    break;
  }

  if (mode != 0) {
    float clamped;
    if (high > low) {
      clamped = CMath::Clamp(low, value, high);
    } else {
      clamped = CMath::Clamp(high, value, low);
    }
    if (clamped != value) {
      mode = 0;
      value = clamped;
    }
  }

  if (outMode != nullptr) {
    *outMode = mode;
  }
  if (outValue != nullptr) {
    *outValue = value;
  }
}

void CScriptFogOverlay::Think(float dt, CStateManager& mgr) {
  CStateManager::SDarkWorldCloud& cloud = mgr.mDarkWorldCloud;
  float& cloudAlpha = cloud.mTime;
  CVector3f& cloudScale = cloud.mScale;

  int command;
  UpdateFade(&command, &cloudAlpha, mAlphaCommand, cloudAlpha, dt, mFadeUpTime, mFadeDownTime, 0.f,
             mFullAlpha);
  mAlphaCommand = command;
  UpdateFade(&command, &mAmbientSpeed, mSpeedCommand, mAmbientSpeed, dt, mSpeedFadeUpTime,
             mSpeedFadeDownTime, mAmbientSpeedBase, mAmbientSpeedTarget);
  mSpeedCommand = command;
  UpdateFade(&command, &mScrollScale, mScaleCommand, mScrollScale, dt, mScaleFadeUpTime,
             mScaleFadeDownTime, 1.f, mScrollScaleTarget);
  mScaleCommand = command;

  const CTransform4f camXf(mgr.GetCameraManager(0)->GetCurrentCameraTransform(mgr, true));
  const CVector3f local =
      camXf.TransposeRotate(dt * (mScrollScale * mScrollVelocity) + camXf.GetTranslation() -
                            cloud.mTransform.GetTranslation());
  cloudScale += CVector3f(local.GetX() * 0.075f, local.GetY() * 0.5f, local.GetZ() * 0.075f);
  cloudScale.SetZ(cloudScale.GetZ() + -0.2f * (cloud.mTransform.Get21() - camXf.Get21()));

  if (CMath::AbsD(camXf.Get21()) < 0.95f && CMath::AbsD(cloud.mTransform.Get21()) < 0.95f) {
    CVector3f camDir(camXf.Get01(), camXf.Get11(), 0.f);
    camDir.Normalize();
    CVector3f cloudDir(cloud.mTransform.Get01(), cloud.mTransform.Get11(), 0.f);
    cloudDir.Normalize();
    const float dot = cloudDir.GetX() * camDir.GetX() + cloudDir.GetY() * camDir.GetY();
    const float cross = cloudDir.GetX() * camDir.GetY() - cloudDir.GetY() * camDir.GetX();
    if (!CMath::IsEpsilon(dot, FLT_EPSILON, 0.00001f) &&
        !CMath::IsEpsilon(cross, FLT_EPSILON, 0.00001f)) {
      cloudScale.SetX(cloudScale.GetX() - 0.1f * static_cast< float >(atan2(cross, dot)));
    }
  }

  if (!CMath::IsEpsilon(mAmbientSpeed, 0.f, 0.00001f)) {
    const float oldCos = CMath::FastCosR(mAmbientAngle);
    const float oldSin = CMath::FastSinR(mAmbientAngle);
    mAmbientAngle = CMath::ClampRadians(mAmbientAngle + M_2PIF * (mAmbientSpeed * dt));
    cloudScale.SetX(cloudScale.GetX() +
                    (CMath::FastCosR(mAmbientAngle) - oldCos) * mAmbientRadius.GetX());
    cloudScale.SetZ(cloudScale.GetZ() +
                    (CMath::FastSinR(mAmbientAngle) - oldSin) * mAmbientRadius.GetY());
  }

  cloudScale = CVector3f(Fraction(cloudScale.GetX()), Fraction(cloudScale.GetY()),
                         Fraction(cloudScale.GetZ()));
  cloud.mTransform = camXf;
  cloud.mColor = mColor;
}

void CScriptFogOverlay::PreRender(CStateManager& mgr) {}

void CScriptFogOverlay::Render(const CStateManager& mgr) const {}

CEntity* REL_LoadFogOverlay(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrFogOverlay sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrFogOverlay.inc"

  return rs_new CScriptFogOverlay(
      mgr.AllocateUniqueId(), LdrToEntityInfo(info, sldrThis.editorProperties),
      sldrThis.editorProperties.name, sldrThis.fullAlpha, sldrThis.fadeDownTime,
      sldrThis.fadeUpTime, sldrThis.startFadedOut, sldrThis.ambientSpeed,
      sldrThis.ambientSpeedTarget, sldrThis.unknown_0x6a111b96, sldrThis.unknown_0xff226ea3,
      sldrThis.unknown_0x9f19f0af, sldrThis.unknown_0x90c10fe7, sldrThis.unknown_0xd8daff1d,
      sldrThis.color.WithAlphaOf(1.f), CVector2f(sldrThis.ambientRadiusX, sldrThis.ambientRadiusY),
      sldrThis.unknown_0x2190ab0a);
}

static void SetFuncPtrs() {
  static SScriptFogOverlay_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &REL_LoadFogOverlay;
  SetSScriptFogOverlay_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSScriptFogOverlay_FuncPtrs(nullptr); }
