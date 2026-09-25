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
  float GetFloat() const { return mValue; }

private:
  float mValue;
};

#endif
