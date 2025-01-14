#ifndef _SLDRROTATIONSPLINES
#define _SLDRROTATIONSPLINES

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrRotationSplines {
    SLdrRotationSplines();
    ~SLdrRotationSplines();

    SLdrSpline xRotation;
    SLdrSpline yRotation;
    SLdrSpline zRotation;
};

void LoadTypedefSLdrRotationSplines(SLdrRotationSplines&, CInputStream&);

#endif // _SLDRROTATIONSPLINES
