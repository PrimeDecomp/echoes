#ifndef _SLDRROTATIONSPLINES
#define _SLDRROTATIONSPLINES

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrRotationSplines {
    SLdrRotationSplines();
    ~SLdrRotationSplines();

    spline x_rotation;
    spline y_rotation;
    spline z_rotation;
};

void LoadTypedefSLdrRotationSplines(SLdrRotationSplines&, CInputStream&);

#endif // _SLDRROTATIONSPLINES
