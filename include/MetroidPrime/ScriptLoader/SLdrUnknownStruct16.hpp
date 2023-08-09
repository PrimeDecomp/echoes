#ifndef _SLDRUNKNOWNSTRUCT16
#define _SLDRUNKNOWNSTRUCT16

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrUnknownStruct16 {
    SLdrUnknownStruct16();
    ~SLdrUnknownStruct16();

    float unknown_0x7ee77018;
    float unknown_0xbfb7ca5c;
    float unknown_0x1e2debc6;
};

void LoadTypedefSLdrUnknownStruct16(SLdrUnknownStruct16&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT16
