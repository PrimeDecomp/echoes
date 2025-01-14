#include "MetroidPrime/ScriptLoader/SLdrTweakGui_Completion.hpp"

SLdrTweakGui_Completion::SLdrTweakGui_Completion() {
  mainFontColor = CColor::Black();
  mainFontOutlineColor = CColor::Black();
  statsFontColor = CColor::Black();
  statsFontOutlineColor = CColor::Black();
  unlockFontColor = CColor::Black();
  unlockFontOutlineColor = CColor::Black();
  unknown_0xb6fe7398 = 0.25f;
  unknown_0x6af2871b = 0.30000001f;
  textStartDelay = 1.0f;
}

SLdrTweakGui_Completion::~SLdrTweakGui_Completion() {}

void LoadTypedefSLdrTweakGui_Completion(SLdrTweakGui_Completion& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x81fc78c2:
      sldrThis.unknown_0x81fc78c2 = rstl::string(input);
      break;
    case 0x5e7f85c7:
      sldrThis.mainFont = rstl::string(input);
      break;
    case 0x0a0d69d0:
      sldrThis.secondaryFont = rstl::string(input);
      break;
    case 0x5a24a7e4:
      sldrThis.mainFontColor = CColor(input);
      break;
    case 0xa938bf39:
      sldrThis.mainFontOutlineColor = CColor(input);
      break;
    case 0xc6cc9d0c:
      sldrThis.statsFontColor = CColor(input);
      break;
    case 0xd3a4a180:
      sldrThis.statsFontOutlineColor = CColor(input);
      break;
    case 0x07ab5642:
      sldrThis.unlockFontColor = CColor(input);
      break;
    case 0x595c63ed:
      sldrThis.unlockFontOutlineColor = CColor(input);
      break;
    case 0xb6fe7398:
      sldrThis.unknown_0xb6fe7398 = input.ReadFloat();
      break;
    case 0x6af2871b:
      sldrThis.unknown_0x6af2871b = input.ReadFloat();
      break;
    case 0x2955d055:
      sldrThis.textStartDelay = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
