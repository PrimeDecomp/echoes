#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun_RicochetDamage_Factor.hpp"

SLdrTweakPlayerGun_RicochetDamage_Factor::SLdrTweakPlayerGun_RicochetDamage_Factor() {
  powerBeam = 0.1f;
  darkBeam = 0.1f;
  lightBeam = 0.1f;
  annihilatorBeam = 0.1f;
  phazonBeam = 0.1f;
  missile = 0.1f;
}

SLdrTweakPlayerGun_RicochetDamage_Factor::~SLdrTweakPlayerGun_RicochetDamage_Factor() {}

void LoadTypedefSLdrTweakPlayerGun_RicochetDamage_Factor(SLdrTweakPlayerGun_RicochetDamage_Factor& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x5d623269:
      sldrThis.powerBeam = input.ReadFloat();
      break;
    case 0x4f420191:
      sldrThis.darkBeam = input.ReadFloat();
      break;
    case 0x7b529049:
      sldrThis.lightBeam = input.ReadFloat();
      break;
    case 0x503dddca:
      sldrThis.annihilatorBeam = input.ReadFloat();
      break;
    case 0xf668c245:
      sldrThis.phazonBeam = input.ReadFloat();
      break;
    case 0x01234cd8:
      sldrThis.missile = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
