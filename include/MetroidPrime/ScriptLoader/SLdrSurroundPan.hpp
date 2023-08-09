#ifndef _SLDRSURROUNDPAN
#define _SLDRSURROUNDPAN

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrSurroundPan {
    SLdrSurroundPan();
    ~SLdrSurroundPan();

    float pan;
    float surround_pan;
};

void LoadTypedefSLdrSurroundPan(SLdrSurroundPan&, CInputStream&);

#endif // _SLDRSURROUNDPAN
