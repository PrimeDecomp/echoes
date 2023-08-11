#ifndef _SLDRLAYERSWITCH
#define _SLDRLAYERSWITCH

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrLayerSwitch {
    SLdrLayerSwitch();
    ~SLdrLayerSwitch();

    int areaId;
    int layerNumber;
};

void LoadTypedefSLdrLayerSwitch(SLdrLayerSwitch&, CInputStream&);

#endif // _SLDRLAYERSWITCH
