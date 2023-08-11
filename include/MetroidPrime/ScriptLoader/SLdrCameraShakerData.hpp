#ifndef _SLDRCAMERASHAKERDATA
#define _SLDRCAMERASHAKERDATA

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrCameraShakerData {
    SLdrCameraShakerData();
    ~SLdrCameraShakerData();

    int flagsCameraShaker;
    float attenuationDistance;
    spline horizontalMotion;
    spline verticalMotion;
    spline forwardMotion;
    float duration;
    int audioEffect;
};

void LoadTypedefSLdrCameraShakerData(SLdrCameraShakerData&, CInputStream&);

#endif // _SLDRCAMERASHAKERDATA
