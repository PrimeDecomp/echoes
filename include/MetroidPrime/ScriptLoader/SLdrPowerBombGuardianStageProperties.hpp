#ifndef _SLDRPOWERBOMBGUARDIANSTAGEPROPERTIES
#define _SLDRPOWERBOMBGUARDIANSTAGEPROPERTIES

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrPowerBombGuardianStageProperties {
    SLdrPowerBombGuardianStageProperties();
    ~SLdrPowerBombGuardianStageProperties();

    float unknown_0x95e7a2c2;
    float unknown_0x76ba1c18;
    float unknown_0x3eb2de35;
    float unknown_0xe50d8dd2;
    int unknown_0x64d482d5;
    int unknown_0xc3e002ac;
    float unknown_0xbb4b6680;
    float unknown_0xd356c997;
    float doubleShotChance;
    int unknown_0x87cc8ba4;
    int unknown_0x6491357e;
};

void LoadTypedefSLdrPowerBombGuardianStageProperties(SLdrPowerBombGuardianStageProperties&, CInputStream&);

#endif // _SLDRPOWERBOMBGUARDIANSTAGEPROPERTIES
