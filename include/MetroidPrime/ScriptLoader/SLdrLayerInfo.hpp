#ifndef _SLDRLAYERINFO
#define _SLDRLAYERINFO

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrLayerInfo {
    SLdrLayerInfo();
    ~SLdrLayerInfo();

    int motionType;
    float unknown;
    float rotation;
    float amplitude;
    float textureScale;
};

void LoadTypedefSLdrLayerInfo(SLdrLayerInfo&, CInputStream&);

#endif // _SLDRLAYERINFO
