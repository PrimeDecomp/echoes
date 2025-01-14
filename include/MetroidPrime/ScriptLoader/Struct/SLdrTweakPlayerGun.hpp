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

#endif // _SLDRTWEAKPLAYERGUN
