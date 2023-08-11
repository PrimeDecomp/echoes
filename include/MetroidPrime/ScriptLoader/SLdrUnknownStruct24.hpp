#ifndef _SLDRUNKNOWNSTRUCT24
#define _SLDRUNKNOWNSTRUCT24

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlasmaBeamInfo.hpp"

struct SLdrUnknownStruct24 {
    SLdrUnknownStruct24();
    ~SLdrUnknownStruct24();

    SLdrPlasmaBeamInfo beamInfo;
    SLdrDamageInfo damage;
    int sound;
};

void LoadTypedefSLdrUnknownStruct24(SLdrUnknownStruct24&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT24
