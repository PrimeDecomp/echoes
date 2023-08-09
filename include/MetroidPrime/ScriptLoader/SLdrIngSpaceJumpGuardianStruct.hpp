#ifndef _SLDRINGSPACEJUMPGUARDIANSTRUCT
#define _SLDRINGSPACEJUMPGUARDIANSTRUCT

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrIngSpaceJumpGuardianStruct {
    SLdrIngSpaceJumpGuardianStruct();
    ~SLdrIngSpaceJumpGuardianStruct();

    float taunt_chance;
    float attack_chance;
    float unknown_0x03698c10;
    float locomotion_speed;
    float unknown_0x3e370622;
};

void LoadTypedefSLdrIngSpaceJumpGuardianStruct(SLdrIngSpaceJumpGuardianStruct&, CInputStream&);

#endif // _SLDRINGSPACEJUMPGUARDIANSTRUCT
