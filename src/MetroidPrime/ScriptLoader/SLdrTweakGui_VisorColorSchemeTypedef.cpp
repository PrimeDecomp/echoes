#include "MetroidPrime/ScriptLoader/SLdrTweakGui_VisorColorSchemeTypedef.hpp"

SLdrTweakGui_VisorColorSchemeTypedef::SLdrTweakGui_VisorColorSchemeTypedef() {
  hudHue = CColor::Black();
  unknown = CColor::Black();
}

SLdrTweakGui_VisorColorSchemeTypedef::~SLdrTweakGui_VisorColorSchemeTypedef() {}

void LoadTypedefSLdrTweakGui_VisorColorSchemeTypedef(SLdrTweakGui_VisorColorSchemeTypedef& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x59148173:
      sldrThis.hudHue = CColor(input);
      break;
    case 0x9da5d1d7:
      sldrThis.unknown = CColor(input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
