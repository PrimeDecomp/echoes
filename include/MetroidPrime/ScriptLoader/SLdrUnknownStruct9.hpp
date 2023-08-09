#ifndef _SLDRUNKNOWNSTRUCT9
#define _SLDRUNKNOWNSTRUCT9

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrUnknownStruct9 {
    SLdrUnknownStruct9();
    ~SLdrUnknownStruct9();

    int type;
};

void LoadTypedefSLdrUnknownStruct9(SLdrUnknownStruct9&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT9
