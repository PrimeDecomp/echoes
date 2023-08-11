#ifndef _SLDRINGBOOSTBALLGUARDIANSTRUCT
#define _SLDRINGBOOSTBALLGUARDIANSTRUCT

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrIngBoostBallGuardianStruct {
    SLdrIngBoostBallGuardianStruct();
    ~SLdrIngBoostBallGuardianStruct();

    float unknown_0x25d02bc5;
    float unknown_0xabe99de0;
    float unknown_0xe2b23f03;
    float unknown_0x2f845006;
    float unknown_0x5d1626fb;
    float unknown_0xbb76891a;
    int unknown_0x285d67ad;
    int unknown_0x6d5f242f;
    int unknown_0xe3ff2ed6;
    int unknown_0xa6fd6d54;
    int unknown_0xecb314b6;
    int unknown_0xa9b15734;
    float locomotionSpeedScale;
    float ingSpotSpeedScale;
};

void LoadTypedefSLdrIngBoostBallGuardianStruct(SLdrIngBoostBallGuardianStruct&, CInputStream&);

#endif // _SLDRINGBOOSTBALLGUARDIANSTRUCT
