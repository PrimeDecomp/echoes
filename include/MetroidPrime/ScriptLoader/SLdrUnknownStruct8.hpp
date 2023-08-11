#ifndef _SLDRUNKNOWNSTRUCT8
#define _SLDRUNKNOWNSTRUCT8

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrUnknownStruct8 {
    SLdrUnknownStruct8();
    ~SLdrUnknownStruct8();

    bool override;
    float zOffset;
};

void LoadTypedefSLdrUnknownStruct8(SLdrUnknownStruct8&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT8
