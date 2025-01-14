#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun_UnknownStruct1.hpp"

SLdrTweakPlayerGun_UnknownStruct1::SLdrTweakPlayerGun_UnknownStruct1() {}

SLdrTweakPlayerGun_UnknownStruct1::~SLdrTweakPlayerGun_UnknownStruct1() {}

void LoadTypedefSLdrTweakPlayerGun_UnknownStruct1(SLdrTweakPlayerGun_UnknownStruct1& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xc713acf9:
      LoadTypedefSLdrTDamageInfo(sldrThis.superMissile, input);
      break;
    case 0x19468f2a:
      LoadTypedefSLdrTDamageInfo(sldrThis.darkburst, input);
      break;
    case 0x48ac6dd8:
      LoadTypedefSLdrTDamageInfo(sldrThis.sunburst, input);
      break;
    case 0xc1c315ff:
      LoadTypedefSLdrTDamageInfo(sldrThis.sonicBoom, input);
      break;
    case 0x42885c6c:
      LoadTypedefSLdrTDamageInfo(sldrThis.unknown, input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
