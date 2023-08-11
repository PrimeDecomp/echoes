#ifndef _SLDRUNKNOWNSTRUCT38
#define _SLDRUNKNOWNSTRUCT38

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrUnknownStruct38 {
    SLdrUnknownStruct38();
    ~SLdrUnknownStruct38();

    float range;
    float turnRate;
    int soundEffect;
    float warpScale;
    CVector3f repelOffset;
};

void LoadTypedefSLdrUnknownStruct38(SLdrUnknownStruct38&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT38
