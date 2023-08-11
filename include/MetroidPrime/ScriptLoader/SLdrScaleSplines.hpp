#ifndef _SLDRSCALESPLINES
#define _SLDRSCALESPLINES

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrScaleSplines {
    SLdrScaleSplines();
    ~SLdrScaleSplines();

    spline xScale;
    spline yScale;
    spline zScale;
};

void LoadTypedefSLdrScaleSplines(SLdrScaleSplines&, CInputStream&);

#endif // _SLDRSCALESPLINES
