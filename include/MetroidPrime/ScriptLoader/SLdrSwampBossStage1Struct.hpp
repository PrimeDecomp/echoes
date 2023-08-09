#ifndef _SLDRSWAMPBOSSSTAGE1STRUCT
#define _SLDRSWAMPBOSSSTAGE1STRUCT

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrSwampBossStage1Struct {
    SLdrSwampBossStage1Struct();
    ~SLdrSwampBossStage1Struct();

    float unknown_0x98106ee2;
    float unknown_0x95e7a2c2;
    float unknown_0x76ba1c18;
    int unknown_0xbb0ffdd6;
    int unknown_0x60b0ae31;
    int first_attack;
    int second_attack;
    int third_attack;
    int fourth_attack;
};

void LoadTypedefSLdrSwampBossStage1Struct(SLdrSwampBossStage1Struct&, CInputStream&);

#endif // _SLDRSWAMPBOSSSTAGE1STRUCT
