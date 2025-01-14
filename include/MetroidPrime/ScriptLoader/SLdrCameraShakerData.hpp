#ifndef _SLDRCAMERASHAKERDATA
#define _SLDRCAMERASHAKERDATA

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrCameraShakerData {
    SLdrCameraShakerData();
    ~SLdrCameraShakerData();

    int flagsCameraShaker;
    float attenuationDistance;
    SLdrSpline horizontalMotion;
    SLdrSpline verticalMotion;
    SLdrSpline forwardMotion;
    float duration;
    int audioEffect;
};

void LoadTypedefSLdrCameraShakerData(SLdrCameraShakerData&, CInputStream&);

#endif // _SLDRCAMERASHAKERDATA
