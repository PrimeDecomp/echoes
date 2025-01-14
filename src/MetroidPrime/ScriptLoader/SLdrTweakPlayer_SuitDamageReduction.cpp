#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_SuitDamageReduction.hpp"

SLdrTweakPlayer_SuitDamageReduction::SLdrTweakPlayer_SuitDamageReduction() {
  varia = 0.1f;
  dark = 0.2f;
  light = 0.5f;
}

SLdrTweakPlayer_SuitDamageReduction::~SLdrTweakPlayer_SuitDamageReduction() {}

void LoadTypedefSLdrTweakPlayer_SuitDamageReduction(SLdrTweakPlayer_SuitDamageReduction& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xdf131ecd:
      sldrThis.varia = input.ReadFloat();
      break;
    case 0x908a8e6c:
      sldrThis.dark = input.ReadFloat();
      break;
    case 0x95700a27:
      sldrThis.light = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
