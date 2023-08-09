#ifndef _SLDRUNKNOWNSTRUCT6
#define _SLDRUNKNOWNSTRUCT6

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrUnknownStruct6 {
    SLdrUnknownStruct6();
    ~SLdrUnknownStruct6();

    bool override;
    float unknown;
};

void LoadTypedefSLdrUnknownStruct6(SLdrUnknownStruct6&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT6
