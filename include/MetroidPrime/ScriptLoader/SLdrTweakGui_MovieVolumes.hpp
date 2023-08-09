#ifndef _SLDRTWEAKGUI_MOVIEVOLUMES
#define _SLDRTWEAKGUI_MOVIEVOLUMES

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakGui_MovieVolumes {
    SLdrTweakGui_MovieVolumes();
    ~SLdrTweakGui_MovieVolumes();

    int unknown_0xae149646;
    int unknown_0xc1a2e858;
    int unknown_0x138c3bb8;
    int unknown_0xe5587648;
    int unknown_0x9ed00248;
    int unknown_0x6f135424;
    int unknown_0xdb2260b7;
    int unknown_0xf38093f5;
};

void LoadTypedefSLdrTweakGui_MovieVolumes(SLdrTweakGui_MovieVolumes&, CInputStream&);

#endif // _SLDRTWEAKGUI_MOVIEVOLUMES
