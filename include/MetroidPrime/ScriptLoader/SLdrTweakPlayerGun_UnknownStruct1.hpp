#ifndef _SLDRTWEAKPLAYERGUN_UNKNOWNSTRUCT1
#define _SLDRTWEAKPLAYERGUN_UNKNOWNSTRUCT1

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTDamageInfo.hpp"

struct SLdrTweakPlayerGun_UnknownStruct1 {
    SLdrTweakPlayerGun_UnknownStruct1();
    ~SLdrTweakPlayerGun_UnknownStruct1();

    SLdrTDamageInfo superMissile;
    SLdrTDamageInfo darkburst;
    SLdrTDamageInfo sunburst;
    SLdrTDamageInfo sonicBoom;
    SLdrTDamageInfo unknown;
};

void LoadTypedefSLdrTweakPlayerGun_UnknownStruct1(SLdrTweakPlayerGun_UnknownStruct1&, CInputStream&);

#endif // _SLDRTWEAKPLAYERGUN_UNKNOWNSTRUCT1
