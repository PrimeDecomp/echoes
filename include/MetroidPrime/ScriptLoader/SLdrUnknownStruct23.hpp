#ifndef _SLDRUNKNOWNSTRUCT23
#define _SLDRUNKNOWNSTRUCT23

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrShockWaveInfo.hpp"

struct SLdrUnknownStruct23 {
    SLdrUnknownStruct23();
    ~SLdrUnknownStruct23();

    float loop_duration;
    int destroy_percentage;
    SLdrShockWaveInfo shock_wave_info;
    int sound;
};

void LoadTypedefSLdrUnknownStruct23(SLdrUnknownStruct23&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT23
