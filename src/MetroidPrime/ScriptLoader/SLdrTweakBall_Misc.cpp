#include "MetroidPrime/ScriptLoader/SLdrTweakBall_Misc.hpp"

SLdrTweakBall_Misc::SLdrTweakBall_Misc() {
  unknown_0x13cfde23 = 0.69999999f;
  unknown_0xf3499713 = 1.0f;
  unknown_0x895a47fb = 200.0f;
  ballTouchRadius = 0.69999999f;
  darkWorldLightRadius = 5.0f;
  unknown_0xad662ae9 = 5.0f;
  unknown_0xb0575d4e = 5.0f;
}

SLdrTweakBall_Misc::~SLdrTweakBall_Misc() {}

void LoadTypedefSLdrTweakBall_Misc(SLdrTweakBall_Misc& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x13cfde23:
      sldrThis.unknown_0x13cfde23 = input.ReadFloat();
      break;
    case 0xf3499713:
      sldrThis.unknown_0xf3499713 = input.ReadFloat();
      break;
    case 0x895a47fb:
      sldrThis.unknown_0x895a47fb = input.ReadFloat();
      break;
    case 0xbb751ca2:
      sldrThis.ballTouchRadius = input.ReadFloat();
      break;
    case 0x9f73ace1:
      sldrThis.darkWorldLightRadius = input.ReadFloat();
      break;
    case 0xad662ae9:
      sldrThis.unknown_0xad662ae9 = input.ReadFloat();
      break;
    case 0xb0575d4e:
      sldrThis.unknown_0xb0575d4e = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
