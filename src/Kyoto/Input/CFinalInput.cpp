#include "Kyoto/Input/CFinalInput.hpp"

#include "Kyoto/Basics/COsContext.hpp"
#include "Kyoto/Input/CControllerGamepadData.hpp"
#include "Kyoto/Math/CMath.hpp"

const float CFinalInput::kInput_AnalogOnThreshhold = 0.7f;
const float CFinalInput::kInput_AnalogTriggerOnThreshhold = 0.05f;

CFinalInput::CFinalInput()
: x0_dt(0.f)
, x4_controllerIdx(0)
, x8_anaLeftX(0.f)
, xc_anaLeftY(0.f)
, x10_anaRightX(0.f)
, x14_anaRightY(0.f)
, x18_anaLeftTrigger(0.f)
, x1c_anaRightTrigger(0.f)
, x28_b24_enableAnaLeftXP(false)
, x28_b25_enableAnaLeftYP(false)
, x28_b26_enableAnaRightXP(false)
, x28_b27_enableAnaRightYP(false)
, x20_anaLeftTriggerP(0.f)
, x24_anaRightTriggerP(0.f)
, x28_b28_A(false)
, x28_b29_B(false)
, x28_b30_X(false)
, x28_b31_Y(false)
, x29_b24_Z(false)
, x29_b25_L(false)
, x29_b26_R(false)
, x29_b27_DPUp(false)
, x29_b28_DPRight(false)
, x29_b29_DPDown(false)
, x29_b30_DPLeft(false)
, x29_b31_Start(false)
, x2a_b24_PA(false)
, x2a_b25_PB(false)
, x2a_b26_PX(false)
, x2a_b27_PY(false)
, x2a_b28_PZ(false)
, x2a_b29_PL(false)
, x2a_b30_PR(false)
, x2a_b31_PDPUp(false)
, x2b_b24_PDPRight(false)
, x2b_b25_PDPDown(false)
, x2b_b26_PDPLeft(false)
, x2b_b27_PStart(false) {}

CFinalInput::CFinalInput(int channel, float dt, const CControllerGamepadData& data, float leftDiv,
                         float rightDiv)
: x0_dt(dt)
, x4_controllerIdx(channel)
, x8_anaLeftX(data.GetAxis(kJA_LeftX).GetAbsoluteValue())
, xc_anaLeftY(data.GetAxis(kJA_LeftY).GetAbsoluteValue())
, x10_anaRightX(data.GetAxis(kJA_RightX).GetAbsoluteValue())
, x14_anaRightY(data.GetAxis(kJA_RightY).GetAbsoluteValue())
, x18_anaLeftTrigger(data.GetAnalogButton(kBA_Left).GetAbsoluteValue())
, x1c_anaRightTrigger(data.GetAnalogButton(kBA_Right).GetAbsoluteValue())
, x20_anaLeftTriggerP(data.GetAnalogButton(kBA_Left).GetRelativeValue())
, x24_anaRightTriggerP(data.GetAnalogButton(kBA_Right).GetRelativeValue())
, x28_b24_enableAnaLeftXP(false)
, x28_b25_enableAnaLeftYP(false)
, x28_b26_enableAnaRightXP(false)
, x28_b27_enableAnaRightYP(false)
, x28_b28_A(data.GetButton(kBU_A).GetIsPressed())
, x28_b29_B(data.GetButton(kBU_B).GetIsPressed())
, x28_b30_X(data.GetButton(kBU_X).GetIsPressed())
, x28_b31_Y(data.GetButton(kBU_Y).GetIsPressed())
, x29_b24_Z(data.GetButton(kBU_Z).GetIsPressed())
, x29_b25_L(data.GetButton(kBU_L).GetIsPressed())
, x29_b26_R(data.GetButton(kBU_R).GetIsPressed())
, x29_b27_DPUp(data.GetButton(kBU_Up).GetIsPressed())
, x29_b28_DPRight(data.GetButton(kBU_Right).GetIsPressed())
, x29_b29_DPDown(data.GetButton(kBU_Down).GetIsPressed())
, x29_b30_DPLeft(data.GetButton(kBU_Left).GetIsPressed())
, x29_b31_Start(data.GetButton(kBU_Start).GetIsPressed())
, x2a_b24_PA(data.GetButton(kBU_A).GetPressEvent())
, x2a_b25_PB(data.GetButton(kBU_B).GetPressEvent())
, x2a_b26_PX(data.GetButton(kBU_X).GetPressEvent())
, x2a_b27_PY(data.GetButton(kBU_Y).GetPressEvent())
, x2a_b28_PZ(data.GetButton(kBU_Z).GetPressEvent())
, x2a_b29_PL(data.GetButton(kBU_L).GetPressEvent())
, x2a_b30_PR(data.GetButton(kBU_R).GetPressEvent())
, x2a_b31_PDPUp(data.GetButton(kBU_Up).GetPressEvent())
, x2b_b24_PDPRight(data.GetButton(kBU_Right).GetPressEvent())
, x2b_b25_PDPDown(data.GetButton(kBU_Down).GetPressEvent())
, x2b_b26_PDPLeft(data.GetButton(kBU_Left).GetPressEvent())
, x2b_b27_PStart(data.GetButton(kBU_Start).GetPressEvent()) {
  InitializeAnalog(leftDiv, rightDiv);
}

static bool sIsAnalogPressed[4][4] = {};

void CFinalInput::InitializeAnalog(float leftDiv, float rightDiv) {
  x8_anaLeftX = CMath::Clamp(-1.f, x8_anaLeftX / leftDiv, 1.f);
  xc_anaLeftY = CMath::Clamp(-1.f, xc_anaLeftY / leftDiv, 1.f);
  x10_anaRightX = CMath::Clamp(-1.f, x10_anaRightX / rightDiv, 1.f);
  x14_anaRightY = CMath::Clamp(-1.f, x14_anaRightY / rightDiv, 1.f);

  if (xc_anaLeftY > 0.7f && !sIsAnalogPressed[x4_controllerIdx][0]) {
    sIsAnalogPressed[x4_controllerIdx][0] = true;
    x28_b25_enableAnaLeftYP = true;
  } else if (xc_anaLeftY > 0.7f && sIsAnalogPressed[x4_controllerIdx][0]) {
    x28_b25_enableAnaLeftYP = false;
  } else if (xc_anaLeftY < -0.7f && !sIsAnalogPressed[x4_controllerIdx][0]) {
    x28_b25_enableAnaLeftYP = true;
    sIsAnalogPressed[x4_controllerIdx][0] = true;
  } else if (xc_anaLeftY < -0.7f && sIsAnalogPressed[x4_controllerIdx][0]) {
    x28_b25_enableAnaLeftYP = false;
  } else if (fabsf(xc_anaLeftY) < 0.7f) {
    x28_b25_enableAnaLeftYP = false;
    sIsAnalogPressed[x4_controllerIdx][0] = false;
  }

  if (x8_anaLeftX > 0.7f && !sIsAnalogPressed[x4_controllerIdx][1]) {
    x28_b24_enableAnaLeftXP = true;
    sIsAnalogPressed[x4_controllerIdx][1] = true;
  } else if (x8_anaLeftX > 0.7f && sIsAnalogPressed[x4_controllerIdx][1]) {
    x28_b24_enableAnaLeftXP = false;
  } else if (x8_anaLeftX < -0.7f && !sIsAnalogPressed[x4_controllerIdx][1]) {
    sIsAnalogPressed[x4_controllerIdx][1] = true;
    x28_b24_enableAnaLeftXP = true;
  } else if (x8_anaLeftX < -0.7f && sIsAnalogPressed[x4_controllerIdx][1]) {
    x28_b24_enableAnaLeftXP = false;
  } else if (fabsf(x8_anaLeftX) < 0.7f) {
    x28_b24_enableAnaLeftXP = false;
    sIsAnalogPressed[x4_controllerIdx][1] = false;
  }

  if (x14_anaRightY > 0.7f && !sIsAnalogPressed[x4_controllerIdx][2]) {
    sIsAnalogPressed[x4_controllerIdx][2] = true;
    x28_b27_enableAnaRightYP = true;
  } else if (x14_anaRightY > 0.7f && sIsAnalogPressed[x4_controllerIdx][2]) {
    x28_b27_enableAnaRightYP = false;
  } else if (x14_anaRightY < -0.7f && !sIsAnalogPressed[x4_controllerIdx][2]) {
    x28_b27_enableAnaRightYP = true;
    sIsAnalogPressed[x4_controllerIdx][2] = true;
  } else if (x14_anaRightY < -0.7f && sIsAnalogPressed[x4_controllerIdx][2]) {
    x28_b27_enableAnaRightYP = false;
  } else if (fabsf(x14_anaRightY) < 0.7f) {
    x28_b27_enableAnaRightYP = false;
    sIsAnalogPressed[x4_controllerIdx][2] = false;
  }

  if (x10_anaRightX > 0.7f && !sIsAnalogPressed[x4_controllerIdx][3]) {
    x28_b26_enableAnaRightXP = true;
    sIsAnalogPressed[x4_controllerIdx][3] = true;
  } else if (x10_anaRightX > 0.7f && sIsAnalogPressed[x4_controllerIdx][3]) {
    x28_b26_enableAnaRightXP = false;
  } else if (x10_anaRightX < -0.7f && !sIsAnalogPressed[x4_controllerIdx][3]) {
    sIsAnalogPressed[x4_controllerIdx][3] = true;
    x28_b26_enableAnaRightXP = true;
  } else if (x10_anaRightX < -0.7f && sIsAnalogPressed[x4_controllerIdx][3]) {
    x28_b26_enableAnaRightXP = false;
  } else if (fabsf(x10_anaRightX) < 0.7f) {
    x28_b26_enableAnaRightXP = false;
    sIsAnalogPressed[x4_controllerIdx][3] = false;
  }
}

CFinalInput CFinalInput::ScaleAnalogueSticks(float leftDiv, float rightDiv) const {
  CFinalInput ret = *this;
  ret.x8_anaLeftX = CMath::Clamp(-1.f, x8_anaLeftX / leftDiv, 1.f);
  ret.xc_anaLeftY = CMath::Clamp(-1.f, xc_anaLeftY / leftDiv, 1.f);
  ret.x10_anaRightX = CMath::Clamp(-1.f, x10_anaRightX / rightDiv, 1.f);
  ret.x14_anaRightY = CMath::Clamp(-1.f, x14_anaRightY / rightDiv, 1.f);
  return ret;
}

CFinalInput::CFinalInput(int channel, float dt, const COsContext& ctx)
: x0_dt(dt)
, x4_controllerIdx(channel)
, x8_anaLeftX(0.f)
, xc_anaLeftY(0.f)
, x10_anaRightX(0.f)
, x14_anaRightY(0.f)
, x18_anaLeftTrigger(0.f)
, x1c_anaRightTrigger(0.f)
, x28_b24_enableAnaLeftXP(false)
, x28_b25_enableAnaLeftYP(false)
, x28_b26_enableAnaRightXP(false)
, x28_b27_enableAnaRightYP(false)
, x20_anaLeftTriggerP(0.f)
, x24_anaRightTriggerP(0.f)
, x28_b28_A(ctx.GetOsKeyState(0x6d).IsPressed())
, x28_b29_B(ctx.GetOsKeyState(0x6e).IsPressed())
, x28_b30_X(ctx.GetOsKeyState(0x84).IsPressed())
, x28_b31_Y(ctx.GetOsKeyState(0x85).IsPressed())
, x29_b24_Z(ctx.GetOsKeyState(0x16).IsPressed())
, x29_b25_L(ctx.GetOsKeyState(0x78).IsPressed())
, x29_b26_R(ctx.GetOsKeyState(0x7e).IsPressed())
, x29_b27_DPUp(ctx.GetOsKeyState(0x1c).IsPressed())
, x29_b28_DPRight(ctx.GetOsKeyState(0x1d).IsPressed())
, x29_b29_DPDown(ctx.GetOsKeyState(0x1e).IsPressed())
, x29_b30_DPLeft(ctx.GetOsKeyState(0x1b).IsPressed())
, x29_b31_Start(ctx.GetOsKeyState(5).IsPressed())
, x2a_b24_PA(ctx.GetOsKeyState(0x6d).JustPressed())
, x2a_b25_PB(ctx.GetOsKeyState(0x6e).JustPressed())
, x2a_b26_PX(ctx.GetOsKeyState(0x84).JustPressed())
, x2a_b27_PY(ctx.GetOsKeyState(0x85).JustPressed())
, x2a_b28_PZ(ctx.GetOsKeyState(0x16).JustPressed())
, x2a_b29_PL(ctx.GetOsKeyState(0x78).JustPressed())
, x2a_b30_PR(ctx.GetOsKeyState(0x7e).JustPressed())
, x2a_b31_PDPUp(ctx.GetOsKeyState(0x1c).JustPressed())
, x2b_b24_PDPRight(ctx.GetOsKeyState(0x1d).JustPressed())
, x2b_b25_PDPDown(ctx.GetOsKeyState(0x1e).JustPressed())
, x2b_b26_PDPLeft(ctx.GetOsKeyState(0x1b).JustPressed())
, x2b_b27_PStart(ctx.GetOsKeyState(5).JustPressed()) {}
