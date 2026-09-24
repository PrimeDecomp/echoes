#ifndef _CRUMBLEGENERATOR
#define _CRUMBLEGENERATOR

#include "types.h"

#include "Kyoto/Input/CRumbleVoice.hpp"
#include "Kyoto/Input/InputTypes.hpp"

class CRumbleGenerator {
private:
  CRumbleVoice x0_voice;
  float x30_periodTime;
  float x34_onTime;
  EMotorState x38_command;
  EIOPort x3c_inputIndex;
  bool x40_24_disabled : 1;

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
