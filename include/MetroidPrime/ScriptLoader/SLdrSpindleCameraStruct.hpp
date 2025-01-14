#ifndef _SLDRSPINDLECAMERASTRUCT
#define _SLDRSPINDLECAMERASTRUCT

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrSpindleCameraStruct {
    SLdrSpindleCameraStruct();
    ~SLdrSpindleCameraStruct();

    int interpolantType;
    SLdrSpline interpolantSpline;
};

void LoadTypedefSLdrSpindleCameraStruct(SLdrSpindleCameraStruct&, CInputStream&);

#endif // _SLDRSPINDLECAMERASTRUCT
