#include "MetroidPrime/ScriptLoader/SLdrTIcon_Configurations.hpp"

SLdrTIcon_Configurations::SLdrTIcon_Configurations() {
  unknown_0x3058aff2 = 0.0f;
  unknown_0xf1d67032 = 0.0f;
  unknown_0x68341633 = 0.0f;
  unknown_0xa9bac9f3 = 0.0f;
  unknown_0x8081dc70 = 0.0f;
  unknown_0x410f03b0 = 0.0f;
  unknown_0xd8ed65b1 = 0.0f;
  unknown_0x1963ba71 = 0.0f;
  unknown_0x8a9b4eb7 = 0.0f;
}

SLdrTIcon_Configurations::~SLdrTIcon_Configurations() {}

void LoadTypedefSLdrTIcon_Configurations(SLdrTIcon_Configurations& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x3058aff2:
      sldrThis.unknown_0x3058aff2 = input.ReadFloat();
      break;
    case 0xf1d67032:
      sldrThis.unknown_0xf1d67032 = input.ReadFloat();
      break;
    case 0x68341633:
      sldrThis.unknown_0x68341633 = input.ReadFloat();
      break;
    case 0xa9bac9f3:
      sldrThis.unknown_0xa9bac9f3 = input.ReadFloat();
      break;
    case 0x8081dc70:
      sldrThis.unknown_0x8081dc70 = input.ReadFloat();
      break;
    case 0x410f03b0:
      sldrThis.unknown_0x410f03b0 = input.ReadFloat();
      break;
    case 0xd8ed65b1:
      sldrThis.unknown_0xd8ed65b1 = input.ReadFloat();
      break;
    case 0x1963ba71:
      sldrThis.unknown_0x1963ba71 = input.ReadFloat();
      break;
    case 0x8a9b4eb7:
      sldrThis.unknown_0x8a9b4eb7 = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
