#ifndef _SLDRDIGITALGUARDIANHEADSTRUCT
#define _SLDRDIGITALGUARDIANHEADSTRUCT

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrDigitalGuardianHeadStruct {
    SLdrDigitalGuardianHeadStruct();
    ~SLdrDigitalGuardianHeadStruct();

    int first_shot_type;
    float projectile_telegraph_time;
    float projectile_attack_time;
    float unknown_0xfdfca535;
    float unknown_0xcd03632c;
    float unknown_0xf1548397;
    float unknown_0xf967e246;
};

void LoadTypedefSLdrDigitalGuardianHeadStruct(SLdrDigitalGuardianHeadStruct&, CInputStream&);

#endif // _SLDRDIGITALGUARDIANHEADSTRUCT
