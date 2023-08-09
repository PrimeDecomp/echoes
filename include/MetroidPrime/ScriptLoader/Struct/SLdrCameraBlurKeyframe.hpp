#ifndef _SLDRCAMERABLURKEYFRAME
#define _SLDRCAMERABLURKEYFRAME

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrCameraBlurKeyframe {
    SLdrCameraBlurKeyframe();
    ~SLdrCameraBlurKeyframe();

    SLdrEditorProperties editor_properties;
    int blur_type;
    float blur_radius;
    int which_filter_group;
    float interpolate_in_time;
    float interpolate_out_time;
};

void LoadTypedefSLdrCameraBlurKeyframe(SLdrCameraBlurKeyframe&, CInputStream&);

#endif // _SLDRCAMERABLURKEYFRAME
