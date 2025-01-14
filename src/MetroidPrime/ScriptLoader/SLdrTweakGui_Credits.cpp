#include "MetroidPrime/ScriptLoader/SLdrTweakGui_Credits.hpp"

SLdrTweakGui_Credits::SLdrTweakGui_Credits() {
  fontColor = CColor::Black();
  fontOutlineColor = CColor::Black();
  totalTime = 191.0f;
  textFadeTime = 2.0f;
  movieFadeTime = 2.0f;
}

SLdrTweakGui_Credits::~SLdrTweakGui_Credits() {}

void LoadTypedefSLdrTweakGui_Credits(SLdrTweakGui_Credits& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x81fc78c2:
      sldrThis.unknown_0x81fc78c2 = rstl::string(input);
      break;
    case 0x2bcd300d:
      sldrThis.unknown_0x2bcd300d = rstl::string(input);
      break;
    case 0xcef90c00:
      sldrThis.alternateFont = rstl::string(input);
      break;
    case 0x1a96ec67:
      sldrThis.fontColor = CColor(input);
      break;
    case 0x844ab6b0:
      sldrThis.fontOutlineColor = CColor(input);
      break;
    case 0x19686bf6:
      sldrThis.totalTime = input.ReadFloat();
      break;
    case 0x52985ad1:
      sldrThis.textFadeTime = input.ReadFloat();
      break;
    case 0xf0f977e6:
      sldrThis.movieFadeTime = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
