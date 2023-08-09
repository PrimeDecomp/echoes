#ifndef _SLDRUNKNOWNSTRUCT4
#define _SLDRUNKNOWNSTRUCT4

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrUnknownStruct4 {
    SLdrUnknownStruct4();
    ~SLdrUnknownStruct4();

    int behaviour_type;
};

void LoadTypedefSLdrUnknownStruct4(SLdrUnknownStruct4&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT4
