#include "MetroidPrime/ScriptLoader/SLdrTweakGuiColors_Multiplayer.hpp"

SLdrTweakGuiColors_Multiplayer::SLdrTweakGuiColors_Multiplayer() {
  scoreTextColor = CColor::Black();
  unknown_0xa09caefe = CColor::Black();
  timerTextColor = CColor::Black();
  timerTextBlinkColor = CColor::Black();
  unknown_0xec4197e3 = CColor::Black();
  unknown_0x823e2fb3 = CColor::Black();
  unknown_0x95cc4ed8 = CColor::Black();
  unknown_0xdb2ca6ff = CColor::Black();
  lockonIndicatorOnColor = CColor::Black();
  lockonIndicatorOffColor = CColor::Black();
}

SLdrTweakGuiColors_Multiplayer::~SLdrTweakGuiColors_Multiplayer() {}

void LoadTypedefSLdrTweakGuiColors_Multiplayer(SLdrTweakGuiColors_Multiplayer& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x23cac744:
      sldrThis.scoreTextColor = CColor(input);
      break;
    case 0xa09caefe:
      sldrThis.unknown_0xa09caefe = CColor(input);
      break;
    case 0x6bf04ff9:
      sldrThis.timerTextColor = CColor(input);
      break;
    case 0x4280d00a:
      sldrThis.timerTextBlinkColor = CColor(input);
      break;
    case 0xec4197e3:
      sldrThis.unknown_0xec4197e3 = CColor(input);
      break;
    case 0x823e2fb3:
      sldrThis.unknown_0x823e2fb3 = CColor(input);
      break;
    case 0x95cc4ed8:
      sldrThis.unknown_0x95cc4ed8 = CColor(input);
      break;
    case 0xdb2ca6ff:
      sldrThis.unknown_0xdb2ca6ff = CColor(input);
      break;
    case 0x03d27ffd:
      sldrThis.lockonIndicatorOnColor = CColor(input);
      break;
    case 0x4c215775:
      sldrThis.lockonIndicatorOffColor = CColor(input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
