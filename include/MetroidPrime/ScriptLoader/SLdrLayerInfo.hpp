#ifndef _SLDRLAYERINFO
#define _SLDRLAYERINFO

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrLayerInfo {
    SLdrLayerInfo();
    ~SLdrLayerInfo();

    int motion_type;
    float unknown;
    float rotation;
    float amplitude;
    float texture_scale;
};

void LoadTypedefSLdrLayerInfo(SLdrLayerInfo&, CInputStream&);

#endif // _SLDRLAYERINFO
