#ifndef _CFINALINPUT
#define _CFINALINPUT

#include <types.h>

#ifdef __MWERKS__
#pragma cpp_extensions on
#endif

class CControllerGamepadData;
class COsContext;

class CFinalInput {
  static const float kInput_AnalogOnThreshhold;
  static const float kInput_AnalogTriggerOnThreshhold;

public:
  CFinalInput();
  CFinalInput(int channel, float dt, const CControllerGamepadData& data, float leftDif,
              float rightDiv);
  CFinalInput(int channel, float dt, const COsContext& context);
  CFinalInput(const CFinalInput& other)
  : x0_dt(other.x0_dt)
  , x4_controllerIdx(other.x4_controllerIdx)
  , x8_anaLeftX(other.x8_anaLeftX)
  , xc_anaLeftY(other.xc_anaLeftY)
  , x10_anaRightX(other.x10_anaRightX)
  , x14_anaRightY(other.x14_anaRightY)
  , x18_anaLeftTrigger(other.x18_anaLeftTrigger)
  , x1c_anaRightTrigger(other.x1c_anaRightTrigger)
  , x20_anaLeftTriggerP(other.x20_anaLeftTriggerP)
  , x24_anaRightTriggerP(other.x24_anaRightTriggerP)
  , btns1(other.btns1)
  , btns2(other.btns2)
  , btns3(other.btns3)
  , btns4(other.btns4) {}

  void InitializeAnalog(float leftDiv, float rightDiv);

  CFinalInput ScaleAnalogueSticks(float leftDiv, float rightDiv) const;

  float DeltaTime() const { return x0_dt; }

  float ALAUp() const { return xc_anaLeftY > 0.f ? xc_anaLeftY : 0.f; }

  float ALADown() const { return xc_anaLeftY < 0.f ? -xc_anaLeftY : 0.f; }

  float ALALeft() const { return x8_anaLeftX < 0.f ? -x8_anaLeftX : 0.f; }

  float ALARight() const { return x8_anaLeftX > 0.f ? x8_anaLeftX : 0.f; }

  float ARAUp() const { return x14_anaRightY > 0.f ? x14_anaRightY : 0.f; }

  float ARADown() const { return x14_anaRightY < 0.f ? -x14_anaRightY : 0.f; }

  float ARALeft() const { return x10_anaRightX < 0.f ? -x10_anaRightX : 0.f; }

  float ARARight() const { return x10_anaRightX > 0.f ? x10_anaRightX : 0.f; }

  float ALTrigger() const { return x18_anaLeftTrigger; }

  float ARTrigger() const { return x1c_anaRightTrigger; }

  float ADPUp() const { return x29_b27_DPUp ? 1.f : 0.f; }

  float ADPDown() const { return x29_b29_DPDown ? 1.f : 0.f; }

  float ADPLeft() const { return x29_b30_DPLeft ? 1.f : 0.f; }

  float ADPRight() const { return x29_b28_DPRight ? 1.f : 0.f; }

  float AA() const { return x28_b28_A ? 1.f : 0.f; }

  float AB() const { return x28_b29_B ? 1.f : 0.f; }

  float AX() const { return x28_b30_X ? 1.f : 0.f; }

  float AY() const { return x28_b31_Y ? 1.f : 0.f; }

  float AZ() const { return x29_b24_Z ? 1.f : 0.f; }

  float AL() const { return x29_b25_L ? 1.f : 0.f; }

  float AR() const { return x29_b26_R ? 1.f : 0.f; }

  float AStart() const { return x29_b31_Start ? 1.f : 0.f; }

  bool DLAUp() const { return kInput_AnalogOnThreshhold < xc_anaLeftY ? true : false; }

  bool DLADown() const { return -kInput_AnalogOnThreshhold > xc_anaLeftY ? true : false; }

  bool DLALeft() const { return -kInput_AnalogOnThreshhold > x8_anaLeftX ? true : false; }

  bool DLARight() const { return kInput_AnalogOnThreshhold < x8_anaLeftX ? true : false; }

  bool DRAUp() const { return kInput_AnalogOnThreshhold < x14_anaRightY ? true : false; }

  bool DRADown() const { return -kInput_AnalogOnThreshhold > x14_anaRightY ? true : false; }

  bool DRALeft() const { return -kInput_AnalogOnThreshhold > x10_anaRightX ? true : false; }

  bool DRARight() const { return kInput_AnalogOnThreshhold < x10_anaRightX ? true : false; }

  bool DLTrigger() const {
    return kInput_AnalogTriggerOnThreshhold < x18_anaLeftTrigger ? true : false;
  }

  bool DRTrigger() const {
    return kInput_AnalogTriggerOnThreshhold < x1c_anaRightTrigger ? true : false;
  }

  bool DDPUp() const { return x29_b27_DPUp; }

  bool DDPDown() const { return x29_b29_DPDown; }

  bool DDPLeft() const { return x29_b30_DPLeft; }

  bool DDPRight() const { return x29_b28_DPRight; }

  bool DA() const { return x28_b28_A; }

  bool DB() const { return x28_b29_B; }

  bool DX() const { return x28_b30_X; }

  bool DY() const { return x28_b31_Y; }

  bool DZ() const { return x29_b24_Z; }

  bool DL() const { return x29_b25_L; }

  bool DR() const { return x29_b26_R; }

  bool DStart() const { return x29_b31_Start; }

  bool PLAUp() const { return kInput_AnalogOnThreshhold < xc_anaLeftY && x28_b25_enableAnaLeftYP ? true : false; }

  bool PLADown() const { return -kInput_AnalogOnThreshhold > xc_anaLeftY && x28_b25_enableAnaLeftYP ? true : false; }

  bool PLALeft() const { return -kInput_AnalogOnThreshhold > x8_anaLeftX && x28_b24_enableAnaLeftXP ? true : false; }

  bool PLARight() const { return kInput_AnalogOnThreshhold < x8_anaLeftX && x28_b24_enableAnaLeftXP ? true : false; }

  bool PRAUp() const { return kInput_AnalogOnThreshhold < x14_anaRightY && x28_b27_enableAnaRightYP ? true : false; }

  bool PRADown() const { return -kInput_AnalogOnThreshhold > x14_anaRightY && x28_b27_enableAnaRightYP ? true : false; }

  bool PRALeft() const { return -kInput_AnalogOnThreshhold > x10_anaRightX && x28_b26_enableAnaRightXP ? true : false; }

  bool PRARight() const { return kInput_AnalogOnThreshhold < x10_anaRightX && x28_b26_enableAnaRightXP ? true : false; }

  bool PLTrigger() const { return kInput_AnalogTriggerOnThreshhold < x20_anaLeftTriggerP ? true : false; }

  bool PRTrigger() const { return kInput_AnalogTriggerOnThreshhold < x24_anaRightTriggerP ? true : false; }

  bool PDPUp() const { return x2a_b31_PDPUp; }
  
  bool PDPDown() const { return x2b_b25_PDPDown; }
  
  bool PDPLeft() const { return x2b_b26_PDPLeft; }
  
  bool PDPRight() const { return x2b_b24_PDPRight; }

  bool PA() const { return x2a_b24_PA; }

  bool PB() const { return x2a_b25_PB; }

  bool PX() const { return x2a_b26_PX; }

  bool PY() const { return x2a_b27_PY; }

  bool PZ() const { return x2a_b28_PZ; }

  bool PL() const { return x2a_b29_PL; }

  bool PR() const { return x2a_b30_PR; }

  bool PStart() const { return x2b_b27_PStart; }

private:
  float x0_dt;
  uint x4_controllerIdx;
  float x8_anaLeftX;
  float xc_anaLeftY;
  float x10_anaRightX;
  float x14_anaRightY;
  float x18_anaLeftTrigger;
  float x1c_anaRightTrigger;
  float x20_anaLeftTriggerP;
  float x24_anaRightTriggerP;

  // Echoes packs Prime's analog press flags into the button bitfield.
  union {
    struct {
      uchar x28_b24_enableAnaLeftXP : 1;
      uchar x28_b25_enableAnaLeftYP : 1;
      uchar x28_b26_enableAnaRightXP : 1;
      uchar x28_b27_enableAnaRightYP : 1;
      uchar x28_b28_A : 1;
      uchar x28_b29_B : 1;
      uchar x28_b30_X : 1;
      uchar x28_b31_Y : 1;
    };
    uchar btns1;
  };
  union {
    struct {
      uchar x29_b24_Z : 1;
      uchar x29_b25_L : 1;
      uchar x29_b26_R : 1;
      uchar x29_b27_DPUp : 1;
      uchar x29_b28_DPRight : 1;
      uchar x29_b29_DPDown : 1;
      uchar x29_b30_DPLeft : 1;
      uchar x29_b31_Start : 1;
    };
    uchar btns2;
  };
  union {
    struct {
      uchar x2a_b24_PA : 1;
      uchar x2a_b25_PB : 1;
      uchar x2a_b26_PX : 1;
      uchar x2a_b27_PY : 1;
      uchar x2a_b28_PZ : 1;
      uchar x2a_b29_PL : 1;
      uchar x2a_b30_PR : 1;
      uchar x2a_b31_PDPUp : 1;
    };
    uchar btns3;
  };
  union {
    struct {
      uchar x2b_b24_PDPRight : 1;
      uchar x2b_b25_PDPDown : 1;
      uchar x2b_b26_PDPLeft : 1;
      uchar x2b_b27_PStart : 1;
    };
    uchar btns4;
  };
};

CHECK_SIZEOF(CFinalInput, 0x2c)

#ifdef __MWERKS__
#pragma cpp_extensions reset
#endif

#endif // _CFINALINPUT
