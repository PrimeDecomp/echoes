#ifndef _SLDRSCALESPLINES
#define _SLDRSCALESPLINES

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrScaleSplines {
    SLdrScaleSplines();
    ~SLdrScaleSplines();

    SLdrSpline xScale;
    SLdrSpline yScale;
    SLdrSpline zScale;
};

void LoadTypedefSLdrScaleSplines(SLdrScaleSplines&, CInputStream&);

#endif // _SLDRSCALESPLINES
