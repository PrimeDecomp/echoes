#include "MetroidPrime/ScriptLoader/SLdrTweakGuiColors_HUDColorsTypedef.hpp"

SLdrTweakGuiColors_HUDColorsTypedef::SLdrTweakGuiColors_HUDColorsTypedef() {
  unknown_0xc8ddc662 = CColor::Black();
  threatGroupActiveColor = CColor::Black();
  threatGroupInactiveColor = CColor::Black();
  unknown_0xa6609cc5 = CColor::Black();
  missileGroupActiveColor = CColor::Black();
  missileGroupInactiveColor = CColor::Black();
  unknown_0xdcaab836 = CColor::Black();
  energyBarFilledColor = CColor::Black();
  energyBarShadowColor = CColor::Black();
  energyBarEmptyColor = CColor::Black();
  energyTanksFilledColor = CColor::Black();
  energyTanksEmptyColor = CColor::Black();
  radarWidgetColor = CColor::Black();
  activeTextForegroundColor = CColor::Black();
  inactiveTextForegroundColor = CColor::Black();
  textShadowOutlineColor = CColor::Black();
}

SLdrTweakGuiColors_HUDColorsTypedef::~SLdrTweakGuiColors_HUDColorsTypedef() {}

void LoadTypedefSLdrTweakGuiColors_HUDColorsTypedef(SLdrTweakGuiColors_HUDColorsTypedef& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xc8ddc662:
      sldrThis.unknown_0xc8ddc662 = CColor(input);
      break;
    case 0x2f3cacaf:
      sldrThis.threatGroupActiveColor = CColor(input);
      break;
    case 0x74e5ffa1:
      sldrThis.threatGroupInactiveColor = CColor(input);
      break;
    case 0xa6609cc5:
      sldrThis.unknown_0xa6609cc5 = CColor(input);
      break;
    case 0xcbb3fb76:
      sldrThis.missileGroupActiveColor = CColor(input);
      break;
    case 0xd110a12f:
      sldrThis.missileGroupInactiveColor = CColor(input);
      break;
    case 0xdcaab836:
      sldrThis.unknown_0xdcaab836 = CColor(input);
      break;
    case 0xacf62d93:
      sldrThis.energyBarFilledColor = CColor(input);
      break;
    case 0xb9a9fc6e:
      sldrThis.energyBarShadowColor = CColor(input);
      break;
    case 0x37e381c2:
      sldrThis.energyBarEmptyColor = CColor(input);
      break;
    case 0x4377e677:
      sldrThis.energyTanksFilledColor = CColor(input);
      break;
    case 0x63384f81:
      sldrThis.energyTanksEmptyColor = CColor(input);
      break;
    case 0xa709db40:
      sldrThis.radarWidgetColor = CColor(input);
      break;
    case 0xaa4a5604:
      sldrThis.activeTextForegroundColor = CColor(input);
      break;
    case 0x6cccdf8f:
      sldrThis.inactiveTextForegroundColor = CColor(input);
      break;
    case 0x0daa7d80:
      sldrThis.textShadowOutlineColor = CColor(input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
