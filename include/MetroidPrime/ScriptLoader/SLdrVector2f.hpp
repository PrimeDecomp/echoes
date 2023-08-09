#ifndef _SLDRVECTOR2F
#define _SLDRVECTOR2F

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrVector2f {
    SLdrVector2f();
    ~SLdrVector2f();

    float x;
    float y;
};

void LoadTypedefSLdrVector2f(SLdrVector2f&, CInputStream&);

#endif // _SLDRVECTOR2F
