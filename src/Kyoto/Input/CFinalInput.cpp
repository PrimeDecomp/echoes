#include "Kyoto/Input/CFinalInput.hpp"

#include "Kyoto/Basics/COsContext.hpp"
#include "Kyoto/Input/CControllerGamepadData.hpp"
#include "Kyoto/Math/CMath.hpp"

const float CFinalInput::kInput_AnalogOnThreshhold = 0.7f;
const float CFinalInput::kInput_AnalogTriggerOnThreshhold = 0.05f;

CFinalInput::CFinalInput()
: mDt(0.f)
, mControllerIdx(0)
, mAnaLeftX(0.f)
, mAnaLeftY(0.f)
, mAnaRightX(0.f)
, mAnaRightY(0.f)
, mAnaLeftTrigger(0.f)
, mAnaRightTrigger(0.f)
, mEnableAnaLeftXP(false)
, mEnableAnaLeftYP(false)
, mEnableAnaRightXP(false)
, mEnableAnaRightYP(false)
, mAnaLeftTriggerP(0.f)
, mAnaRightTriggerP(0.f)
, mA(false)
, mB(false)
, mX(false)
, mY(false)
, mZ(false)
, mL(false)
, mR(false)
, mDPUp(false)
, mDPRight(false)
, mDPDown(false)
, mDPLeft(false)
, mStart(false)
, mPA(false)
, mPB(false)
, mPX(false)
, mPY(false)
, mPZ(false)
, mPL(false)
, mPR(false)
, mPDPUp(false)
, mPDPRight(false)
, mPDPDown(false)
, mPDPLeft(false)
, mPStart(false) {}

CFinalInput::CFinalInput(int channel, float dt, const CControllerGamepadData& data, float leftDiv,
                         float rightDiv)
: mDt(dt)
, mControllerIdx(channel)
, mAnaLeftX(data.GetAxis(kJA_LeftX).GetAbsoluteValue())
, mAnaLeftY(data.GetAxis(kJA_LeftY).GetAbsoluteValue())
, mAnaRightX(data.GetAxis(kJA_RightX).GetAbsoluteValue())
, mAnaRightY(data.GetAxis(kJA_RightY).GetAbsoluteValue())
, mAnaLeftTrigger(data.GetAnalogButton(kBA_Left).GetAbsoluteValue())
, mAnaRightTrigger(data.GetAnalogButton(kBA_Right).GetAbsoluteValue())
, mAnaLeftTriggerP(data.GetAnalogButton(kBA_Left).GetRelativeValue())
, mAnaRightTriggerP(data.GetAnalogButton(kBA_Right).GetRelativeValue())
, mEnableAnaLeftXP(false)
, mEnableAnaLeftYP(false)
, mEnableAnaRightXP(false)
, mEnableAnaRightYP(false)
, mA(data.GetButton(kBU_A).GetIsPressed())
, mB(data.GetButton(kBU_B).GetIsPressed())
, mX(data.GetButton(kBU_X).GetIsPressed())
, mY(data.GetButton(kBU_Y).GetIsPressed())
, mZ(data.GetButton(kBU_Z).GetIsPressed())
, mL(data.GetButton(kBU_L).GetIsPressed())
, mR(data.GetButton(kBU_R).GetIsPressed())
, mDPUp(data.GetButton(kBU_Up).GetIsPressed())
, mDPRight(data.GetButton(kBU_Right).GetIsPressed())
, mDPDown(data.GetButton(kBU_Down).GetIsPressed())
, mDPLeft(data.GetButton(kBU_Left).GetIsPressed())
, mStart(data.GetButton(kBU_Start).GetIsPressed())
, mPA(data.GetButton(kBU_A).GetPressEvent())
, mPB(data.GetButton(kBU_B).GetPressEvent())
, mPX(data.GetButton(kBU_X).GetPressEvent())
, mPY(data.GetButton(kBU_Y).GetPressEvent())
, mPZ(data.GetButton(kBU_Z).GetPressEvent())
, mPL(data.GetButton(kBU_L).GetPressEvent())
, mPR(data.GetButton(kBU_R).GetPressEvent())
, mPDPUp(data.GetButton(kBU_Up).GetPressEvent())
, mPDPRight(data.GetButton(kBU_Right).GetPressEvent())
, mPDPDown(data.GetButton(kBU_Down).GetPressEvent())
, mPDPLeft(data.GetButton(kBU_Left).GetPressEvent())
, mPStart(data.GetButton(kBU_Start).GetPressEvent()) {
  InitializeAnalog(leftDiv, rightDiv);
}

static bool sIsAnalogPressed[4][4] = {};

void CFinalInput::InitializeAnalog(float leftDiv, float rightDiv) {
  mAnaLeftX = CMath::Clamp(-1.f, mAnaLeftX / leftDiv, 1.f);
  mAnaLeftY = CMath::Clamp(-1.f, mAnaLeftY / leftDiv, 1.f);
  mAnaRightX = CMath::Clamp(-1.f, mAnaRightX / rightDiv, 1.f);
  mAnaRightY = CMath::Clamp(-1.f, mAnaRightY / rightDiv, 1.f);

  if (mAnaLeftY > 0.7f && !sIsAnalogPressed[mControllerIdx][0]) {
    sIsAnalogPressed[mControllerIdx][0] = true;
    mEnableAnaLeftYP = true;
  } else if (mAnaLeftY > 0.7f && sIsAnalogPressed[mControllerIdx][0]) {
    mEnableAnaLeftYP = false;
  } else if (mAnaLeftY < -0.7f && !sIsAnalogPressed[mControllerIdx][0]) {
    mEnableAnaLeftYP = true;
    sIsAnalogPressed[mControllerIdx][0] = true;
  } else if (mAnaLeftY < -0.7f && sIsAnalogPressed[mControllerIdx][0]) {
    mEnableAnaLeftYP = false;
  } else if (fabsf(mAnaLeftY) < 0.7f) {
    mEnableAnaLeftYP = false;
    sIsAnalogPressed[mControllerIdx][0] = false;
  }

  if (mAnaLeftX > 0.7f && !sIsAnalogPressed[mControllerIdx][1]) {
    mEnableAnaLeftXP = true;
    sIsAnalogPressed[mControllerIdx][1] = true;
  } else if (mAnaLeftX > 0.7f && sIsAnalogPressed[mControllerIdx][1]) {
    mEnableAnaLeftXP = false;
  } else if (mAnaLeftX < -0.7f && !sIsAnalogPressed[mControllerIdx][1]) {
    sIsAnalogPressed[mControllerIdx][1] = true;
    mEnableAnaLeftXP = true;
  } else if (mAnaLeftX < -0.7f && sIsAnalogPressed[mControllerIdx][1]) {
    mEnableAnaLeftXP = false;
  } else if (fabsf(mAnaLeftX) < 0.7f) {
    mEnableAnaLeftXP = false;
    sIsAnalogPressed[mControllerIdx][1] = false;
  }

  if (mAnaRightY > 0.7f && !sIsAnalogPressed[mControllerIdx][2]) {
    sIsAnalogPressed[mControllerIdx][2] = true;
    mEnableAnaRightYP = true;
  } else if (mAnaRightY > 0.7f && sIsAnalogPressed[mControllerIdx][2]) {
    mEnableAnaRightYP = false;
  } else if (mAnaRightY < -0.7f && !sIsAnalogPressed[mControllerIdx][2]) {
    mEnableAnaRightYP = true;
    sIsAnalogPressed[mControllerIdx][2] = true;
  } else if (mAnaRightY < -0.7f && sIsAnalogPressed[mControllerIdx][2]) {
    mEnableAnaRightYP = false;
  } else if (fabsf(mAnaRightY) < 0.7f) {
    mEnableAnaRightYP = false;
    sIsAnalogPressed[mControllerIdx][2] = false;
  }

  if (mAnaRightX > 0.7f && !sIsAnalogPressed[mControllerIdx][3]) {
    mEnableAnaRightXP = true;
    sIsAnalogPressed[mControllerIdx][3] = true;
  } else if (mAnaRightX > 0.7f && sIsAnalogPressed[mControllerIdx][3]) {
    mEnableAnaRightXP = false;
  } else if (mAnaRightX < -0.7f && !sIsAnalogPressed[mControllerIdx][3]) {
    sIsAnalogPressed[mControllerIdx][3] = true;
    mEnableAnaRightXP = true;
  } else if (mAnaRightX < -0.7f && sIsAnalogPressed[mControllerIdx][3]) {
    mEnableAnaRightXP = false;
  } else if (fabsf(mAnaRightX) < 0.7f) {
    mEnableAnaRightXP = false;
    sIsAnalogPressed[mControllerIdx][3] = false;
  }
}

CFinalInput CFinalInput::ScaleAnalogueSticks(float leftDiv, float rightDiv) const {
  CFinalInput ret = *this;
  ret.mAnaLeftX = CMath::Clamp(-1.f, mAnaLeftX / leftDiv, 1.f);
  ret.mAnaLeftY = CMath::Clamp(-1.f, mAnaLeftY / leftDiv, 1.f);
  ret.mAnaRightX = CMath::Clamp(-1.f, mAnaRightX / rightDiv, 1.f);
  ret.mAnaRightY = CMath::Clamp(-1.f, mAnaRightY / rightDiv, 1.f);
  return ret;
}

CFinalInput::CFinalInput(int channel, float dt, const COsContext& ctx)
: mDt(dt)
, mControllerIdx(channel)
, mAnaLeftX(0.f)
, mAnaLeftY(0.f)
, mAnaRightX(0.f)
, mAnaRightY(0.f)
, mAnaLeftTrigger(0.f)
, mAnaRightTrigger(0.f)
, mEnableAnaLeftXP(false)
, mEnableAnaLeftYP(false)
, mEnableAnaRightXP(false)
, mEnableAnaRightYP(false)
, mAnaLeftTriggerP(0.f)
, mAnaRightTriggerP(0.f)
, mA(ctx.GetOsKeyState(0x6d).IsPressed())
, mB(ctx.GetOsKeyState(0x6e).IsPressed())
, mX(ctx.GetOsKeyState(0x84).IsPressed())
, mY(ctx.GetOsKeyState(0x85).IsPressed())
, mZ(ctx.GetOsKeyState(0x16).IsPressed())
, mL(ctx.GetOsKeyState(0x78).IsPressed())
, mR(ctx.GetOsKeyState(0x7e).IsPressed())
, mDPUp(ctx.GetOsKeyState(0x1c).IsPressed())
, mDPRight(ctx.GetOsKeyState(0x1d).IsPressed())
, mDPDown(ctx.GetOsKeyState(0x1e).IsPressed())
, mDPLeft(ctx.GetOsKeyState(0x1b).IsPressed())
, mStart(ctx.GetOsKeyState(5).IsPressed())
, mPA(ctx.GetOsKeyState(0x6d).JustPressed())
, mPB(ctx.GetOsKeyState(0x6e).JustPressed())
, mPX(ctx.GetOsKeyState(0x84).JustPressed())
, mPY(ctx.GetOsKeyState(0x85).JustPressed())
, mPZ(ctx.GetOsKeyState(0x16).JustPressed())
, mPL(ctx.GetOsKeyState(0x78).JustPressed())
, mPR(ctx.GetOsKeyState(0x7e).JustPressed())
, mPDPUp(ctx.GetOsKeyState(0x1c).JustPressed())
, mPDPRight(ctx.GetOsKeyState(0x1d).JustPressed())
, mPDPDown(ctx.GetOsKeyState(0x1e).JustPressed())
, mPDPLeft(ctx.GetOsKeyState(0x1b).JustPressed())
, mPStart(ctx.GetOsKeyState(5).JustPressed()) {}
