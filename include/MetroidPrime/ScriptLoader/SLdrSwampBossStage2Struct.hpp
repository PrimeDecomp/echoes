#ifndef _SLDRSWAMPBOSSSTAGE2STRUCT
#define _SLDRSWAMPBOSSSTAGE2STRUCT

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrSwampBossStage2Struct {
    SLdrSwampBossStage2Struct();
    ~SLdrSwampBossStage2Struct();

    float unknown_0x95e7a2c2;
    float unknown_0x76ba1c18;
    float unknown_0x29e6ead6;
    float unknown_0x1753225e;
    float dashChance;
    float tauntChance;
    int firstAttack;
    int secondAttack;
    int thirdAttack;
    int fourthAttack;
    int fifthAttack;
    float health;
    int unknown_0x2e7e55f2;
    int unknown_0xef3efec0;
    int unknown_0x2b0bfd51;
};

void LoadTypedefSLdrSwampBossStage2Struct(SLdrSwampBossStage2Struct&, CInputStream&);

#endif // _SLDRSWAMPBOSSSTAGE2STRUCT
