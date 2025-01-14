#include "MetroidPrime/ScriptLoader/SLdrTweakGame_TimeLimitChoices.hpp"

SLdrTweakGame_TimeLimitChoices::SLdrTweakGame_TimeLimitChoices() {
  timeLimit0 = 0.0f;
  timeLimit1 = 3.0f;
  timeLimit2 = 5.0f;
  timeLimit3 = 10.0f;
  timeLimit4 = 20.0f;
}

SLdrTweakGame_TimeLimitChoices::~SLdrTweakGame_TimeLimitChoices() {}

void LoadTypedefSLdrTweakGame_TimeLimitChoices(SLdrTweakGame_TimeLimitChoices& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x779e8ff4:
      sldrThis.timeLimit0 = input.ReadFloat();
      break;
    case 0xbcc25c51:
      sldrThis.timeLimit1 = input.ReadFloat();
      break;
    case 0x3a562eff:
      sldrThis.timeLimit2 = input.ReadFloat();
      break;
    case 0xf10afd5a:
      sldrThis.timeLimit3 = input.ReadFloat();
      break;
    case 0xec0fcde2:
      sldrThis.timeLimit4 = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
