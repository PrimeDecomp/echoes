#ifndef _SLDRROTATIONSPLINES
#define _SLDRROTATIONSPLINES

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrRotationSplines {
    SLdrRotationSplines();
    ~SLdrRotationSplines();

    spline xRotation;
    spline yRotation;
    spline zRotation;
};

void LoadTypedefSLdrRotationSplines(SLdrRotationSplines&, CInputStream&);

#endif // _SLDRROTATIONSPLINES
