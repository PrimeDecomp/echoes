#ifndef _SLDRSANDWORMSTRUCT
#define _SLDRSANDWORMSTRUCT

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrSandwormStruct {
    SLdrSandwormStruct();
    ~SLdrSandwormStruct();

    float unknown_0x98106ee2;
    float unknown_0x95081226;
    float unknown_0xc2064265;
    float move_speed_multiplier;
    int unknown_0x59f14d7c;
    int unknown_0x9606b4b0;
    int unknown_0xfc2697dd;
};

void LoadTypedefSLdrSandwormStruct(SLdrSandwormStruct&, CInputStream&);

#endif // _SLDRSANDWORMSTRUCT
