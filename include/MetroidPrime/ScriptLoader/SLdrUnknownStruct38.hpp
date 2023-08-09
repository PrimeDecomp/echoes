#ifndef _SLDRUNKNOWNSTRUCT38
#define _SLDRUNKNOWNSTRUCT38

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrUnknownStruct38 {
    SLdrUnknownStruct38();
    ~SLdrUnknownStruct38();

    float range;
    float turn_rate;
    int sound_effect;
    float warp_scale;
    CVector3f repel_offset;
};

void LoadTypedefSLdrUnknownStruct38(SLdrUnknownStruct38&, CInputStream&);

#endif // _SLDRUNKNOWNSTRUCT38
