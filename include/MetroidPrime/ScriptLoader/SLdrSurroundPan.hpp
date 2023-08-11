#ifndef _SLDRSURROUNDPAN
#define _SLDRSURROUNDPAN

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrSurroundPan {
    SLdrSurroundPan();
    ~SLdrSurroundPan();

    float pan;
    float surroundPan;
};

void LoadTypedefSLdrSurroundPan(SLdrSurroundPan&, CInputStream&);

#endif // _SLDRSURROUNDPAN
