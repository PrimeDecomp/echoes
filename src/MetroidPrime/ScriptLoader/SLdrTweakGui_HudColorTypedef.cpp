#include "MetroidPrime/ScriptLoader/SLdrTweakGui_HudColorTypedef.hpp"

SLdrTweakGui_HudColorTypedef::SLdrTweakGui_HudColorTypedef() {
  energyBarFilledColor = CColor::Black();
  energyBarEmptyColor = CColor::Black();
  energyBarShadowColor = CColor::Black();
  energyTanksFilledColor = CColor::Black();
  energyTanksEmptyColor = CColor::Black();
  unknown_0x1fd3d43a = CColor::Black();
  unknown_0xe1ff2a4f = CColor::Black();
  unknown_0x9cfb8a36 = CColor::Black();
  unknown_0x89a45bcb = CColor::Black();
  unknown_0xc2a8ccc6 = CColor::Black();
}

SLdrTweakGui_HudColorTypedef::~SLdrTweakGui_HudColorTypedef() {}

void LoadTypedefSLdrTweakGui_HudColorTypedef(SLdrTweakGui_HudColorTypedef& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xacf62d93:
      sldrThis.energyBarFilledColor = CColor(input);
      break;
    case 0x37e381c2:
      sldrThis.energyBarEmptyColor = CColor(input);
      break;
    case 0xb9a9fc6e:
      sldrThis.energyBarShadowColor = CColor(input);
      break;
    case 0x4377e677:
      sldrThis.energyTanksFilledColor = CColor(input);
      break;
    case 0x63384f81:
      sldrThis.energyTanksEmptyColor = CColor(input);
      break;
    case 0x1fd3d43a:
      sldrThis.unknown_0x1fd3d43a = CColor(input);
      break;
    case 0xe1ff2a4f:
      sldrThis.unknown_0xe1ff2a4f = CColor(input);
      break;
    case 0x9cfb8a36:
      sldrThis.unknown_0x9cfb8a36 = CColor(input);
      break;
    case 0x89a45bcb:
      sldrThis.unknown_0x89a45bcb = CColor(input);
      break;
    case 0xc2a8ccc6:
      sldrThis.unknown_0xc2a8ccc6 = CColor(input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
