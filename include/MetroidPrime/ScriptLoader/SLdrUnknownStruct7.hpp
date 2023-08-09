#ifndef _SLDRUNKNOWNSTRUCT7
#define _SLDRUNKNOWNSTRUCT7

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrUnknownStruct7 {
    SLdrUnknownStruct7();
    ~SLdrUnknownStruct7();

    bool override;
    float speed;
};

void LoadTypedefSLdrUnknownStruct7(SLdrUnknownStruct7&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT7
