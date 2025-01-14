#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_Shield.hpp"

SLdrTweakPlayer_Shield::SLdrTweakPlayer_Shield() {
  maxEnergy = 2.0f;
  usageRate = 1.5f;
  rechargeRate = 0.5f;
  allowsMotion = true;
}

SLdrTweakPlayer_Shield::~SLdrTweakPlayer_Shield() {}

void LoadTypedefSLdrTweakPlayer_Shield(SLdrTweakPlayer_Shield& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xd42fa1c1:
      sldrThis.maxEnergy = input.ReadFloat();
      break;
    case 0x787855e6:
      sldrThis.usageRate = input.ReadFloat();
      break;
    case 0x5dadd6ab:
      sldrThis.rechargeRate = input.ReadFloat();
      break;
    case 0x59efbb34:
      sldrThis.allowsMotion = input.ReadBool();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
