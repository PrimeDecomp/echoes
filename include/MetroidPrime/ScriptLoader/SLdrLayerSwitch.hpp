#ifndef _SLDRLAYERSWITCH
#define _SLDRLAYERSWITCH

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrLayerSwitch {
    SLdrLayerSwitch();
    ~SLdrLayerSwitch();

    int area_id;
    int layer_number;
};

void LoadTypedefSLdrLayerSwitch(SLdrLayerSwitch&, CInputStream&);

#endif // _SLDRLAYERSWITCH
