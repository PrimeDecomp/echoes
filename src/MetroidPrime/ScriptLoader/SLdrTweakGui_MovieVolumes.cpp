#include "MetroidPrime/ScriptLoader/SLdrTweakGui_MovieVolumes.hpp"

SLdrTweakGui_MovieVolumes::SLdrTweakGui_MovieVolumes() {
  unknown_0xae149646 = 127;
  unknown_0xc1a2e858 = 127;
  unknown_0x138c3bb8 = 127;
  unknown_0xe5587648 = 127;
  unknown_0x9ed00248 = 127;
  unknown_0x6f135424 = 127;
  unknown_0xdb2260b7 = 127;
  unknown_0xf38093f5 = 127;
}

SLdrTweakGui_MovieVolumes::~SLdrTweakGui_MovieVolumes() {}

void LoadTypedefSLdrTweakGui_MovieVolumes(SLdrTweakGui_MovieVolumes& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xae149646:
      sldrThis.unknown_0xae149646 = input.ReadInt32();
      break;
    case 0xc1a2e858:
      sldrThis.unknown_0xc1a2e858 = input.ReadInt32();
      break;
    case 0x138c3bb8:
      sldrThis.unknown_0x138c3bb8 = input.ReadInt32();
      break;
    case 0xe5587648:
      sldrThis.unknown_0xe5587648 = input.ReadInt32();
      break;
    case 0x9ed00248:
      sldrThis.unknown_0x9ed00248 = input.ReadInt32();
      break;
    case 0x6f135424:
      sldrThis.unknown_0x6f135424 = input.ReadInt32();
      break;
    case 0xdb2260b7:
      sldrThis.unknown_0xdb2260b7 = input.ReadInt32();
      break;
    case 0xf38093f5:
      sldrThis.unknown_0xf38093f5 = input.ReadInt32();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
