#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_Frozen.hpp"

SLdrTweakPlayer_Frozen::SLdrTweakPlayer_Frozen() {
  frozenTimer = 18.0f;
  frozenJumpCounter = 4;
  frozenDamageThreshold = 20.0f;
}

SLdrTweakPlayer_Frozen::~SLdrTweakPlayer_Frozen() {}

void LoadTypedefSLdrTweakPlayer_Frozen(SLdrTweakPlayer_Frozen& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xb3f20575:
      sldrThis.frozenTimer = input.ReadFloat();
      break;
    case 0xb851d54f:
      sldrThis.frozenJumpCounter = input.ReadInt32();
      break;
    case 0x33b040bf:
      sldrThis.frozenDamageThreshold = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
