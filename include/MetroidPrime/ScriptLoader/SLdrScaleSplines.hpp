#ifndef _SLDRSCALESPLINES
#define _SLDRSCALESPLINES

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrScaleSplines {
    SLdrScaleSplines();
    ~SLdrScaleSplines();

    spline x_scale;
    spline y_scale;
    spline z_scale;
};

void LoadTypedefSLdrScaleSplines(SLdrScaleSplines&, CInputStream&);

#endif // _SLDRSCALESPLINES
