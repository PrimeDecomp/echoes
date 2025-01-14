#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun_Beam_Misc.hpp"

SLdrTweakPlayerGun_Beam_Misc::SLdrTweakPlayerGun_Beam_Misc() {
  aiBurnDamage = 0.25f;
  unknown_0x4848f444 = 5.0f;
  maxAbsorbedPhazonShots = 5;
  unknown_0x3ae5d1fa = 0.75f;
}

SLdrTweakPlayerGun_Beam_Misc::~SLdrTweakPlayerGun_Beam_Misc() {}

void LoadTypedefSLdrTweakPlayerGun_Beam_Misc(SLdrTweakPlayerGun_Beam_Misc& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x8aacfc27:
      LoadTypedefSLdrTDamageInfo(sldrThis.unknown_0x8aacfc27, input);
      break;
    case 0xa054ff1c:
      LoadTypedefSLdrTDamageInfo(sldrThis.unknown_0xa054ff1c, input);
      break;
    case 0xabfa93e9:
      LoadTypedefSLdrTDamageInfo(sldrThis.imploderAnnihilator, input);
      break;
    case 0xf8f9bf33:
      sldrThis.aiBurnDamage = input.ReadFloat();
      break;
    case 0x4848f444:
      sldrThis.unknown_0x4848f444 = input.ReadFloat();
      break;
    case 0x1e710222:
      sldrThis.maxAbsorbedPhazonShots = input.ReadInt32();
      break;
    case 0x3ae5d1fa:
      sldrThis.unknown_0x3ae5d1fa = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
