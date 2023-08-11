#ifndef _SLDRSPINDLECAMERASTRUCT
#define _SLDRSPINDLECAMERASTRUCT

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrSpindleCameraStruct {
    SLdrSpindleCameraStruct();
    ~SLdrSpindleCameraStruct();

    int interpolantType;
    spline interpolantSpline;
};

void LoadTypedefSLdrSpindleCameraStruct(SLdrSpindleCameraStruct&, CInputStream&);

#endif // _SLDRSPINDLECAMERASTRUCT
