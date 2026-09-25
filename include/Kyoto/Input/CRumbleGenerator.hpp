#ifndef _CRUMBLEGENERATOR
#define _CRUMBLEGENERATOR

#include "types.h"

#include "Kyoto/Input/CRumbleVoice.hpp"
#include "Kyoto/Input/InputTypes.hpp"

class CRumbleGenerator {
private:
  CRumbleVoice mVoice;
  float mPeriodTime;
  float mOnTime;
  EMotorState mCommand;
  EIOPort mInputIndex;
  bool mDisabled : 1;

public:
  CRumbleGenerator(EIOPort inputIndex);
  ~CRumbleGenerator();

  short Rumble(const SAdsrData& adsr, float, ERumblePriority prio);
  void Stop(short id);
  void Update(float);
  void HardStopAll();
  void SetDisabled(const bool disabled);
};

CHECK_SIZEOF(CRumbleGenerator, 0x44);

#endif // _CRUMBLEGENERATOR
