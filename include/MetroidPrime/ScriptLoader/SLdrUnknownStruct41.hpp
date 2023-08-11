#ifndef _SLDRUNKNOWNSTRUCT41
#define _SLDRUNKNOWNSTRUCT41

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlasmaBeamInfo.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSandBossStructB.hpp"

struct SLdrUnknownStruct41 {
    SLdrUnknownStruct41();
    ~SLdrUnknownStruct41();

    SLdrSandBossStructB sandBossStructB_0xb9784f0e;
    SLdrSandBossStructB sandBossStructB_0xb8ae1bdc;
    SLdrPlasmaBeamInfo chargeBeamInfo;
};

void LoadTypedefSLdrUnknownStruct41(SLdrUnknownStruct41&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT41
