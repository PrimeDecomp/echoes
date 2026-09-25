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
, mB24_enableAnaLeftXP(false)
, mB25_enableAnaLeftYP(false)
, mB26_enableAnaRightXP(false)
, mB27_enableAnaRightYP(false)
, mAnaLeftTriggerP(0.f)
, mAnaRightTriggerP(0.f)
, mB28_A(false)
, mB29_B(false)
, mB30_X(false)
, mB31_Y(false)
, mB24_Z(false)
, mB25_L(false)
, mB26_R(false)
, mB27_DPUp(false)
, mB28_DPRight(false)
, mB29_DPDown(false)
, mB30_DPLeft(false)
, mB31_Start(false)
, mB24_PA(false)
, mB25_PB(false)
, mB26_PX(false)
, mB27_PY(false)
, mB28_PZ(false)
, mB29_PL(false)
, mB30_PR(false)
, mB31_PDPUp(false)
, mB24_PDPRight(false)
, mB25_PDPDown(false)
, mB26_PDPLeft(false)
, mB27_PStart(false) {}

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
, mB24_enableAnaLeftXP(false)
, mB25_enableAnaLeftYP(false)
, mB26_enableAnaRightXP(false)
, mB27_enableAnaRightYP(false)
, mB28_A(data.GetButton(kBU_A).GetIsPressed())
, mB29_B(data.GetButton(kBU_B).GetIsPressed())
, mB30_X(data.GetButton(kBU_X).GetIsPressed())
, mB31_Y(data.GetButton(kBU_Y).GetIsPressed())
, mB24_Z(data.GetButton(kBU_Z).GetIsPressed())
, mB25_L(data.GetButton(kBU_L).GetIsPressed())
, mB26_R(data.GetButton(kBU_R).GetIsPressed())
, mB27_DPUp(data.GetButton(kBU_Up).GetIsPressed())
, mB28_DPRight(data.GetButton(kBU_Right).GetIsPressed())
, mB29_DPDown(data.GetButton(kBU_Down).GetIsPressed())
, mB30_DPLeft(data.GetButton(kBU_Left).GetIsPressed())
, mB31_Start(data.GetButton(kBU_Start).GetIsPressed())
, mB24_PA(data.GetButton(kBU_A).GetPressEvent())
, mB25_PB(data.GetButton(kBU_B).GetPressEvent())
, mB26_PX(data.GetButton(kBU_X).GetPressEvent())
, mB27_PY(data.GetButton(kBU_Y).GetPressEvent())
, mB28_PZ(data.GetButton(kBU_Z).GetPressEvent())
, mB29_PL(data.GetButton(kBU_L).GetPressEvent())
, mB30_PR(data.GetButton(kBU_R).GetPressEvent())
, mB31_PDPUp(data.GetButton(kBU_Up).GetPressEvent())
, mB24_PDPRight(data.GetButton(kBU_Right).GetPressEvent())
, mB25_PDPDown(data.GetButton(kBU_Down).GetPressEvent())
, mB26_PDPLeft(data.GetButton(kBU_Left).GetPressEvent())
, mB27_PStart(data.GetButton(kBU_Start).GetPressEvent()) {
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
    mB25_enableAnaLeftYP = true;
  } else if (mAnaLeftY > 0.7f && sIsAnalogPressed[mControllerIdx][0]) {
    mB25_enableAnaLeftYP = false;
  } else if (mAnaLeftY < -0.7f && !sIsAnalogPressed[mControllerIdx][0]) {
    mB25_enableAnaLeftYP = true;
    sIsAnalogPressed[mControllerIdx][0] = true;
  } else if (mAnaLeftY < -0.7f && sIsAnalogPressed[mControllerIdx][0]) {
    mB25_enableAnaLeftYP = false;
  } else if (fabsf(mAnaLeftY) < 0.7f) {
    mB25_enableAnaLeftYP = false;
    sIsAnalogPressed[mControllerIdx][0] = false;
  }

  if (mAnaLeftX > 0.7f && !sIsAnalogPressed[mControllerIdx][1]) {
    mB24_enableAnaLeftXP = true;
    sIsAnalogPressed[mControllerIdx][1] = true;
  } else if (mAnaLeftX > 0.7f && sIsAnalogPressed[mControllerIdx][1]) {
    mB24_enableAnaLeftXP = false;
  } else if (mAnaLeftX < -0.7f && !sIsAnalogPressed[mControllerIdx][1]) {
    sIsAnalogPressed[mControllerIdx][1] = true;
    mB24_enableAnaLeftXP = true;
  } else if (mAnaLeftX < -0.7f && sIsAnalogPressed[mControllerIdx][1]) {
    mB24_enableAnaLeftXP = false;
  } else if (fabsf(mAnaLeftX) < 0.7f) {
    mB24_enableAnaLeftXP = false;
    sIsAnalogPressed[mControllerIdx][1] = false;
  }

  if (mAnaRightY > 0.7f && !sIsAnalogPressed[mControllerIdx][2]) {
    sIsAnalogPressed[mControllerIdx][2] = true;
    mB27_enableAnaRightYP = true;
  } else if (mAnaRightY > 0.7f && sIsAnalogPressed[mControllerIdx][2]) {
    mB27_enableAnaRightYP = false;
  } else if (mAnaRightY < -0.7f && !sIsAnalogPressed[mControllerIdx][2]) {
    mB27_enableAnaRightYP = true;
    sIsAnalogPressed[mControllerIdx][2] = true;
  } else if (mAnaRightY < -0.7f && sIsAnalogPressed[mControllerIdx][2]) {
    mB27_enableAnaRightYP = false;
  } else if (fabsf(mAnaRightY) < 0.7f) {
    mB27_enableAnaRightYP = false;
    sIsAnalogPressed[mControllerIdx][2] = false;
  }

  if (mAnaRightX > 0.7f && !sIsAnalogPressed[mControllerIdx][3]) {
    mB26_enableAnaRightXP = true;
    sIsAnalogPressed[mControllerIdx][3] = true;
  } else if (mAnaRightX > 0.7f && sIsAnalogPressed[mControllerIdx][3]) {
    mB26_enableAnaRightXP = false;
  } else if (mAnaRightX < -0.7f && !sIsAnalogPressed[mControllerIdx][3]) {
    sIsAnalogPressed[mControllerIdx][3] = true;
    mB26_enableAnaRightXP = true;
  } else if (mAnaRightX < -0.7f && sIsAnalogPressed[mControllerIdx][3]) {
    mB26_enableAnaRightXP = false;
  } else if (fabsf(mAnaRightX) < 0.7f) {
    mB26_enableAnaRightXP = false;
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
, mB24_enableAnaLeftXP(false)
, mB25_enableAnaLeftYP(false)
, mB26_enableAnaRightXP(false)
, mB27_enableAnaRightYP(false)
, mAnaLeftTriggerP(0.f)
, mAnaRightTriggerP(0.f)
, mB28_A(ctx.GetOsKeyState(0x6d).IsPressed())
, mB29_B(ctx.GetOsKeyState(0x6e).IsPressed())
, mB30_X(ctx.GetOsKeyState(0x84).IsPressed())
, mB31_Y(ctx.GetOsKeyState(0x85).IsPressed())
, mB24_Z(ctx.GetOsKeyState(0x16).IsPressed())
, mB25_L(ctx.GetOsKeyState(0x78).IsPressed())
, mB26_R(ctx.GetOsKeyState(0x7e).IsPressed())
, mB27_DPUp(ctx.GetOsKeyState(0x1c).IsPressed())
, mB28_DPRight(ctx.GetOsKeyState(0x1d).IsPressed())
, mB29_DPDown(ctx.GetOsKeyState(0x1e).IsPressed())
, mB30_DPLeft(ctx.GetOsKeyState(0x1b).IsPressed())
, mB31_Start(ctx.GetOsKeyState(5).IsPressed())
, mB24_PA(ctx.GetOsKeyState(0x6d).JustPressed())
, mB25_PB(ctx.GetOsKeyState(0x6e).JustPressed())
, mB26_PX(ctx.GetOsKeyState(0x84).JustPressed())
, mB27_PY(ctx.GetOsKeyState(0x85).JustPressed())
, mB28_PZ(ctx.GetOsKeyState(0x16).JustPressed())
, mB29_PL(ctx.GetOsKeyState(0x78).JustPressed())
, mB30_PR(ctx.GetOsKeyState(0x7e).JustPressed())
, mB31_PDPUp(ctx.GetOsKeyState(0x1c).JustPressed())
, mB24_PDPRight(ctx.GetOsKeyState(0x1d).JustPressed())
, mB25_PDPDown(ctx.GetOsKeyState(0x1e).JustPressed())
, mB26_PDPLeft(ctx.GetOsKeyState(0x1b).JustPressed())
, mB27_PStart(ctx.GetOsKeyState(5).JustPressed()) {}
