#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_DarkWorld.hpp"

SLdrTweakPlayer_DarkWorld::SLdrTweakPlayer_DarkWorld() {
  damageGracePeriod = 2.0f;
  unknown_0xa4e33ef0 = 4.0f;
  darkSuitDamageReduction = 1.0f;
  unknown_0xee452490 = 0.34999999f;
  unknown_0x19275a97 = 0.5f;
}

SLdrTweakPlayer_DarkWorld::~SLdrTweakPlayer_DarkWorld() {}

void LoadTypedefSLdrTweakPlayer_DarkWorld(SLdrTweakPlayer_DarkWorld& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x4985bc73:
      sldrThis.damageGracePeriod = input.ReadFloat();
      break;
    case 0xa4e33ef0:
      sldrThis.unknown_0xa4e33ef0 = input.ReadFloat();
      break;
    case 0xf9bf59a2:
      LoadTypedefSLdrDamageInfo(sldrThis.damagePerSecond, input);
      break;
    case 0x333b7549:
      sldrThis.darkSuitDamageReduction = input.ReadFloat();
      break;
    case 0xee452490:
      sldrThis.unknown_0xee452490 = input.ReadFloat();
      break;
    case 0x19275a97:
      sldrThis.unknown_0x19275a97 = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
