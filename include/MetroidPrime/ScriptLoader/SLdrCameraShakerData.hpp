#ifndef _SLDRCAMERASHAKERDATA
#define _SLDRCAMERASHAKERDATA

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrCameraShakerData {
    SLdrCameraShakerData();
    ~SLdrCameraShakerData();

    int flags_camera_shaker;
    float attenuation_distance;
    spline horizontal_motion;
    spline vertical_motion;
    spline forward_motion;
    float duration;
    int audio_effect;
};

void LoadTypedefSLdrCameraShakerData(SLdrCameraShakerData&, CInputStream&);

#endif // _SLDRCAMERASHAKERDATA
