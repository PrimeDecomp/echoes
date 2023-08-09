#ifndef _SLDRSCANSPEED
#define _SLDRSCANSPEED

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrScanSpeed {
    SLdrScanSpeed();
    ~SLdrScanSpeed();

    int value;
};

void LoadTypedefSLdrScanSpeed(SLdrScanSpeed&, CInputStream&);

#endif // _SLDRSCANSPEED
