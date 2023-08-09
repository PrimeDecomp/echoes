#ifndef _SLDRUNKNOWNSTRUCT35
#define _SLDRUNKNOWNSTRUCT35

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrUnknownStruct35 {
    SLdrUnknownStruct35();
    ~SLdrUnknownStruct35();

    float rotation_speed;
    CAssetId state_machine;
};

void LoadTypedefSLdrUnknownStruct35(SLdrUnknownStruct35&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT35
