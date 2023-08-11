#ifndef _SLDRUNKNOWNSTRUCT23
#define _SLDRUNKNOWNSTRUCT23

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrShockWaveInfo.hpp"

struct SLdrUnknownStruct23 {
    SLdrUnknownStruct23();
    ~SLdrUnknownStruct23();

    float loopDuration;
    int destroyPercentage;
    SLdrShockWaveInfo shockWaveInfo;
    int sound;
};

void LoadTypedefSLdrUnknownStruct23(SLdrUnknownStruct23&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT23
