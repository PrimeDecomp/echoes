#include "MetroidPrime/ScriptLoader/SLdrTweakGame_FragLimitChoices.hpp"

SLdrTweakGame_FragLimitChoices::SLdrTweakGame_FragLimitChoices() {
  fragLimit0 = 0;
  fragLimit1 = 5;
  fragLimit2 = 10;
  fragLimit3 = 15;
  fragLimit4 = 20;
}

SLdrTweakGame_FragLimitChoices::~SLdrTweakGame_FragLimitChoices() {}

void LoadTypedefSLdrTweakGame_FragLimitChoices(SLdrTweakGame_FragLimitChoices& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xa2023ba8:
      sldrThis.fragLimit0 = input.ReadInt32();
      break;
    case 0x1abe5ccd:
      sldrThis.fragLimit1 = input.ReadInt32();
      break;
    case 0x080bf323:
      sldrThis.fragLimit2 = input.ReadInt32();
      break;
    case 0xb0b79446:
      sldrThis.fragLimit3 = input.ReadInt32();
      break;
    case 0x2d60acff:
      sldrThis.fragLimit4 = input.ReadInt32();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
