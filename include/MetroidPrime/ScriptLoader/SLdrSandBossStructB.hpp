#ifndef _SLDRSANDBOSSSTRUCTB
#define _SLDRSANDBOSSSTRUCTB

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"

struct SLdrSandBossStructB {
    SLdrSandBossStructB();
    ~SLdrSandBossStructB();

    SLdrDamageInfo damage;
    float duration;
    float changeDirectionInterval;
    float unknown_0x1b57d422;
    float changeDirectionChance;
    float innerRadius;
    float outerRadius;
    float unknown_0x52642b7e;
    float unknown_0xfda3eb4b;
    float turnSpeed;
    float unknown_0x47cde539;
    int soundChargeBeam;
    int unknown_0x8d4f3b88;
    float unknown_0xbf88fe4f;
    float unknown_0x74c702b3;
};

void LoadTypedefSLdrSandBossStructB(SLdrSandBossStructB&, CInputStream&);

#endif // _SLDRSANDBOSSSTRUCTB
