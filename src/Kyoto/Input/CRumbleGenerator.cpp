#include "Kyoto/Input/CRumbleGenerator.hpp"

CRumbleGenerator::CRumbleGenerator(EIOPort inputIndex)
: x3c_inputIndex(inputIndex), x40_24_disabled(false) {
  HardStopAll();
}

CRumbleGenerator::~CRumbleGenerator() { HardStopAll(); }

void CRumbleGenerator::Stop(short id) {
  if (id != -1)
    x0_voice.Deactivate(id, false);
}

short CRumbleGenerator::Rumble(const SAdsrData& adsr, float gain, ERumblePriority prio) {
  ushort freeChan = x0_voice.GetFreeChannel();
  if (prio >= x0_voice.GetPriority(freeChan)) {
    x30_periodTime = 0.f;
    x34_onTime = 0.f;
    return x0_voice.Activate(adsr, freeChan, gain, prio);
  }
  return -1;
}

void CRumbleGenerator::Update(float dt) {
  if (!x40_24_disabled) {
    bool updated = false;
    const float intensity = x0_voice.GetIntensity();
    if (!x0_voice.Update(dt) || intensity <= 0.f) {
      x30_periodTime = 0.f;
      x34_onTime = 0.f;
      if (x38_command != kMS_Stop) {
        x38_command = kMS_Stop;
        updated = true;
      }
    } else {
      x30_periodTime += dt;
      if (x30_periodTime >= 1.f / (30.f * intensity)) {
        x30_periodTime = 0.f;
        if (x38_command != kMS_Rumble) {
          x38_command = kMS_Rumble;
          updated = true;
        }
      } else {
        x34_onTime += dt;
        if (x34_onTime >= (1.f / 30.f)) {
          x34_onTime = 0.f;
          if (x38_command != kMS_Stop) {
            x38_command = kMS_Stop;
            updated = true;
          }
        }
      }
    }
    if (updated) {
      PADControlMotor(x3c_inputIndex, x38_command);
    }
  }
}

void CRumbleGenerator::HardStopAll() {

  x30_periodTime = 0.f;
  x34_onTime = 0.f;
  x38_command = kMS_Stop;
  x0_voice.HardReset();
  PADControlMotor(x3c_inputIndex, kMS_StopHard);
}

void CRumbleGenerator::SetDisabled(const bool disabled) {
  if (disabled) {
    HardStopAll();
  }

  x40_24_disabled = disabled;
}
