#include "MetroidPrime/ScriptLoader/SLdrTweakGuiColors_TurretHudTypedef.hpp"

SLdrTweakGuiColors_TurretHudTypedef::SLdrTweakGuiColors_TurretHudTypedef() {
  frameColor = CColor::Black();
  fontColor = CColor::Black();
  fontOutlineColor = CColor::Black();
  energyBarFillColor = CColor::Black();
  energyBarShadowColor = CColor::Black();
  energyBarEmptyColor = CColor::Black();
}

SLdrTweakGuiColors_TurretHudTypedef::~SLdrTweakGuiColors_TurretHudTypedef() {}

void LoadTypedefSLdrTweakGuiColors_TurretHudTypedef(SLdrTweakGuiColors_TurretHudTypedef& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xa485372c:
      sldrThis.frameColor = CColor(input);
      break;
    case 0x1a96ec67:
      sldrThis.fontColor = CColor(input);
      break;
    case 0x844ab6b0:
      sldrThis.fontOutlineColor = CColor(input);
      break;
    case 0xbdb697a9:
      sldrThis.energyBarFillColor = CColor(input);
      break;
    case 0xb9a9fc6e:
      sldrThis.energyBarShadowColor = CColor(input);
      break;
    case 0x37e381c2:
      sldrThis.energyBarEmptyColor = CColor(input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
