#ifndef _SLDRSPLINETYPE
#define _SLDRSPLINETYPE

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrSplineType {
    SLdrSplineType();
    ~SLdrSplineType();

    int type;
};

void LoadTypedefSLdrSplineType(SLdrSplineType&, CInputStream&);

#endif // _SLDRSPLINETYPE
