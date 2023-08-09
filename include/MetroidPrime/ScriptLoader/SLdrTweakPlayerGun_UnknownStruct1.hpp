#ifndef _SLDRTWEAKPLAYERGUN_UNKNOWNSTRUCT1
#define _SLDRTWEAKPLAYERGUN_UNKNOWNSTRUCT1

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTDamageInfo.hpp"

struct SLdrTweakPlayerGun_UnknownStruct1 {
    SLdrTweakPlayerGun_UnknownStruct1();
    ~SLdrTweakPlayerGun_UnknownStruct1();

    SLdrTDamageInfo super_missile;
    SLdrTDamageInfo darkburst;
    SLdrTDamageInfo sunburst;
    SLdrTDamageInfo sonic_boom;
    SLdrTDamageInfo unknown;
};

void LoadTypedefSLdrTweakPlayerGun_UnknownStruct1(SLdrTweakPlayerGun_UnknownStruct1&, CInputStream&);

#endif // _SLDRTWEAKPLAYERGUN_UNKNOWNSTRUCT1
