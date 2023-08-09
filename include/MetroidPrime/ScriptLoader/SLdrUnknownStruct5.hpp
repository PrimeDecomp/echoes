#ifndef _SLDRUNKNOWNSTRUCT5
#define _SLDRUNKNOWNSTRUCT5

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrUnknownStruct5 {
    SLdrUnknownStruct5();
    ~SLdrUnknownStruct5();

    bool override;
    CVector3f offset;
};

void LoadTypedefSLdrUnknownStruct5(SLdrUnknownStruct5&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT5
