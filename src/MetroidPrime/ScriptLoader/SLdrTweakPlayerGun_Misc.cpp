#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun_Misc.hpp"

SLdrTweakPlayerGun_Misc::SLdrTweakPlayerGun_Misc() {
  upLookAngle = 22.0f;
  downLookAngle = 0.0f;
  verticalSpread = 7.0f;
  horizontalSpread = 7.0f;
  highVerticalSpread = 7.0f;
  highHorizontalSpread = 7.0f;
  lowVerticalSpread = 7.0f;
  lowHorizontalSpread = 7.0f;
  aimVerticalSpeed = 4.0f;
  aimHorizontalSpeed = 10.0f;
  hologramDisplayTime = 0.0625f;
  gunTransformTime = 0.25f;
}

SLdrTweakPlayerGun_Misc::~SLdrTweakPlayerGun_Misc() {}

void LoadTypedefSLdrTweakPlayerGun_Misc(SLdrTweakPlayerGun_Misc& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xe8bb7e3c:
      sldrThis.upLookAngle = input.ReadFloat();
      break;
    case 0x5ed7e0bd:
      sldrThis.downLookAngle = input.ReadFloat();
      break;
    case 0x842ae0b4:
      sldrThis.verticalSpread = input.ReadFloat();
      break;
    case 0x8c29e91c:
      sldrThis.horizontalSpread = input.ReadFloat();
      break;
    case 0x7d5a6c93:
      sldrThis.highVerticalSpread = input.ReadFloat();
      break;
    case 0xb2e26d02:
      sldrThis.highHorizontalSpread = input.ReadFloat();
      break;
    case 0xd81d1450:
      sldrThis.lowVerticalSpread = input.ReadFloat();
      break;
    case 0x0cebb5c6:
      sldrThis.lowHorizontalSpread = input.ReadFloat();
      break;
    case 0x904cd49d:
      sldrThis.aimVerticalSpeed = input.ReadFloat();
      break;
    case 0xfccddb00:
      sldrThis.aimHorizontalSpeed = input.ReadFloat();
      break;
    case 0xf355d075:
      sldrThis.hologramDisplayTime = input.ReadFloat();
      break;
    case 0x9262a722:
      sldrThis.gunTransformTime = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
