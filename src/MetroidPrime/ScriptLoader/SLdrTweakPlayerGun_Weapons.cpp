#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun_Weapons.hpp"

SLdrTweakPlayerGun_Weapons::SLdrTweakPlayerGun_Weapons()
  : bomb()
  , unknown_0xe8907530(2.0)
  , unknown_0x0a9186cb(0.60000002)
  , powerBomb()
  , missile()
  , powerBeam()
  , darkBeam()
  , darkBeamBlob()
  , lightBeam()
  , annihilatorBeam()
  , phazonBeam()
{}

SLdrTweakPlayerGun_Weapons::~SLdrTweakPlayerGun_Weapons() {}

void LoadTypedefSLdrTweakPlayerGun_Weapons(SLdrTweakPlayerGun_Weapons& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x6173ad96:
      LoadTypedefSLdrTDamageInfo(sldrThis.bomb, input);
      break;
    case 0xe8907530:
      sldrThis.unknown_0xe8907530 = input.ReadFloat();
      break;
    case 0x0a9186cb:
      sldrThis.unknown_0x0a9186cb = input.ReadFloat();
      break;
    case 0xdcc0c6fb:
      LoadTypedefSLdrTDamageInfo(sldrThis.powerBomb, input);
      break;
    case 0x58f00b0a:
      LoadTypedefSLdrTDamageInfo(sldrThis.missile, input);
      break;
    case 0x1f6c1a6b:
      LoadTypedefSLdrTBeamInfo(sldrThis.powerBeam, input);
      break;
    case 0xc50f608b:
      LoadTypedefSLdrTBeamInfo(sldrThis.darkBeam, input);
      break;
    case 0x059dce11:
      LoadTypedefSLdrTDamageInfo(sldrThis.darkBeamBlob, input);
      break;
    case 0xde7a8255:
      LoadTypedefSLdrTBeamInfo(sldrThis.lightBeam, input);
      break;
    case 0x74b9b983:
      LoadTypedefSLdrTBeamInfo(sldrThis.annihilatorBeam, input);
      break;
    case 0xdd5f2e3d:
      LoadTypedefSLdrTBeamInfo(sldrThis.phazonBeam, input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
