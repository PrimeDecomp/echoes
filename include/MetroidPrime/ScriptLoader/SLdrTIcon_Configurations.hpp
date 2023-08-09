#ifndef _SLDRTICON_CONFIGURATIONS
#define _SLDRTICON_CONFIGURATIONS

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTIcon_Configurations {
    SLdrTIcon_Configurations();
    ~SLdrTIcon_Configurations();

    float unknown_0x3058aff2;
    float unknown_0xf1d67032;
    float unknown_0x68341633;
    float unknown_0xa9bac9f3;
    float unknown_0x8081dc70;
    float unknown_0x410f03b0;
    float unknown_0xd8ed65b1;
    float unknown_0x1963ba71;
    float unknown_0x8a9b4eb7;
};

void LoadTypedefSLdrTIcon_Configurations(SLdrTIcon_Configurations&, CInputStream&);

#endif // _SLDRTICON_CONFIGURATIONS
