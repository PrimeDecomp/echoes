#ifndef _STATEMACHINECOMMON
#define _STATEMACHINECOMMON

enum EStateMsg {
  kStateMsg_Activate,
  kStateMsg_Update,
  kStateMsg_Deactivate,
};

// Only the scalar argument consumed by the recovered triggers is established.
class CTriggerData {
public:
  explicit CTriggerData(float value) : mValue(value) {}
  // Guessed constructor: some Splinter calls store the integer zero for the argument.
  explicit CTriggerData(int value) : mInt(value) {}
  float GetFloat() const { return mValue; }

private:
  union {
    float mValue;
    int mInt;
  };
};

#endif
