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
  : mDt(other.mDt)
  , mControllerIdx(other.mControllerIdx)
  , mAnaLeftX(other.mAnaLeftX)
  , mAnaLeftY(other.mAnaLeftY)
  , mAnaRightX(other.mAnaRightX)
  , mAnaRightY(other.mAnaRightY)
  , mAnaLeftTrigger(other.mAnaLeftTrigger)
  , mAnaRightTrigger(other.mAnaRightTrigger)
  , mAnaLeftTriggerP(other.mAnaLeftTriggerP)
  , mAnaRightTriggerP(other.mAnaRightTriggerP)
  , btns1(other.btns1)
  , btns2(other.btns2)
  , btns3(other.btns3)
  , btns4(other.btns4) {}

  void InitializeAnalog(float leftDiv, float rightDiv);

  CFinalInput ScaleAnalogueSticks(float leftDiv, float rightDiv) const;

  float DeltaTime() const { return mDt; }

  float ALAUp() const { return mAnaLeftY > 0.f ? mAnaLeftY : 0.f; }

  float ALADown() const { return mAnaLeftY < 0.f ? -mAnaLeftY : 0.f; }

  float ALALeft() const { return mAnaLeftX < 0.f ? -mAnaLeftX : 0.f; }

  float ALARight() const { return mAnaLeftX > 0.f ? mAnaLeftX : 0.f; }

  float ARAUp() const { return mAnaRightY > 0.f ? mAnaRightY : 0.f; }

  float ARADown() const { return mAnaRightY < 0.f ? -mAnaRightY : 0.f; }

  float ARALeft() const { return mAnaRightX < 0.f ? -mAnaRightX : 0.f; }

  float ARARight() const { return mAnaRightX > 0.f ? mAnaRightX : 0.f; }

  float ALTrigger() const { return mAnaLeftTrigger; }

  float ARTrigger() const { return mAnaRightTrigger; }

  float ADPUp() const { return mDPUp ? 1.f : 0.f; }

  float ADPDown() const { return mDPDown ? 1.f : 0.f; }

  float ADPLeft() const { return mDPLeft ? 1.f : 0.f; }

  float ADPRight() const { return mDPRight ? 1.f : 0.f; }

  float AA() const { return mA ? 1.f : 0.f; }

  float AB() const { return mB ? 1.f : 0.f; }

  float AX() const { return mX ? 1.f : 0.f; }

  float AY() const { return mY ? 1.f : 0.f; }

  float AZ() const { return mZ ? 1.f : 0.f; }

  float AL() const { return mL ? 1.f : 0.f; }

  float AR() const { return mR ? 1.f : 0.f; }

  float AStart() const { return mStart ? 1.f : 0.f; }

  bool DLAUp() const { return kInput_AnalogOnThreshhold < mAnaLeftY ? true : false; }

  bool DLADown() const { return -kInput_AnalogOnThreshhold > mAnaLeftY ? true : false; }

  bool DLALeft() const { return -kInput_AnalogOnThreshhold > mAnaLeftX ? true : false; }

  bool DLARight() const { return kInput_AnalogOnThreshhold < mAnaLeftX ? true : false; }

  bool DRAUp() const { return kInput_AnalogOnThreshhold < mAnaRightY ? true : false; }

  bool DRADown() const { return -kInput_AnalogOnThreshhold > mAnaRightY ? true : false; }

  bool DRALeft() const { return -kInput_AnalogOnThreshhold > mAnaRightX ? true : false; }

  bool DRARight() const { return kInput_AnalogOnThreshhold < mAnaRightX ? true : false; }

  bool DLTrigger() const {
    return kInput_AnalogTriggerOnThreshhold < mAnaLeftTrigger ? true : false;
  }

  bool DRTrigger() const {
    return kInput_AnalogTriggerOnThreshhold < mAnaRightTrigger ? true : false;
  }

  bool DDPUp() const { return mDPUp; }

  bool DDPDown() const { return mDPDown; }

  bool DDPLeft() const { return mDPLeft; }

  bool DDPRight() const { return mDPRight; }

  bool DA() const { return mA; }

  bool DB() const { return mB; }

  bool DX() const { return mX; }

  bool DY() const { return mY; }

  bool DZ() const { return mZ; }

  bool DL() const { return mL; }

  bool DR() const { return mR; }

  bool DStart() const { return mStart; }

  bool PLAUp() const { return kInput_AnalogOnThreshhold < mAnaLeftY && mEnableAnaLeftYP ? true : false; }

  bool PLADown() const { return -kInput_AnalogOnThreshhold > mAnaLeftY && mEnableAnaLeftYP ? true : false; }

  bool PLALeft() const { return -kInput_AnalogOnThreshhold > mAnaLeftX && mEnableAnaLeftXP ? true : false; }

  bool PLARight() const { return kInput_AnalogOnThreshhold < mAnaLeftX && mEnableAnaLeftXP ? true : false; }

  bool PRAUp() const { return kInput_AnalogOnThreshhold < mAnaRightY && mEnableAnaRightYP ? true : false; }

  bool PRADown() const { return -kInput_AnalogOnThreshhold > mAnaRightY && mEnableAnaRightYP ? true : false; }

  bool PRALeft() const { return -kInput_AnalogOnThreshhold > mAnaRightX && mEnableAnaRightXP ? true : false; }

  bool PRARight() const { return kInput_AnalogOnThreshhold < mAnaRightX && mEnableAnaRightXP ? true : false; }

  bool PLTrigger() const { return kInput_AnalogTriggerOnThreshhold < mAnaLeftTriggerP ? true : false; }

  bool PRTrigger() const { return kInput_AnalogTriggerOnThreshhold < mAnaRightTriggerP ? true : false; }

  bool PDPUp() const { return mPDPUp; }
  
  bool PDPDown() const { return mPDPDown; }
  
  bool PDPLeft() const { return mPDPLeft; }
  
  bool PDPRight() const { return mPDPRight; }

  bool PA() const { return mPA; }

  bool PB() const { return mPB; }

  bool PX() const { return mPX; }

  bool PY() const { return mPY; }

  bool PZ() const { return mPZ; }

  bool PL() const { return mPL; }

  bool PR() const { return mPR; }

  bool PStart() const { return mPStart; }

private:
  float mDt;
  uint mControllerIdx;
  float mAnaLeftX;
  float mAnaLeftY;
  float mAnaRightX;
  float mAnaRightY;
  float mAnaLeftTrigger;
  float mAnaRightTrigger;
  float mAnaLeftTriggerP;
  float mAnaRightTriggerP;

  // Echoes packs Prime's analog press flags into the button bitfield.
  union {
    struct {
      uchar mEnableAnaLeftXP : 1;
      uchar mEnableAnaLeftYP : 1;
      uchar mEnableAnaRightXP : 1;
      uchar mEnableAnaRightYP : 1;
      uchar mA : 1;
      uchar mB : 1;
      uchar mX : 1;
      uchar mY : 1;
    };
    uchar btns1;
  };
  union {
    struct {
      uchar mZ : 1;
      uchar mL : 1;
      uchar mR : 1;
      uchar mDPUp : 1;
      uchar mDPRight : 1;
      uchar mDPDown : 1;
      uchar mDPLeft : 1;
      uchar mStart : 1;
    };
    uchar btns2;
  };
  union {
    struct {
      uchar mPA : 1;
      uchar mPB : 1;
      uchar mPX : 1;
      uchar mPY : 1;
      uchar mPZ : 1;
      uchar mPL : 1;
      uchar mPR : 1;
      uchar mPDPUp : 1;
    };
    uchar btns3;
  };
  union {
    struct {
      uchar mPDPRight : 1;
      uchar mPDPDown : 1;
      uchar mPDPLeft : 1;
      uchar mPStart : 1;
    };
    uchar btns4;
  };
};

CHECK_SIZEOF(CFinalInput, 0x2c)

#ifdef __MWERKS__
#pragma cpp_extensions reset
#endif

#endif // _CFINALINPUT
