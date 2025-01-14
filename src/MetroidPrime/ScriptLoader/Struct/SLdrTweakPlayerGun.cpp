#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakPlayerGun.hpp"

SLdrTweakPlayerGun::SLdrTweakPlayerGun() {}

SLdrTweakPlayerGun::~SLdrTweakPlayerGun() {}

void LoadTypedefSLdrTweakPlayerGun(SLdrTweakPlayerGun& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x7fda1466:
      sldrThis.instanceName = rstl::string(input);
      break;
    case 0xb82ed424:
      LoadTypedefSLdrTweakPlayerGun_Misc(sldrThis.misc, input);
      break;
    case 0x6b6bdc47:
      LoadTypedefSLdrTweakPlayerGun_Holstering(sldrThis.holstering, input);
      break;
    case 0x87882cb0:
      LoadTypedefSLdrTweakPlayerGun_Position(sldrThis.position, input);
      break;
    case 0x255007ad:
      LoadTypedefSLdrTweakPlayerGun_Arm_Position(sldrThis.armPosition, input);
      break;
    case 0x83d758ab:
      LoadTypedefSLdrTweakPlayerGun_Weapons(sldrThis.weapons, input);
      break;
    case 0x888c8775:
      LoadTypedefSLdrTweakPlayerGun_UnknownStruct1(sldrThis.combos, input);
      break;
    case 0xaaebb73e:
      LoadTypedefSLdrTweakPlayerGun_Beam_Misc(sldrThis.beamMisc, input);
      break;
    case 0x8da058fe:
      LoadTypedefSLdrTweakPlayerGun_RicochetDamage_Factor(sldrThis.ricochetDamageFactor, input);
      break;
    case 0xffdb4bb7:
      LoadTypedefSLdrCameraShakerData(sldrThis.recoil, input);
      break;
    case 0x937a35bd:
      LoadTypedefSLdrCameraShakerData(sldrThis.comboRecoil, input);
      break;
    case 0x26196738:
      LoadTypedefSLdrCameraShakerData(sldrThis.projectileRecoil, input);
      break;
    case 0xf40808c9:
      LoadTypedefSLdrCameraShakerData(sldrThis.flameThrower, input);
      break;
    case 0x9a6d7a31:
      LoadTypedefSLdrCameraShakerData(sldrThis.waveBuster, input);
      break;
    case 0x12f14c5a:
      LoadTypedefSLdrCameraShakerData(sldrThis.projectileImpact, input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
