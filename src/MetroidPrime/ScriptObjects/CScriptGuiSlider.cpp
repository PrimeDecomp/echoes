#include "MetroidPrime/ScriptObjects/CScriptGuiSlider.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CSaveGameScreen.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader/SLdrGuiSlider.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "rstl/math.hpp"

#include <math.h>

CScriptGuiSlider::CScriptGuiSlider(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                                   int controller, const rstl::string& label, bool locked,
                                   float minValue, float maxValue, float increment,
                                   float slideSpeed, ushort slideSfx, int slideSfxVolume)
: CScriptGuiWidget(uid, name, info, controller, label, locked)
, mMinValue(minValue)
, mMaxValue(maxValue)
, mIncrement(increment)
, mSlideSpeed(slideSpeed)
, mTargetValue(mMinValue)
, mValue(mTargetValue)
, mSecondaryValue(0.f)
, mPrimaryPlatform(kInvalidUniqueId)
, mSecondaryPlatform(kInvalidUniqueId)
, mSlideState(kSS_Idle)
, mSlideRequested(false)
, mSlideSfx(slideSfx)
, mSlideSfxVolume(slideSfxVolume) {}

void CScriptGuiSlider::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CScriptGuiWidget::AcceptScriptMsg(mgr, msg);

  switch (msg.GetMessage()) {
  case kSM_AreaLoaded:
    FindPlatforms(mgr);
    return;
  case kSM_Deactivate:
  case kSM_Delete:
    if (mSlideSfxHandle) {
      CSfxManager::SfxStop(mSlideSfxHandle);
      mSlideSfxHandle = CSfxHandle();
    }
    break;
  default:
    break;
  }

  if (GetActive()) {
    switch (msg.GetMessage()) {
    case kSM_Increment:
      StartIncrease(mgr);
      break;
    case kSM_Decrement:
      StartDecrease(mgr);
      break;
    default:
      break;
    }
  }
}

void CScriptGuiSlider::Think(float dt, CStateManager& mgr) {
  CScriptGuiWidget::Think(dt, mgr);
  if (!GetActive()) {
    return;
  }

  float prevValue;
  float delta = 0.f;
  CSaveGameScreen* saveScreen = mgr.mSaveGameScreen.get();
  if (saveScreen == nullptr || saveScreen->GetUIType() == CSaveGameScreen::kUIT_SaveReady) {
    const CFinalInput& input = mgr.mFinalInputs[mControllerNumber];
    if (input.ALALeft() > 0.f) {
      const float speed = mSlideSpeed * (dt * input.ALALeft());
      delta = speed * (mMaxValue - mMinValue);
      StartDecrease(mgr);
    } else if (input.ALARight() > 0.f) {
      const float speed = mSlideSpeed * (dt * input.ALARight());
      delta = speed * (mMaxValue - mMinValue);
      StartIncrease(mgr);
    } else if (input.PDPLeft()) {
      delta = 1.f;
      StartDecrease(mgr);
    } else if (input.PDPRight()) {
      delta = 1.f;
      StartIncrease(mgr);
    }
  }

  prevValue = mValue;
  if (mSlideState == kSS_Decreasing) {
    if (mSlideRequested) {
      mValue = rstl::max_val(mMinValue, prevValue - delta);
    }
  } else if (mSlideState == kSS_Increasing) {
    if (mSlideRequested) {
      mValue = rstl::min_val(mMaxValue, prevValue + delta);
    }
  }

  if (prevValue == mValue) {
    mSlideState = kSS_Idle;
  }

  const float prevTarget = mTargetValue;
  mTargetValue = mValue;
  if (prevTarget != mTargetValue) {
    const TEventCallback& callback = mCallback;
    if (callback) {
      callback(mgr, this, 4);
    }
  }

  UpdatePlatforms(mgr);

  if (prevValue != mValue) {
    SendScriptMsgs(kSS_Modify, mgr);
    if (!mSlideSfxHandle) {
      mSlideSfxHandle = CSfxManager::SfxStart(mSlideSfx, mSlideSfxVolume, 63,
                                              mgr.GetNextAreaId().value, true, true);
    }
  } else if (mSlideSfxHandle) {
    CSfxManager::SfxStop(mSlideSfxHandle);
    mSlideSfxHandle = CSfxHandle();
  }

  CSfxManager::SfxVolume(mSlideSfxHandle, mSlideSfxVolume);
  mSlideRequested = false;
}

void CScriptGuiSlider::StartIncrease(CStateManager& mgr) {
  mSlideState = kSS_Increasing;
  mSlideRequested = true;
}

void CScriptGuiSlider::StartDecrease(CStateManager& mgr) {
  mSlideState = kSS_Decreasing;
  mSlideRequested = true;
}

static inline float RoundToNearest(float value) {
  const float lower = floor(value);
  const float upper = CMath::CeilingF(value);
  return value - lower < upper - value ? lower : upper;
}

int CScriptGuiSlider::GetRoundedValue(float rangeMin, float rangeMax) const {
  const float t = (mTargetValue - mMinValue) / (mMaxValue - mMinValue);
  return RoundToNearest(CMath::Clamp(rangeMin, t * (rangeMax - rangeMin) + rangeMin, rangeMax));
}

float CScriptGuiSlider::GetValue(float rangeMin, float rangeMax) const {
  const float t = (mValue - mMinValue) / (mMaxValue - mMinValue);
  return CMath::Clamp(rangeMin, t * (rangeMax - rangeMin) + rangeMin, rangeMax);
}

void CScriptGuiSlider::SetValue(CStateManager& mgr, float value, float rangeMin, float rangeMax) {
  const float t = (value - rangeMin) / (rangeMax - rangeMin);
  const float scaled = CMath::Clamp(mMinValue, t * (mMaxValue - mMinValue) + mMinValue, mMaxValue);
  mTargetValue = scaled;
  mValue = scaled;
  mSlideState = kSS_Idle;
  UpdatePlatforms(mgr);
}

void CScriptGuiSlider::SetSecondaryValue(CStateManager& mgr, float value, float rangeMin,
                                         float rangeMax) {
  const float t = (value - rangeMin) / (rangeMax - rangeMin);
  mSecondaryValue = CMath::Clamp(mMinValue, t * (mMaxValue - mMinValue) + mMinValue, mMaxValue);
  UpdatePlatforms(mgr);
}

void CScriptGuiSlider::FindPlatforms(CStateManager& mgr) {
  for (rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
       it != GetConnectionList().end(); ++it) {
    if (it->state == kSS_Play) {
      const TUniqueId id = mgr.GetIdForScript(it->objId);
      if (CScriptPlatform* platform = TCastToPtr< CScriptPlatform >(mgr.ObjectById(id))) {
        if (it->msg == kSM_Activate) {
          mPrimaryPlatform = id;
          platform->SetControlledAnimation(true);
        } else if (it->msg == kSM_Deactivate) {
          mSecondaryPlatform = id;
          platform->SetControlledAnimation(true);
        }
      }
    }
  }
}

void CScriptGuiSlider::UpdatePlatforms(CStateManager& mgr) {
  if (CScriptPlatform* platform = TCastToPtr< CScriptPlatform >(mgr.ObjectById(mPrimaryPlatform))) {
    const float t = mMaxValue == mMinValue ? 0.f : (mValue - mMinValue) / (mMaxValue - mMinValue);
    if (platform->HasAnimation()) {
      CAnimData* animData = platform->AnimationData();
      animData->SetPhase(0.f);
      animData->SetPlaybackRate(1.f);
      platform->UpdateAnimation(t * animData->GetAnimationDuration(animData->GetCurrentAnimation()),
                                mgr, true);
    }
  }

  if (CScriptPlatform* platform =
          TCastToPtr< CScriptPlatform >(mgr.ObjectById(mSecondaryPlatform))) {
    const float t =
        mMaxValue == mMinValue ? 0.f : (mSecondaryValue - mMinValue) / (mMaxValue - mMinValue);
    if (platform->HasAnimation()) {
      CAnimData* animData = platform->AnimationData();
      animData->SetPhase(0.f);
      animData->SetPlaybackRate(1.f);
      platform->UpdateAnimation(t * animData->GetAnimationDuration(animData->GetCurrentAnimation()),
                                mgr, true);
    }
  }
}

CEntity* LoadGuiSlider(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrGuiSlider sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrGuiSlider.inc"

  const int controller = sldrThis.widgetProperties.controllerNumber - 1;
  return rs_new CScriptGuiSlider(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                                 LdrToEntityInfo(info, sldrThis.editorProperties), controller,
                                 sldrThis.widgetProperties.guiLabel,
                                 sldrThis.widgetProperties.isLocked, sldrThis.minValue,
                                 sldrThis.maxValue, sldrThis.increment, sldrThis.slideSpeed,
                                 sldrThis.slideSound, sldrThis.slideSoundVolume);
}

CScriptGuiSlider::~CScriptGuiSlider() {}
