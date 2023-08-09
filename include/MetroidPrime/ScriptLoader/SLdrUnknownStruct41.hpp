#ifndef _SLDRUNKNOWNSTRUCT41
#define _SLDRUNKNOWNSTRUCT41

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlasmaBeamInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSandBossStructB.hpp"

struct SLdrUnknownStruct41 {
    SLdrUnknownStruct41();
    ~SLdrUnknownStruct41();

    SLdrSandBossStructB sand_boss_struct_b_0xb9784f0e;
    SLdrSandBossStructB sand_boss_struct_b_0xb8ae1bdc;
    SLdrPlasmaBeamInfo charge_beam_info;
};

void LoadTypedefSLdrUnknownStruct41(SLdrUnknownStruct41&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT41
