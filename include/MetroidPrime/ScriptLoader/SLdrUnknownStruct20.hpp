#ifndef _SLDRUNKNOWNSTRUCT20
#define _SLDRUNKNOWNSTRUCT20

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDamageInfo.hpp"

struct SLdrUnknownStruct20 {
    SLdrUnknownStruct20();
    ~SLdrUnknownStruct20();

    SLdrDamageInfo damage;
};

void LoadTypedefSLdrUnknownStruct20(SLdrUnknownStruct20&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT20
