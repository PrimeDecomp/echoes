#ifndef _SLDRUNKNOWNSTRUCT2
#define _SLDRUNKNOWNSTRUCT2

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrUnknownStruct2 {
    SLdrUnknownStruct2();
    ~SLdrUnknownStruct2();

    bool enabled;
    float chance;
    float modifier;
};

void LoadTypedefSLdrUnknownStruct2(SLdrUnknownStruct2&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT2
