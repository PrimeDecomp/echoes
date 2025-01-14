#ifndef _SLDRTWEAKPLAYERGUN
#define _SLDRTWEAKPLAYERGUN

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrCameraShakerData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun_Arm_Position.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun_Beam_Misc.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun_Holstering.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun_Misc.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun_Position.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun_RicochetDamage_Factor.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun_UnknownStruct1.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun_Weapons.hpp"
#include "rstl/string.hpp"

struct SLdrTweakPlayerGun {
    SLdrTweakPlayerGun();
    ~SLdrTweakPlayerGun();

    rstl::string instanceName;
    SLdrTweakPlayerGun_Misc misc;
    SLdrTweakPlayerGun_Holstering holstering;
    SLdrTweakPlayerGun_Position position;
    SLdrTweakPlayerGun_Arm_Position armPosition;
    SLdrTweakPlayerGun_Weapons weapons;
    SLdrTweakPlayerGun_UnknownStruct1 combos;
    SLdrTweakPlayerGun_Beam_Misc beamMisc;
    SLdrTweakPlayerGun_RicochetDamage_Factor ricochetDamageFactor;
    SLdrCameraShakerData recoil;
    SLdrCameraShakerData comboRecoil;
    SLdrCameraShakerData projectileRecoil;
    SLdrCameraShakerData flameThrower;
    SLdrCameraShakerData waveBuster;
    SLdrCameraShakerData projectileImpact;
};

void LoadTypedefSLdrTweakPlayerGun(SLdrTweakPlayerGun&, CInputStream&);

/* {
  SLdrTweakPlayerGun sldrThis;
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x7fda1466:
      // sldrThis.instanceName = TODO
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
} */
#endif // _SLDRTWEAKPLAYERGUN
