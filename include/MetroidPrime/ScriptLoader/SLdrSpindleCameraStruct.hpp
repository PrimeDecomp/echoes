#ifndef _SLDRSPINDLECAMERASTRUCT
#define _SLDRSPINDLECAMERASTRUCT

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrSpindleCameraStruct {
    SLdrSpindleCameraStruct();
    ~SLdrSpindleCameraStruct();

    int interpolant_type;
    spline interpolant_spline;
};

void LoadTypedefSLdrSpindleCameraStruct(SLdrSpindleCameraStruct&, CInputStream&);

#endif // _SLDRSPINDLECAMERASTRUCT
